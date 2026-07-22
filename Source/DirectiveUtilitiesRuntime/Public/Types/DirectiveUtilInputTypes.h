// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilInputTypes.generated.h"

class UInputMappingContext;

/** An input mapping context and its application priority. */
USTRUCT(BlueprintType)
struct FDirectiveUtilEnhancedInputContextData
{
	GENERATED_BODY()

	/** The input context to be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputContext;

	/** The priority of the input context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 Priority;

	FDirectiveUtilEnhancedInputContextData()
		: Priority(0)
	{
	}
};
