// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameFramework/Controller.h"
#include "UDAT_MoveToLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncMoveToLocation, bool, bSuccess);

/**
 * UDAT_MoveToLocation
 * Asynchronously moves an actor to a location.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Move To Location"))
class UDCORE_API UUDAT_MoveToLocation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	
	/**
	 * Moves the actor to the specified location.
	 * When the movement has succeeded or failed, the Completed delegate is called with success/failure.
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
			Category = "UDCore|AI|Navigation",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Move To Location",
			AdvancedDisplay=6
			))
	static UUDAT_MoveToLocation* MoveToLocation(
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
	UFUNCTION(BlueprintCallable, Category = "UDCore|AI|Navigation")
	void EndTask();
	virtual void Activate() override;

	// The delegate called when the movement has completed regardless of success.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncMoveToLocation Completed;

protected:

	// The cached controller to move.
	UPROPERTY()
	AController* Controller;
	
	// The cached data
	FVector Destination;
	FVector StartLocation;
	FVector CurrentLocation;
	FVector LastCheckedLocation;
	float AcceptanceRadius = 10.0f;
	bool bCheckStuckMovement = true;
	float StuckThreshold = 1.0f;
	bool bDebugLineTrace;

	/* The timer handle to check for completed movement. */
	FTimerHandle TimerHandle;

	/* The timer handle to check for stuck movement. */
	FTimerHandle StuckTimerHandle;

	/** Handles the movement to the destination. */
	void CheckMoveToLocation();

	/** Handles the stuck movement. */
	void CheckStuckMovement();

	/* Called at completion of movement to destination. */
	virtual void ExecuteCompleted(bool bSuccess);
};
