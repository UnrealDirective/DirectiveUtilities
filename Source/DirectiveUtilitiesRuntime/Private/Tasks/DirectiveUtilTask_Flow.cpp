// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_Flow.h"

#include "DirectiveUtilLogChannels.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDirectiveUtilTask_UpdateForDuration* UDirectiveUtilTask_UpdateForDuration::UpdateForDuration(
	UObject* WorldContextObject,
	const float Duration,
	const float UpdateInterval)
{
	UDirectiveUtilTask_UpdateForDuration* Action = NewObject<UDirectiveUtilTask_UpdateForDuration>();
	Action->WorldContextObject = WorldContextObject;
	Action->Duration = Duration;
	Action->UpdateInterval = UpdateInterval;
	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}
	return Action;
}

void UDirectiveUtilTask_UpdateForDuration::Activate()
{
	UWorld* World = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Update for Duration failed to activate. World is null."));
		bFinished = true;
		SetReadyToDestroy();
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (!FMath::IsFinite(Duration) || Duration <= 0.0f)
	{
		CompletionTimerHandle = TimerManager.SetTimerForNextTick(this, &UDirectiveUtilTask_UpdateForDuration::OnComplete);
		return;
	}

	TimerManager.SetTimer(
		CompletionTimerHandle,
		this,
		&UDirectiveUtilTask_UpdateForDuration::OnComplete,
		Duration,
		false);

	if (!FMath::IsFinite(UpdateInterval) || UpdateInterval <= 0.0f)
	{
		UpdateTimerHandle = TimerManager.SetTimerForNextTick(this, &UDirectiveUtilTask_UpdateForDuration::OnUpdate);
		return;
	}

	FTimerManagerTimerParameters UpdateParameters;
	UpdateParameters.bLoop = true;
	UpdateParameters.bMaxOncePerFrame = true;
	UpdateParameters.FirstDelay = UpdateInterval;
	TimerManager.SetTimer(
		UpdateTimerHandle,
		this,
		&UDirectiveUtilTask_UpdateForDuration::OnUpdate,
		UpdateInterval,
		UpdateParameters);
}

void UDirectiveUtilTask_UpdateForDuration::Cancel()
{
	bFinished = true;
	ClearTimers();
	Updated.Clear();
	Completed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_UpdateForDuration::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_UpdateForDuration::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_UpdateForDuration::OnUpdate()
{
	if (!ShouldBroadcastDelegates())
	{
		ClearTimers();
		return;
	}

	FTimerManager* TimerManager = GetTimerManager();
	if (!TimerManager)
	{
		Cancel();
		return;
	}

	const float RemainingTime = TimerManager->GetTimerRemaining(CompletionTimerHandle);
	const float ElapsedTime = RemainingTime >= 0.0f
		? FMath::Clamp(Duration - RemainingTime, 0.0f, Duration)
		: Duration;
	BroadcastUpdate(ElapsedTime, ElapsedTime / Duration);
	if (ShouldBroadcastDelegates() && (!FMath::IsFinite(UpdateInterval) || UpdateInterval <= 0.0f))
	{
		UpdateTimerHandle = TimerManager->SetTimerForNextTick(this, &UDirectiveUtilTask_UpdateForDuration::OnUpdate);
	}
}

void UDirectiveUtilTask_UpdateForDuration::OnComplete()
{
	if (!ShouldBroadcastDelegates())
	{
		ClearTimers();
		return;
	}

	if (!bHasUpdated || LastElapsedTime < Duration)
	{
		const float FinalElapsedTime = FMath::IsFinite(Duration) && Duration > 0.0f ? Duration : 0.0f;
		BroadcastUpdate(FinalElapsedTime, 1.0f);
	}
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	ClearTimers();
	Completed.Broadcast();
	bFinished = true;
	SetReadyToDestroy();
}

void UDirectiveUtilTask_UpdateForDuration::BroadcastUpdate(const float ElapsedTime, const float Alpha)
{
	const float DeltaTime = bHasUpdated ? FMath::Max(ElapsedTime - LastElapsedTime, 0.0f) : ElapsedTime;
	LastElapsedTime = ElapsedTime;
	bHasUpdated = true;
	Updated.Broadcast(ElapsedTime, DeltaTime, Alpha);
}

void UDirectiveUtilTask_UpdateForDuration::ClearTimers()
{
	if (FTimerManager* TimerManager = GetTimerManager())
	{
		TimerManager->ClearTimer(UpdateTimerHandle);
		TimerManager->ClearTimer(CompletionTimerHandle);
	}
}

UDirectiveUtilTask_RepeatWithInterval* UDirectiveUtilTask_RepeatWithInterval::RepeatWithInterval(
	UObject* WorldContextObject,
	const int32 Count,
	const float Interval,
	const float InitialDelay)
{
	UDirectiveUtilTask_RepeatWithInterval* Action = NewObject<UDirectiveUtilTask_RepeatWithInterval>();
	Action->WorldContextObject = WorldContextObject;
	Action->Count = Count;
	Action->Interval = Interval;
	Action->InitialDelay = InitialDelay;
	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}
	return Action;
}

void UDirectiveUtilTask_RepeatWithInterval::Activate()
{
	UWorld* World = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Repeat with Interval failed to activate. World is null."));
		bFinished = true;
		SetReadyToDestroy();
		return;
	}

	if (Count <= 0)
	{
		TimerHandle = World->GetTimerManager().SetTimerForNextTick(this, &UDirectiveUtilTask_RepeatWithInterval::Complete);
		return;
	}

	const float SafeInitialDelay = FMath::IsFinite(InitialDelay) && InitialDelay > 0.0f
		? InitialDelay
		: 0.0f;
	Schedule(SafeInitialDelay);
}

void UDirectiveUtilTask_RepeatWithInterval::Cancel()
{
	bFinished = true;
	ClearTimer();
	Iteration.Clear();
	Completed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_RepeatWithInterval::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_RepeatWithInterval::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_RepeatWithInterval::Schedule(const float Delay)
{
	FTimerManager* TimerManager = GetTimerManager();
	if (!TimerManager)
	{
		Cancel();
		return;
	}

	if (Delay > 0.0f)
	{
		TimerManager->SetTimer(TimerHandle, this, &UDirectiveUtilTask_RepeatWithInterval::OnIteration, Delay, false);
	}
	else
	{
		TimerHandle = TimerManager->SetTimerForNextTick(this, &UDirectiveUtilTask_RepeatWithInterval::OnIteration);
	}
}

void UDirectiveUtilTask_RepeatWithInterval::OnIteration()
{
	if (!ShouldBroadcastDelegates())
	{
		ClearTimer();
		return;
	}

	const int32 CurrentIndex = NextIndex++;
	Iteration.Broadcast(CurrentIndex, Count - NextIndex);
	if (!ShouldBroadcastDelegates())
	{
		return;
	}
	if (NextIndex >= Count)
	{
		Complete();
		return;
	}

	const float SafeInterval = FMath::IsFinite(Interval) && Interval > 0.0f
		? Interval
		: 0.0f;
	Schedule(SafeInterval);
}

void UDirectiveUtilTask_RepeatWithInterval::Complete()
{
	if (!ShouldBroadcastDelegates())
	{
		ClearTimer();
		return;
	}

	ClearTimer();
	Completed.Broadcast();
	bFinished = true;
	SetReadyToDestroy();
}

void UDirectiveUtilTask_RepeatWithInterval::ClearTimer()
{
	if (FTimerManager* TimerManager = GetTimerManager())
	{
		TimerManager->ClearTimer(TimerHandle);
	}
}
