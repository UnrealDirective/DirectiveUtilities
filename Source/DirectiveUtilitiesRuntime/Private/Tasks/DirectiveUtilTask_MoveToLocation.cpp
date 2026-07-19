// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Tasks/DirectiveUtilTask_MoveToLocation.h"
#include "DirectiveUtilLogChannels.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

namespace
{
	float GetNonNegativeFiniteValue(const float Value)
	{
		return FMath::IsFinite(Value) ? FMath::Max(Value, 0.0f) : 0.0f;
	}
}

UDirectiveUtilTask_MoveToLocation* UDirectiveUtilTask_MoveToLocation::MoveToLocation(
	UObject* WorldContextObject,
	AController* Controller,
	const FVector Destination,
	const float AcceptanceRadius,
	const bool bCheckStuckMovement,
	const float StuckThreshold,
	const bool bDebugLineTrace)
{
	UDirectiveUtilTask_MoveToLocation* Action = NewObject<UDirectiveUtilTask_MoveToLocation>();
	Action->Controller = Controller;
	Action->Destination = Destination;
	Action->AcceptanceRadius = GetNonNegativeFiniteValue(AcceptanceRadius);
	Action->bDebugLineTrace = bDebugLineTrace;
	Action->StuckThreshold = GetNonNegativeFiniteValue(StuckThreshold);
	Action->bCheckStuckMovement = bCheckStuckMovement;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}


void UDirectiveUtilTask_MoveToLocation::EndTask()
{
	ExecuteCompleted(false);
}

void UDirectiveUtilTask_MoveToLocation::Activate()
{
	if (bHasCompleted)
	{
		return;
	}

	APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	UWorld* World = IsValid(Pawn) ? Controller->GetWorld() : nullptr;
	if (!World)
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, or world is unavailable while moving to location. Aborting."));
		return;
	}

	TimerWorld = World;
	StartLocation = Pawn->GetActorLocation();
	LastCheckedLocation = StartLocation;
	CurrentLocation = StartLocation;

	World->GetTimerManager().SetTimer(TimerHandle, this, &UDirectiveUtilTask_MoveToLocation::CheckMoveToLocation, 0.1f, true);

	if (bCheckStuckMovement)
	{
		World->GetTimerManager().SetTimer(StuckTimerHandle, this, &UDirectiveUtilTask_MoveToLocation::CheckStuckMovement, 3.f, true);
	}

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, Destination);
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Moving controller to location (%s)."), *Destination.ToString());

	if (bDebugLineTrace)
	{
		DrawDebugLine(
			World,
			Destination + FVector(0, 0, 100),
			Destination,
			FColor::Green,
			false,
			5.0f,
			0,
			1.0f
		);
	}
}

void UDirectiveUtilTask_MoveToLocation::CheckMoveToLocation()
{
	APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	if (!IsValid(Pawn) || !TimerWorld.IsValid())
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, or world is unavailable while moving to location. Aborting."));
		return;
	}

	CurrentLocation = Pawn->GetActorLocation();
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Controller is moving to location (%s). Current distance: %f."), *Destination.ToString(), FVector::Dist(CurrentLocation, Destination));

	if (FVector::Dist(CurrentLocation, Destination) <= AcceptanceRadius)
	{
		UE_LOG(LogDirectiveUtil, Verbose, TEXT("Controller has moved to location."));
		ExecuteCompleted(true);
		return;
	}

	const UPathFollowingComponent* PathFollowing = Controller->FindComponentByClass<UPathFollowingComponent>();
	if (!PathFollowing || PathFollowing->GetStatus() == EPathFollowingStatus::Idle)
	{
		UE_LOG(LogDirectiveUtil, Verbose, TEXT("Path following has stopped. Completing move to location."));
		ExecuteCompleted(FVector::Dist(CurrentLocation, Destination) <= AcceptanceRadius);
	}
}

void UDirectiveUtilTask_MoveToLocation::CheckStuckMovement()
{
	const APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	if (!IsValid(Pawn) || !TimerWorld.IsValid())
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, or world is unavailable while moving to location. Aborting."));
		return;
	}

	if (FVector::Dist(CurrentLocation, LastCheckedLocation) < StuckThreshold)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller is stuck while moving to location. Aborting"));
		ExecuteCompleted(false);
	}

	LastCheckedLocation = CurrentLocation;
}

void UDirectiveUtilTask_MoveToLocation::ExecuteCompleted(const bool bSuccess)
{
	if (bHasCompleted)
	{
		return;
	}
	bHasCompleted = true;

	UE_LOG(LogDirectiveUtil, Log, TEXT("Movement to location completed. Success: %s."), bSuccess ? TEXT("true") : TEXT("false"));

	if (UWorld* World = TimerWorld.Get())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
		World->GetTimerManager().ClearTimer(StuckTimerHandle);
	}
	TimerWorld.Reset();

	Completed.Broadcast(bSuccess);

	Controller = nullptr;
	Destination = FVector::ZeroVector;

	SetReadyToDestroy();
}

