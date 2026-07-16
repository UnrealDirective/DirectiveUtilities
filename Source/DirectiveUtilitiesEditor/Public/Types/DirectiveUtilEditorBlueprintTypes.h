// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilEditorBlueprintTypes.generated.h"

UENUM(BlueprintType)
enum class EDirectiveUtilBlueprintCompileStatus : uint8
{
	Unknown,
	Dirty,
	Error,
	UpToDate UMETA(DisplayName = "Up To Date"),
	BeingCreated UMETA(DisplayName = "Being Created"),
	UpToDateWithWarnings UMETA(DisplayName = "Up To Date With Warnings"),
};

USTRUCT(BlueprintType)
struct FDirectiveUtilBlueprintSearchOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directive Utilities|Blueprint Inspection")
	TArray<FName> PackagePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directive Utilities|Blueprint Inspection")
	TArray<FName> ExcludedPackagePaths;

	FDirectiveUtilBlueprintSearchOptions()
	{
		PackagePaths.Add(TEXT("/Game"));
	}
};
