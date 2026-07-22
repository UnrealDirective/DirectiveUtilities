// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilAsyncActionBase.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

void UDirectiveUtilAsyncActionBase::RegisterWithGameInstance(const UObject* WorldContextObject)
{
	UnbindWorldCleanup();
	Super::RegisterWithGameInstance(WorldContextObject);
	RegisteredWorld = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (RegisteredWorld.IsValid())
	{
		WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(
			this,
			&UDirectiveUtilAsyncActionBase::HandleWorldCleanup);
	}
}

void UDirectiveUtilAsyncActionBase::SetReadyToDestroy()
{
	UnbindWorldCleanup();
	Super::SetReadyToDestroy();
}

void UDirectiveUtilAsyncActionBase::BeginDestroy()
{
	UnbindWorldCleanup();
	Super::BeginDestroy();
}

void UDirectiveUtilAsyncActionBase::HandleWorldCleanup(
	UWorld* World,
	const bool,
	const bool)
{
	if (World == RegisteredWorld.Get())
	{
		SetReadyToDestroy();
	}
}

void UDirectiveUtilAsyncActionBase::UnbindWorldCleanup()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}
	RegisteredWorld.Reset();
}

void UDirectiveUtilCancellableAsyncAction::RegisterWithGameInstance(const UObject* WorldContextObject)
{
	UnbindWorldCleanup();
	Super::RegisterWithGameInstance(WorldContextObject);
	RegisteredWorld = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (RegisteredWorld.IsValid())
	{
		WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(
			this,
			&UDirectiveUtilCancellableAsyncAction::HandleWorldCleanup);
	}
}

void UDirectiveUtilCancellableAsyncAction::SetReadyToDestroy()
{
	UnbindWorldCleanup();
	Super::SetReadyToDestroy();
}

void UDirectiveUtilCancellableAsyncAction::BeginDestroy()
{
	UnbindWorldCleanup();
	Super::BeginDestroy();
}

void UDirectiveUtilCancellableAsyncAction::HandleWorldCleanup(
	UWorld* World,
	const bool,
	const bool)
{
	if (World == RegisteredWorld.Get())
	{
		Cancel();
	}
}

void UDirectiveUtilCancellableAsyncAction::UnbindWorldCleanup()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}
	RegisteredWorld.Reset();
}
