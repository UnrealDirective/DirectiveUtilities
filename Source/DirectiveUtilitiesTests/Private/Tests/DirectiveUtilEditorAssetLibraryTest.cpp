// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "AssetRegistry/AssetData.h"
#include "Libraries/DirectiveUtilEditorAssetLibrary.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorAssetLibraryTest, "DirectiveUtilities.EditorAssetLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorAssetLibraryTest::RunTest(const FString& Parameters)
{
	EDirectiveUtilSuccessStatus Status = EDirectiveUtilSuccessStatus::Success;

	const FString DefaultName = UDirectiveUtilEditorAssetLibrary::GetDefaultAssetNameForClass(nullptr, Status);
	TestEqual("GetDefaultAssetNameForClass should fail for a null class", Status, EDirectiveUtilSuccessStatus::Failure);
	TestTrue("GetDefaultAssetNameForClass should return an empty name for a null class", DefaultName.IsEmpty());

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FAssetData> NoAssets = UDirectiveUtilEditorAssetLibrary::GetAssetsByClass(nullptr, TEXT("/Game"), false, true, Status);
	TestEqual("GetAssetsByClass should fail for a null class", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetsByClass should return no assets for a null class", NoAssets.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FString> NoDependencies = UDirectiveUtilEditorAssetLibrary::GetAssetDependencies(FAssetData(), false, Status);
	TestEqual("GetAssetDependencies should fail for an invalid asset", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetDependencies should return nothing for an invalid asset", NoDependencies.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FString> NoReferencers = UDirectiveUtilEditorAssetLibrary::GetAssetReferencers(FAssetData(), false, Status);
	TestEqual("GetAssetReferencers should fail for an invalid asset", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetReferencers should return nothing for an invalid asset", NoReferencers.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Failure;
	UDirectiveUtilEditorAssetLibrary::GetAssetsByClass(UWorld::StaticClass(), TEXT("/Game"), false, true, Status);
	TestEqual("GetAssetsByClass should succeed for a valid class query", Status, EDirectiveUtilSuccessStatus::Success);

	const TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> DuplicateAssets =
		UDirectiveUtilEditorAssetLibrary::FindDuplicateAssets({TEXT("/Engine/BasicShapes"), TEXT("/Engine/BasicShapes")}, false);
	for (const TPair<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData>& Pair : DuplicateAssets)
	{
		TSet<FString> UniquePaths;
		for (const FString& AssetPath : Pair.Value.DuplicateAssetPaths)
		{
			UniquePaths.Add(AssetPath);
		}
		TestEqual("FindDuplicateAssets should return each asset path once", UniquePaths.Num(), Pair.Value.DuplicateAssetPaths.Num());
	}

	return true;
}

#endif
