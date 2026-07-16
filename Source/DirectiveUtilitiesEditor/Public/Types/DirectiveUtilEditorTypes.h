// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilEditorTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EDirectiveUtilActorLayoutAxis : uint8
{
	X,
	Y,
	Z,
};

UENUM(BlueprintType)
enum class EDirectiveUtilActorAlignment : uint8
{
	Minimum,
	Center,
	Maximum,
};

UENUM(BlueprintType)
enum class EDirectiveUtilActorDistribution : uint8
{
	Centers,
	BoundsGaps UMETA(DisplayName = "Bounds Gaps"),
};

UENUM(BlueprintType)
enum class EDirectiveUtilSurfacePlacement : uint8
{
	Pivot,
	Bounds,
};

USTRUCT(BlueprintType)
struct FDirectiveUtilActorOperationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Actor Layout")
	TArray<TObjectPtr<AActor>> ChangedActors;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Actor Layout")
	TArray<TObjectPtr<AActor>> SkippedActors;
};

/**
 * EDirectiveUtilSelectionMethod
 *
 * The method type used to select actors within the world.
 */
UENUM(BlueprintType, Category = "Directive Utilities")
enum EDirectiveUtilSelectionMethod : uint8
{
	World UMETA(DisplayName = "World", Tooltip="Select based on the actors within the world."),
	Selection UMETA(DisplayName = "Selection", Tooltip="Select based on the actors within the current selection."),
};

/**
 * EDirectiveUtilInclusivity
 *
 * The inclusivity type used to select actors within the world.
 */
UENUM(BlueprintType, Category = "Directive Utilities")
enum EDirectiveUtilInclusivity : uint8
{
	Include UMETA(DisplayName = "Include", Tooltip="Include items based on the provided criteria."),
	Exclude UMETA(DisplayName = "Exclude", Tooltip="Exclude items based on the provided criteria."),
};

/**
 * EDirectiveUtilSearchLocation
 *
 * The object source to use.
 */
UENUM(BlueprintType, Category = "Directive Utilities")
enum EDirectiveUtilSearchLocation : uint8
{
	BaseAndOverride UMETA(DisplayName = "Base & Override",
	                      Tooltip="With search the base object along with actor overrides."),
	BaseOnly UMETA(DisplayName = "Base Only", Tooltip="Will only search the base object."),
	OverrideOnly UMETA(DisplayName = "Override Only", Tooltip="Will only search actor overrides."),
};
