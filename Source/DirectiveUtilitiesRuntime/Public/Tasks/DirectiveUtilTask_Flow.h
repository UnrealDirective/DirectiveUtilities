// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/DirectiveUtilAsyncActionBase.h"
#include "Engine/TimerHandle.h"
#include "DirectiveUtilTask_Flow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDurationUpdated, float, ElapsedTime, float, DeltaTime, float, Alpha);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDurationCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRepeatIteration, int32, Index, int32, Remaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepeatCompleted);

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Update for Duration"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_UpdateForDuration : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|FlowControl",
			WorldContext = "WorldContextObject",
			DisplayName = "Update for Duration",
			AdvancedDisplay = "UpdateInterval"
			))
	static UDirectiveUtilTask_UpdateForDuration* UpdateForDuration(
		UObject* WorldContextObject,
		float Duration,
		float UpdateInterval = 0.0f);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	UPROPERTY(BlueprintAssignable)
	FOnDurationUpdated Updated;

	UPROPERTY(BlueprintAssignable)
	FOnDurationCompleted Completed;

private:
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	float Duration = 0.0f;
	float UpdateInterval = 0.0f;
	float LastElapsedTime = 0.0f;
	bool bHasUpdated = false;
	bool bFinished = false;
	FTimerHandle UpdateTimerHandle;
	FTimerHandle CompletionTimerHandle;

	void OnUpdate();
	void OnComplete();
	void BroadcastUpdate(float ElapsedTime, float Alpha);
	void ClearTimers();
};

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Repeat with Interval"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_RepeatWithInterval : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|FlowControl",
			WorldContext = "WorldContextObject",
			DisplayName = "Repeat with Interval",
			AdvancedDisplay = "InitialDelay"
			))
	static UDirectiveUtilTask_RepeatWithInterval* RepeatWithInterval(
		UObject* WorldContextObject,
		int32 Count,
		float Interval,
		float InitialDelay = 0.0f);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	UPROPERTY(BlueprintAssignable)
	FOnRepeatIteration Iteration;

	UPROPERTY(BlueprintAssignable)
	FOnRepeatCompleted Completed;

private:
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	int32 Count = 0;
	int32 NextIndex = 0;
	float Interval = 0.0f;
	float InitialDelay = 0.0f;
	bool bFinished = false;
	FTimerHandle TimerHandle;

	void Schedule(float Delay);
	void OnIteration();
	void Complete();
	void ClearTimer();
};
