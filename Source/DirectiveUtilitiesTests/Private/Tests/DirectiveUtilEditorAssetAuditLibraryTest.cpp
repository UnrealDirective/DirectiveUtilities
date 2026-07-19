// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "AssetRegistry/IAssetRegistry.h"
#include "Libraries/DirectiveUtilEditorAssetAuditLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorAssetAuditLibraryTest,
	"DirectiveUtilities.EditorAssetAuditLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorAssetAuditLibraryTest::RunTest(const FString& Parameters)
{
	FDirectiveUtilAssetAuditOptions Options;
	Options.PackagePaths = {TEXT("/Engine/BasicShapes")};

	const FDirectiveUtilAssetAuditReport Report = UDirectiveUtilEditorAssetAuditLibrary::BuildAssetAuditReport(Options);
	TestTrue(TEXT("The engine shape scan returns assets"), !Report.Assets.IsEmpty());
	if (const IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
	{
		TestTrue(TEXT("An asset audit starts the initial registry scan"), AssetRegistry->IsSearchAllAssets());
	}

	for (const FDirectiveUtilAssetAuditEntry& Entry : Report.Assets)
	{
		TestTrue(TEXT("Report assets have valid data"), Entry.Asset.IsValid());
		TestTrue(TEXT("Report assets have a package"), !Entry.PackageName.IsNone());
		TestTrue(TEXT("Report assets have a class"), !Entry.AssetClass.IsEmpty());
		TestTrue(TEXT("Report counts are non-negative"), Entry.DependencyCount >= 0 && Entry.ReferencerCount >= 0);
	}

	const FString Csv = UDirectiveUtilEditorAssetAuditLibrary::AssetAuditReportToCsv(Report);
	TestTrue(TEXT("CSV includes its header"), Csv.StartsWith(TEXT("Asset,Package,Path,Class")));
	TestTrue(TEXT("CSV includes scanned assets"), Csv.Contains(TEXT("/Engine/BasicShapes")));

	Options.ExcludedPackagePaths = {TEXT("/Engine/BasicShapes")};
	TestTrue(
		TEXT("Excluded paths return no candidates"),
		UDirectiveUtilEditorAssetAuditLibrary::FindUnreferencedAssetCandidates(Options).IsEmpty());
	TestTrue(
		TEXT("Excluded paths return no missing references"),
		UDirectiveUtilEditorAssetAuditLibrary::FindMissingAssetReferences(Options).IsEmpty());
	TestTrue(
		TEXT("Excluded paths return no cycles"),
		UDirectiveUtilEditorAssetAuditLibrary::FindAssetDependencyCycles(Options).IsEmpty());

	FDirectiveUtilAssetAuditOptions EngineOptions;
	EngineOptions.PackagePaths = {TEXT("/Engine")};
	const FDirectiveUtilAssetAuditReport EngineReport = UDirectiveUtilEditorAssetAuditLibrary::BuildAssetAuditReport(EngineOptions);
	TestTrue(TEXT("A full engine audit returns more than the basic shapes scan"), EngineReport.Assets.Num() > Report.Assets.Num());
	for (const FDirectiveUtilAssetDependencyCycle& Cycle : EngineReport.DependencyCycles)
	{
		TestTrue(TEXT("Dependency cycles contain at least one package"), !Cycle.Packages.IsEmpty());
		for (int32 Index = 1; Index < Cycle.Packages.Num(); ++Index)
		{
			TestTrue(TEXT("Dependency cycle packages are sorted"), Cycle.Packages[Index - 1].LexicalLess(Cycle.Packages[Index]));
		}
	}

	return true;
}

#endif
