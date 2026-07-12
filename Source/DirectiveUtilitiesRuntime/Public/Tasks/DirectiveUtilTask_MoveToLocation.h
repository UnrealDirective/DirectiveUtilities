// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameFramework/Controller.h"
#include "DirectiveUtilTask_MoveToLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncMoveToLocation, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncMoveToActor, bool, bSuccess);

/**
 * DirectiveUtilTask_MoveToLocation
 * Asynchronously moves an actor to a location.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Move To Location"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_MoveToLocation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Moves the actor to the specified location.
	 * When the movement has succeeded or failed, the Completed delegate is called exactly once with success/failure.
	 * bSuccess is true only when the pawn ends within AcceptanceRadius of Destination; the task also ends
	 * (with the same distance test) when path-following stops for any reason.
	 *
	 * If the controller or pawn is destroyed while moving, the task will automatically end.
	 * If bCheckStuckMovement is enabled and the controller gets stuck while moving, the task will automatically end.
	 *
	 * @param WorldContextObject The world context object.
	 * @param Controller The controller to move.
	 * @param Destination The vector location to move to.
	 * @param AcceptanceRadius The radius around the destination location that is considered acceptable. Be sure to set this to a reasonable value as the controller may never reach the exact destination.
	 * @param bCheckStuckMovement Check if the controller gets stuck while moving.
	 * @param StuckThreshold The distance threshold to consider the controller stuck.
	 * @param bDebugLineTrace Display a line trace to the destination location for a short duration.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Navigation",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Move To Location",
			AdvancedDisplay=6
			))
	static UDirectiveUtilTask_MoveToLocation* MoveToLocation(
		UObject* WorldContextObject,
		AController* Controller,
		FVector Destination,
		float AcceptanceRadius = 100.0f,
		bool bCheckStuckMovement = true,
		float StuckThreshold = 1.0f,
		bool bDebugLineTrace = false);

	/**
	 * Ends the async action.
	 * This must be called manually when the task is no longer necessary.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Navigation")
	void EndTask();
	virtual void Activate() override;

	// The delegate called when the movement has completed regardless of success. Fires exactly once.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncMoveToLocation Completed;

protected:

	UPROPERTY()
	AController* Controller;

	FVector Destination;
	FVector StartLocation;
	FVector CurrentLocation;
	FVector LastCheckedLocation;
	float AcceptanceRadius = 10.0f;
	bool bCheckStuckMovement = true;
	float StuckThreshold = 1.0f;
	bool bDebugLineTrace;

	FTimerHandle TimerHandle;

	FTimerHandle StuckTimerHandle;

	bool bHasCompleted = false;

	void CheckMoveToLocation();

	void CheckStuckMovement();

	virtual void ExecuteCompleted(bool bSuccess);
};

/**
 * DirectiveUtilTask_MoveToActor
 * Asynchronously moves an actor to another actor.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Move To Actor"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_MoveToActor : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Moves the controller's pawn to the goal actor.
	 * When the movement has succeeded or failed, the Completed delegate is called exactly once with success/failure.
	 * bSuccess is true only when the pawn ends within AcceptanceRadius of the goal actor; the task also ends
	 * (with the same distance test) when path-following stops for any reason. The goal's location is re-read
	 * every poll, so a moving goal is tracked.
	 *
	 * If the controller, pawn, or goal actor is destroyed while moving, the task will automatically end.
	 * If bCheckStuckMovement is enabled and the controller gets stuck while moving, the task will automatically end.
	 *
	 * @param WorldContextObject The world context object.
	 * @param Controller The controller to move.
	 * @param Goal The actor to move to.
	 * @param AcceptanceRadius The radius around the goal actor that is considered acceptable. Be sure to set this to a reasonable value as the controller may never reach the goal's exact location.
	 * @param bCheckStuckMovement Check if the controller gets stuck while moving.
	 * @param StuckThreshold The distance threshold to consider the controller stuck.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Navigation",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Move To Actor"
			))
	static UDirectiveUtilTask_MoveToActor* MoveToActor(
		UObject* WorldContextObject,
		AController* Controller,
		AActor* Goal,
		float AcceptanceRadius = 100.0f,
		bool bCheckStuckMovement = true,
		float StuckThreshold = 1.0f);

	/**
	 * Ends the async action.
	 * This must be called manually when the task is no longer necessary.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Navigation")
	void EndTask();
	virtual void Activate() override;

	// The delegate called when the movement has completed regardless of success. Fires exactly once.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncMoveToActor Completed;

protected:

	UPROPERTY()
	AController* Controller;

	// The cached goal actor; GC nulls it if the actor is destroyed.
	UPROPERTY()
	TObjectPtr<AActor> Goal;

	FVector StartLocation;
	FVector CurrentLocation;
	FVector LastCheckedLocation;
	float AcceptanceRadius = 10.0f;
	bool bCheckStuckMovement = true;
	float StuckThreshold = 1.0f;

	FTimerHandle TimerHandle;

	FTimerHandle StuckTimerHandle;

	bool bHasCompleted = false;

	void CheckMoveToActor();

	void CheckStuckMovement();

	virtual void ExecuteCompleted(bool bSuccess);
};
