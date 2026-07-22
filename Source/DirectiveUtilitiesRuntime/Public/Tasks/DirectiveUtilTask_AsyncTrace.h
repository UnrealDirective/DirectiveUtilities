// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/DirectiveUtilAsyncActionBase.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "DirectiveUtilTask_AsyncTrace.generated.h"

struct FTraceHandle;
struct FTraceDatum;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncTraceCompleted, const TArray<FHitResult>&, Hits);

/** The collision shape used by an async trace. */
enum class EDirectiveUtilTraceShape : uint8
{
	Line,
	Sphere,
	Box,
	Capsule,
};

/**
 * DirectiveUtilTask_AsyncTrace
 * Queues a collision trace and broadcasts the hit results on the next tick.
 * The trace cannot be cancelled.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Trace"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_AsyncTrace : public UDirectiveUtilAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Performs an asynchronous line trace against the given trace channel.
	 * @param WorldContextObject The world context object.
	 * @param Start The start of the trace.
	 * @param End The end of the trace.
	 * @param TraceChannel The trace channel to test against.
	 * @param bMultiTrace If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Collision",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Line Trace By Channel"
			))
	static UDirectiveUtilTask_AsyncTrace* AsyncLineTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);

	/**
	 * Performs an asynchronous sphere sweep against the given trace channel.
	 * @param WorldContextObject The world context object.
	 * @param Start The start of the sweep.
	 * @param End The end of the sweep.
	 * @param Radius The radius of the sphere.
	 * @param TraceChannel The trace channel to test against.
	 * @param bMultiTrace If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Collision",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Sphere Trace By Channel"
			))
	static UDirectiveUtilTask_AsyncTrace* AsyncSphereTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);

	/**
	 * Performs an asynchronous box sweep against the given trace channel.
	 * @param WorldContextObject The world context object.
	 * @param Start The start of the sweep.
	 * @param End The end of the sweep.
	 * @param HalfSize The half-extents of the box.
	 * @param Orientation The orientation of the box.
	 * @param TraceChannel The trace channel to test against.
	 * @param bMultiTrace If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Collision",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Box Trace By Channel"
			))
	static UDirectiveUtilTask_AsyncTrace* AsyncBoxTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const FVector HalfSize, const FRotator Orientation, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);

	/**
	 * Performs an asynchronous capsule sweep against the given trace channel.
	 * @param WorldContextObject The world context object.
	 * @param Start The start of the sweep.
	 * @param End The end of the sweep.
	 * @param Radius The radius of the capsule.
	 * @param HalfHeight The half-height of the capsule (including the radius).
	 * @param TraceChannel The trace channel to test against.
	 * @param bMultiTrace If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|Collision",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Capsule Trace By Channel"
			))
	static UDirectiveUtilTask_AsyncTrace* AsyncCapsuleTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const float HalfHeight, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);

	virtual void Activate() override;

	// Called when the trace has completed. Empty if nothing was hit.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncTraceCompleted Completed;

protected:

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	ETraceTypeQuery TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
	bool bMultiTrace = false;
	EDirectiveUtilTraceShape Shape = EDirectiveUtilTraceShape::Line;
	float Radius = 0.0f;
	float HalfHeight = 0.0f;
	FVector HalfSize = FVector::ZeroVector;
	FQuat Orientation = FQuat::Identity;

	void OnTraceComplete(const FTraceHandle& Handle, FTraceDatum& Datum);
};
