// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TimerHandle.h"
#include "DirectiveUtilTask_Delay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDelayCompleted);

/**
 * DirectiveUtilTask_Delay
 * A cancellable delay that can be ended early by calling EndTask.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Cancellable Delay"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_Delay : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Starts a cancellable delay.
	 * When the delay has completed, the Completed delegate is called.
	 * Call EndTask to cancel the delay before it completes.
	 *
	 * @param WorldContextObject The world context object.
	 * @param Duration The duration of the delay in seconds. Non-finite and non-positive values complete on the next timer tick.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|FlowControl",
			WorldContext = "WorldContextObject",
			DisplayName = "Cancellable Delay"
			))
	static UDirectiveUtilTask_Delay* CancellableDelay(UObject* WorldContextObject, float Duration);

	/**
	 * Ends the delay early.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FlowControl")
	void EndTask();
	virtual void Activate() override;

	// The delegate called when the delay has completed.
	UPROPERTY(BlueprintAssignable)
	FOnDelayCompleted Completed;

protected:

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	float Duration = 0.0f;
	FTimerHandle TimerHandle;

	void OnDelayComplete();
};
