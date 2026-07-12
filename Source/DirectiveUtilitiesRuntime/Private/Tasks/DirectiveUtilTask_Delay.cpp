// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Tasks/DirectiveUtilTask_Delay.h"
#include "DirectiveUtilLogChannels.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"


UDirectiveUtilTask_Delay* UDirectiveUtilTask_Delay::CancellableDelay(UObject* WorldContextObject, const float Duration)
{
	UDirectiveUtilTask_Delay* Action = NewObject<UDirectiveUtilTask_Delay>();
	Action->WorldContextObject = WorldContextObject;
	Action->Duration = Duration;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}


void UDirectiveUtilTask_Delay::EndTask()
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	SetReadyToDestroy();
}

void UDirectiveUtilTask_Delay::Activate()
{
	UWorld* World = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Cancellable Delay failed to activate. World is null."));
		SetReadyToDestroy();
		return;
	}

	const float ClampedDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	World->GetTimerManager().SetTimer(TimerHandle, this, &UDirectiveUtilTask_Delay::OnDelayComplete, ClampedDuration, false);
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Cancellable Delay started for %f seconds."), ClampedDuration);
}

void UDirectiveUtilTask_Delay::OnDelayComplete()
{
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Cancellable Delay completed."));
	Completed.Broadcast();
	SetReadyToDestroy();
}
