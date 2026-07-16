// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "Libraries/DirectiveUtilEditorBlueprintLibrary.h"
#include "Misc/AutomationTest.h"
#include "Components/ActorComponent.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "UObject/Interface.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorBlueprintLibraryTest,
	"DirectiveUtilities.EditorBlueprintLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorBlueprintLibraryTest::RunTest(const FString& Parameters)
{
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

	return true;
}

#endif
