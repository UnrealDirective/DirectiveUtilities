// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilTextFunctionLibrary.generated.h"

/**
 * DirectiveUtilTextFunctionLibrary
 * A collection of helpful text utility functions that improve the usability of text in Blueprints.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTextFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns true if the provided text is not empty.
	 * @param Text - The text to check.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Text" )
	static bool IsNotEmpty(const FText& Text);
};
