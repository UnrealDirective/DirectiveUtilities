// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Libraries/DirectiveUtilEditorBlueprintLibrary.h"
#include "Misc/AutomationTest.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/Guid.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorBlueprintLibraryTest,
	"DirectiveUtilities.EditorBlueprintLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorBlueprintLibraryTest::RunTest(const FString& Parameters)
{
	auto ContainsBlueprint = [](const TArray<FAssetData>& Assets, const UBlueprint* Expected) {
		return Assets.ContainsByPredicate([Expected](const FAssetData& Asset) {
			return Asset.GetAsset() == Expected;
		});
	};

	TestEqual(
		TEXT("A null Blueprint has unknown status"),
		UDirectiveUtilEditorBlueprintLibrary::GetBlueprintCompileStatus(nullptr),
		EDirectiveUtilBlueprintCompileStatus::Unknown);

	UBlueprint* Blueprint = NewObject<UBlueprint>();
	const TArray<TPair<EBlueprintStatus, EDirectiveUtilBlueprintCompileStatus>> Statuses = {
		{BS_Unknown, EDirectiveUtilBlueprintCompileStatus::Unknown},
		{BS_Dirty, EDirectiveUtilBlueprintCompileStatus::Dirty},
		{BS_Error, EDirectiveUtilBlueprintCompileStatus::Error},
		{BS_UpToDate, EDirectiveUtilBlueprintCompileStatus::UpToDate},
		{BS_BeingCreated, EDirectiveUtilBlueprintCompileStatus::BeingCreated},
		{BS_UpToDateWithWarnings, EDirectiveUtilBlueprintCompileStatus::UpToDateWithWarnings},
	};
	for (const TPair<EBlueprintStatus, EDirectiveUtilBlueprintCompileStatus>& Status : Statuses)
	{
		Blueprint->Status = Status.Key;
		TestEqual(
			TEXT("Blueprint status maps to the public enum"),
			UDirectiveUtilEditorBlueprintLibrary::GetBlueprintCompileStatus(Blueprint),
			Status.Value);
	}

	FDirectiveUtilBlueprintSearchOptions Options;
	Options.PackagePaths = {TEXT("/Engine/BasicShapes")};
	TestTrue(
		TEXT("A folder without Blueprints has no compile-status matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByCompileStatus(
			EDirectiveUtilBlueprintCompileStatus::UpToDate,
			Options).IsEmpty());
	TestTrue(
		TEXT("A null parent class returns no matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(nullptr, Options).IsEmpty());
	TestTrue(
		TEXT("A folder without Blueprints has no parent-class matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(AActor::StaticClass(), Options).IsEmpty());
	TestTrue(
		TEXT("A non-interface class returns no interface matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsImplementingInterface(UObject::StaticClass(), Options).IsEmpty());
	TestTrue(
		TEXT("A folder without Blueprints has no interface matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsImplementingInterface(UInterface::StaticClass(), Options).IsEmpty());
	TestTrue(
		TEXT("A non-component class returns no component matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsContainingComponentClass(UObject::StaticClass(), Options).IsEmpty());
	TestTrue(
		TEXT("A folder without Blueprints has no component matches"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsContainingComponentClass(UActorComponent::StaticClass(), Options).IsEmpty());
	TestTrue(
		TEXT("A Blueprint without generated properties has no unused variables"),
		UDirectiveUtilEditorBlueprintLibrary::GetUnusedBlueprintVariables(Blueprint).IsEmpty());

	const FName FixtureName(*FString::Printf(
		TEXT("BP_InspectionFixture_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits)));
	const FString FixturePackageName = FString::Printf(
		TEXT("/Game/DirectiveUtilitiesTests/%s"),
		*FixtureName.ToString());
	UPackage* TestPackage = CreatePackage(*FixturePackageName);
	UBlueprint* InspectionBlueprint = FKismetEditorUtilities::CreateBlueprint(
		AActor::StaticClass(),
		TestPackage,
		FixtureName,
		BPTYPE_Normal,
		TEXT("DirectiveUtilities.EditorBlueprintLibraryTests"));
	if (!TestNotNull(TEXT("The inspection fixture Blueprint is created"), InspectionBlueprint))
	{
		return false;
	}

	TestTrue(
		TEXT("The inspection fixture implements its test interface"),
		FBlueprintEditorUtils::ImplementNewInterface(
			InspectionBlueprint,
			UDirectiveUtilTestInterface::StaticClass()->GetClassPathName()));
	USCS_Node* ComponentNode = InspectionBlueprint->SimpleConstructionScript->CreateNode(
		UStaticMeshComponent::StaticClass(),
		TEXT("InspectionComponent"));
	InspectionBlueprint->SimpleConstructionScript->AddNode(ComponentNode);
	FEdGraphPinType VariableType;
	VariableType.PinCategory = UEdGraphSchema_K2::PC_Int;
	TestTrue(
		TEXT("The inspection fixture adds an unused variable"),
		FBlueprintEditorUtils::AddMemberVariable(InspectionBlueprint, TEXT("UnusedValue"), VariableType));
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(InspectionBlueprint);
	FKismetEditorUtilities::CompileBlueprint(InspectionBlueprint);
	FAssetRegistryModule::AssetCreated(InspectionBlueprint);

	FDirectiveUtilBlueprintSearchOptions FixtureOptions;
	FixtureOptions.PackagePaths = {TEXT("/Game/DirectiveUtilitiesTests")};
	TestTrue(
		TEXT("Compile-status search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByCompileStatus(
				EDirectiveUtilBlueprintCompileStatus::UpToDate,
				FixtureOptions),
			InspectionBlueprint));
	TestTrue(
		TEXT("Direct-parent search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(
				AActor::StaticClass(),
				FixtureOptions,
				false),
			InspectionBlueprint));
	TestTrue(
		TEXT("Descendant-parent search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(
				AActor::StaticClass(),
				FixtureOptions,
				true),
			InspectionBlueprint));
	TestTrue(
		TEXT("Interface search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsImplementingInterface(
				UDirectiveUtilTestInterface::StaticClass(),
				FixtureOptions),
			InspectionBlueprint));
	TestTrue(
		TEXT("Exact component search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsContainingComponentClass(
				UStaticMeshComponent::StaticClass(),
				FixtureOptions,
				false),
			InspectionBlueprint));
	TestTrue(
		TEXT("Derived component search finds the fixture"),
		ContainsBlueprint(
			UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsContainingComponentClass(
				UActorComponent::StaticClass(),
				FixtureOptions,
				true),
			InspectionBlueprint));
	TestTrue(
		TEXT("Unused-variable search finds the fixture variable"),
		UDirectiveUtilEditorBlueprintLibrary::GetUnusedBlueprintVariables(InspectionBlueprint).Contains(TEXT("UnusedValue")));

	FixtureOptions.ExcludedPackagePaths = {TEXT("/Game/DirectiveUtilitiesTests")};
	TestTrue(
		TEXT("Excluded paths remove the fixture from search"),
		UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(
			AActor::StaticClass(),
			FixtureOptions).IsEmpty());

	FAssetRegistryModule::AssetDeleted(InspectionBlueprint);
	TestPackage->SetDirtyFlag(false);

	return true;
}

#endif
