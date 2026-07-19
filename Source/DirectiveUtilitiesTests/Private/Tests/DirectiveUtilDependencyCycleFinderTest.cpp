// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "AssetRegistry/DirectiveUtilDependencyCycleFinder.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilDependencyCycleFinderTest,
	"DirectiveUtilities.EditorDependencyCycleFinderTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilDependencyCycleFinderTest::RunTest(const FString& Parameters)
{
	TMap<FName, TArray<FName>> Graph;
	Graph.Add(TEXT("/Game/A"), {TEXT("/Game/B")});
	Graph.Add(TEXT("/Game/B"), {TEXT("/Game/A")});
	Graph.Add(TEXT("/Game/C"), {TEXT("/Game/C")});
	Graph.Add(TEXT("/Game/D"), {TEXT("/Game/E")});
	Graph.Add(TEXT("/Game/E"));

	const TArray<FDirectiveUtilAssetDependencyCycle> Cycles =
		DirectiveUtilitiesEditor::FDependencyCycleFinder(Graph).Find();
	TestEqual(TEXT("The graph contains two cycles"), Cycles.Num(), 2);
	if (Cycles.Num() == 2)
	{
		TestEqual(TEXT("The first cycle contains two packages"), Cycles[0].Packages.Num(), 2);
		TestTrue(TEXT("The first cycle contains A"), Cycles[0].Packages.Contains(TEXT("/Game/A")));
		TestTrue(TEXT("The first cycle contains B"), Cycles[0].Packages.Contains(TEXT("/Game/B")));
		TestEqual(TEXT("The self-cycle contains one package"), Cycles[1].Packages.Num(), 1);
		TestTrue(TEXT("The self-cycle contains C"), Cycles[1].Packages.Contains(TEXT("/Game/C")));
	}

	constexpr int32 NodeCount = 20000;
	constexpr int32 CycleStart = NodeCount / 2;
	TArray<FName> Nodes;
	Nodes.Reserve(NodeCount);
	for (int32 Index = 0; Index < NodeCount; ++Index)
	{
		Nodes.Add(*FString::Printf(TEXT("/Game/Deep/%05d"), Index));
	}

	TMap<FName, TArray<FName>> DeepGraph;
	DeepGraph.Reserve(NodeCount);
	for (int32 Index = 0; Index < NodeCount - 1; ++Index)
	{
		DeepGraph.Add(Nodes[Index], {Nodes[Index + 1]});
	}
	DeepGraph.Add(Nodes.Last(), {Nodes[CycleStart]});

	const TArray<FDirectiveUtilAssetDependencyCycle> DeepCycles =
		DirectiveUtilitiesEditor::FDependencyCycleFinder(DeepGraph).Find();
	TestEqual(TEXT("The deep graph contains one cycle"), DeepCycles.Num(), 1);
	if (DeepCycles.Num() == 1)
	{
		TestEqual(TEXT("The deep cycle contains every connected member"), DeepCycles[0].Packages.Num(), NodeCount - CycleStart);
		TestTrue(TEXT("The deep cycle starts at the expected package"), DeepCycles[0].Packages.Contains(Nodes[CycleStart]));
		TestTrue(TEXT("The deep cycle ends at the expected package"), DeepCycles[0].Packages.Contains(Nodes.Last()));
	}

	return true;
}

#endif