UDirectiveUtilTask_MoveToActor* UDirectiveUtilTask_MoveToActor::MoveToActor(
	UObject* WorldContextObject,
	AController* Controller,
	AActor* Goal,
	const float AcceptanceRadius,
	const bool bCheckStuckMovement,
	const float StuckThreshold)
{
	UDirectiveUtilTask_MoveToActor* Action = NewObject<UDirectiveUtilTask_MoveToActor>();
	Action->Controller = Controller;
	Action->Goal = Goal;
	Action->AcceptanceRadius = GetNonNegativeFiniteValue(AcceptanceRadius);
	Action->StuckThreshold = GetNonNegativeFiniteValue(StuckThreshold);
	Action->bCheckStuckMovement = bCheckStuckMovement;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

void UDirectiveUtilTask_MoveToActor::EndTask()
{
	ExecuteCompleted(false);
}

void UDirectiveUtilTask_MoveToActor::Activate()
{
	if (bHasCompleted)
	{
		return;
	}

	APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	UWorld* World = IsValid(Pawn) ? Controller->GetWorld() : nullptr;
	if (!World || !IsValid(Goal))
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, goal, or world is unavailable while moving to actor. Aborting."));
		return;
	}

	TimerWorld = World;
	StartLocation = Pawn->GetActorLocation();
	LastCheckedLocation = StartLocation;
	CurrentLocation = StartLocation;

	World->GetTimerManager().SetTimer(TimerHandle, this, &UDirectiveUtilTask_MoveToActor::CheckMoveToActor, 0.1f, true);

	if (bCheckStuckMovement)
	{
		World->GetTimerManager().SetTimer(StuckTimerHandle, this, &UDirectiveUtilTask_MoveToActor::CheckStuckMovement, 3.f, true);
	}

	UAIBlueprintHelperLibrary::SimpleMoveToActor(Controller, Goal);
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Moving controller to actor (%s)."), *GetNameSafe(Goal));
}

void UDirectiveUtilTask_MoveToActor::CheckMoveToActor()
{
	APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	if (!IsValid(Pawn) || !TimerWorld.IsValid())
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, or world is unavailable while moving to actor. Aborting."));
		return;
	}

	if (!IsValid(Goal))
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Goal actor has been destroyed while moving to actor. Aborting."));
		return;
	}

	// The goal can move, so its location is re-read every poll.
	const FVector GoalLocation = Goal->GetActorLocation();
	CurrentLocation = Pawn->GetActorLocation();
	UE_LOG(LogDirectiveUtil, Verbose, TEXT("Controller is moving to actor (%s). Current distance: %f."), *GetNameSafe(Goal), FVector::Dist(CurrentLocation, GoalLocation));

	if (FVector::Dist(CurrentLocation, GoalLocation) <= AcceptanceRadius)
	{
		UE_LOG(LogDirectiveUtil, Verbose, TEXT("Controller has moved to actor."));
		ExecuteCompleted(true);
		return;
	}

	const UPathFollowingComponent* PathFollowing = Controller->FindComponentByClass<UPathFollowingComponent>();
	if (!PathFollowing || PathFollowing->GetStatus() == EPathFollowingStatus::Idle)
	{
		UE_LOG(LogDirectiveUtil, Verbose, TEXT("Path following has stopped. Completing move to actor."));
		ExecuteCompleted(FVector::Dist(CurrentLocation, GoalLocation) <= AcceptanceRadius);
	}
}

void UDirectiveUtilTask_MoveToActor::CheckStuckMovement()
{
	const APawn* Pawn = IsValid(Controller) ? Controller->GetPawn() : nullptr;
	if (!IsValid(Pawn) || !TimerWorld.IsValid())
	{
		ExecuteCompleted(false);
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller, pawn, or world is unavailable while moving to actor. Aborting."));
		return;
	}

	if (FVector::Dist(CurrentLocation, LastCheckedLocation) < StuckThreshold)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller is stuck while moving to actor. Aborting"));
		ExecuteCompleted(false);
	}

	LastCheckedLocation = CurrentLocation;
}

void UDirectiveUtilTask_MoveToActor::ExecuteCompleted(const bool bSuccess)
{
	if (bHasCompleted)
	{
		return;
	}
	bHasCompleted = true;

	UE_LOG(LogDirectiveUtil, Log, TEXT("Movement to actor completed. Success: %s."), bSuccess ? TEXT("true") : TEXT("false"));

	if (UWorld* World = TimerWorld.Get())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
		World->GetTimerManager().ClearTimer(StuckTimerHandle);
	}
	TimerWorld.Reset();

	Completed.Broadcast(bSuccess);

	Controller = nullptr;
	Goal = nullptr;

	SetReadyToDestroy();
}
