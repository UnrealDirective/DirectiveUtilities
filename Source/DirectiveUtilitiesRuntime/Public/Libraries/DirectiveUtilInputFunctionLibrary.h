// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilInputTypes.h"
#include "Types/DirectiveUtilTypes.h"
#include "DirectiveUtilInputFunctionLibrary.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
/**
 * UDirectiveUtilInputFunctionLibrary
 * A function library that adds functionality for working with input in Unreal Engine.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Apply multiple Input Mapping Contexts.
	* @param PlayerController The player controller to add the contexts to. Will attempt to get the LocalPlayer from the controller.
	* @param Contexts The contexts to apply.
	* @param bClearPrevious Whether to clear all previous contexts before applying the new ones.
	* @returns Returns Success if the contexts were successfully applied.
	*/
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EDirectiveUtilSuccessStatus AddInputMappingContexts(
		AController* PlayerController,
		const TArray<FDirectiveUtilEnhancedInputContextData>& Contexts,
		bool bClearPrevious);

	/**
	 * Remove multiple Input Mapping Contexts.
	 * @param PlayerController The player controller to remove the contexts from. Will attempt to get the LocalPlayer from the controller.
	 * @param Contexts The contexts to remove.
	 * @returns Returns Success if the contexts were successfully removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EDirectiveUtilSuccessStatus RemoveInputMappingContexts(
		AController* PlayerController,
		const TArray<TSoftObjectPtr<UInputMappingContext>>& Contexts);

	/**
	* Swap a designated Input Mapping Context with a new one.
	* If the previous context is found, it will be removed and the new context will be added.
	* If the previous context is not found, the new context will be added at the specified priority.
	* @param PlayerController The player controller to swap the contexts on. Will attempt to get the LocalPlayer from the controller.
	* @param PreviousContext The context to swap out.
	* @param NewContext The context to swap in.
	* @param Priority The priority to set the new context to.
	* @param bUsePreviousPriority Whether to use the previous context's priority when adding the new context.
	* @returns Returns Success if the contexts were successfully swapped.
    */
    UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EDirectiveUtilSuccessStatus SwapInputMappingContexts(
		AController* PlayerController,
		TSoftObjectPtr<UInputMappingContext> PreviousContext,
		TSoftObjectPtr<UInputMappingContext> NewContext,
		int32 Priority,
		bool bUsePreviousPriority);

	/**
	 * Returns the Enhanced Input local player subsystem for the given controller, if available.
	 * @param PlayerController - The player controller; the Local Player's subsystem is retrieved.
	 * @returns The Enhanced Input subsystem, or null if the controller has no associated local player subsystem.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Input", meta=(DefaultToSelf="PlayerController"))
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* PlayerController);

	/**
	 * Returns whether the given input mapping context is currently active on the controller.
	 * @param PlayerController - The player controller to query.
	 * @param Context - The input mapping context to check.
	 * @returns True if the context is currently applied.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Input", meta=(DefaultToSelf="PlayerController"))
	static bool IsInputMappingContextActive(AController* PlayerController, TSoftObjectPtr<UInputMappingContext> Context);

	/**
	 * Removes all input mapping contexts from the controller.
	 * @param PlayerController - The player controller to clear.
	 * @returns Success if the contexts were cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EDirectiveUtilSuccessStatus ClearAllInputMappingContexts(AController* PlayerController);

protected:
	/**
	 * Attempt to get the Enhanced Input Subsystem from the provided controller.
	 * @param PlayerController - The player controller to get the subsystem from.
	 * @param EnhancedInput - The Enhanced Input Subsystem to return
	 * @return True if the subsystem was found.
	 */
	static bool TryGetEnhancedInputSubsystemFromController(
		AController* PlayerController,
		UEnhancedInputLocalPlayerSubsystem*& EnhancedInput);
};
