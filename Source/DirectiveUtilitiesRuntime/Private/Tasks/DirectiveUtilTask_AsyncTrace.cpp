// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Tasks/DirectiveUtilTask_AsyncTrace.h"
#include "DirectiveUtilLogChannels.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "WorldCollision.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"

UDirectiveUtilTask_AsyncTrace* UDirectiveUtilTask_AsyncTrace::AsyncLineTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const ETraceTypeQuery TraceChannel, const bool bMultiTrace)
{
	UDirectiveUtilTask_AsyncTrace* Action = NewObject<UDirectiveUtilTask_AsyncTrace>();
	Action->WorldContextObject = WorldContextObject;
	Action->Start = Start;
	Action->End = End;
	Action->TraceChannel = TraceChannel;
	Action->bMultiTrace = bMultiTrace;
	Action->Shape = EDirectiveUtilTraceShape::Line;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

UDirectiveUtilTask_AsyncTrace* UDirectiveUtilTask_AsyncTrace::AsyncSphereTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const ETraceTypeQuery TraceChannel, const bool bMultiTrace)
{
	UDirectiveUtilTask_AsyncTrace* Action = NewObject<UDirectiveUtilTask_AsyncTrace>();
	Action->WorldContextObject = WorldContextObject;
	Action->Start = Start;
	Action->End = End;
	Action->Radius = Radius;
	Action->TraceChannel = TraceChannel;
	Action->bMultiTrace = bMultiTrace;
	Action->Shape = EDirectiveUtilTraceShape::Sphere;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

UDirectiveUtilTask_AsyncTrace* UDirectiveUtilTask_AsyncTrace::AsyncBoxTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const FVector HalfSize, const FRotator Orientation, const ETraceTypeQuery TraceChannel, const bool bMultiTrace)
{
	UDirectiveUtilTask_AsyncTrace* Action = NewObject<UDirectiveUtilTask_AsyncTrace>();
	Action->WorldContextObject = WorldContextObject;
	Action->Start = Start;
	Action->End = End;
	Action->HalfSize = HalfSize;
	Action->Orientation = Orientation.Quaternion();
	Action->TraceChannel = TraceChannel;
	Action->bMultiTrace = bMultiTrace;
	Action->Shape = EDirectiveUtilTraceShape::Box;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

UDirectiveUtilTask_AsyncTrace* UDirectiveUtilTask_AsyncTrace::AsyncCapsuleTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const float HalfHeight, const ETraceTypeQuery TraceChannel, const bool bMultiTrace)
{
	UDirectiveUtilTask_AsyncTrace* Action = NewObject<UDirectiveUtilTask_AsyncTrace>();
	Action->WorldContextObject = WorldContextObject;
	Action->Start = Start;
	Action->End = End;
	Action->Radius = Radius;
	Action->HalfHeight = HalfHeight;
	Action->TraceChannel = TraceChannel;
	Action->bMultiTrace = bMultiTrace;
	Action->Shape = EDirectiveUtilTraceShape::Capsule;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

void UDirectiveUtilTask_AsyncTrace::Activate()
{
	UWorld* World = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Trace failed to activate. World is null."));
		Completed.Broadcast(TArray<FHitResult>());
		SetReadyToDestroy();
		return;
	}

	const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
	const EAsyncTraceType AsyncType = bMultiTrace ? EAsyncTraceType::Multi : EAsyncTraceType::Single;

	FTraceDelegate TraceDelegate;
	TraceDelegate.BindUObject(this, &UDirectiveUtilTask_AsyncTrace::OnTraceComplete);

	FCollisionQueryParams Params(FName(TEXT("DirectiveUtilAsyncTrace")), false);

	if (Shape == EDirectiveUtilTraceShape::Line)
	{
		World->AsyncLineTraceByChannel(AsyncType, Start, End, CollisionChannel, Params, FCollisionResponseParams::DefaultResponseParam, &TraceDelegate);
	}
	else
	{
		FCollisionShape CollisionShape;
		switch (Shape)
		{
		case EDirectiveUtilTraceShape::Sphere:
			CollisionShape = FCollisionShape::MakeSphere(Radius);
			break;
		case EDirectiveUtilTraceShape::Box:
			CollisionShape = FCollisionShape::MakeBox(HalfSize);
			break;
		case EDirectiveUtilTraceShape::Capsule:
			CollisionShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
			break;
		default:
			break;
		}
		World->AsyncSweepByChannel(AsyncType, Start, End, Orientation, CollisionChannel, CollisionShape, Params, FCollisionResponseParams::DefaultResponseParam, &TraceDelegate);
	}
}

void UDirectiveUtilTask_AsyncTrace::OnTraceComplete(const FTraceHandle& Handle, FTraceDatum& Datum)
{
	Completed.Broadcast(Datum.OutHits);
	SetReadyToDestroy();
}
