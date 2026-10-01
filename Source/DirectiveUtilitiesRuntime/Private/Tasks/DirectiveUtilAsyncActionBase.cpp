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
	bReadyToDestroy = true;
	UnbindWorldCleanup();
	UnrootWithoutGameInstance();
	Super::SetReadyToDestroy();
}

void UDirectiveUtilAsyncActionBase::BeginDestroy()
{
	UnbindWorldCleanup();
	UnrootWithoutGameInstance();
	Super::BeginDestroy();
}

void UDirectiveUtilAsyncActionBase::RootWithoutGameInstance()
{
	if (!bReadyToDestroy && !bKeptAlive && !RegisteredWithGameInstance.IsValid())
	{
		AddToRoot();
		bKeptAlive = true;
	}
}

bool UDirectiveUtilAsyncActionBase::IsRootedWithoutGameInstance() const
{
	return bKeptAlive;
}

bool UDirectiveUtilAsyncActionBase::IsReadyToDestroy() const
{
	return bReadyToDestroy;
}

void UDirectiveUtilAsyncActionBase::UnrootWithoutGameInstance()
{
	if (bKeptAlive)
	{
		RemoveFromRoot();
		bKeptAlive = false;
	}
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
	bReadyToDestroy = true;
	UnbindWorldCleanup();
	UnrootWithoutGameInstance();
	Super::SetReadyToDestroy();
}

bool UDirectiveUtilCancellableAsyncAction::ShouldBroadcastDelegates() const
{
	return bKeptAlive || Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilCancellableAsyncAction::BeginDestroy()
{
	UnbindWorldCleanup();
	UnrootWithoutGameInstance();
	Super::BeginDestroy();
}

void UDirectiveUtilCancellableAsyncAction::RootWithoutGameInstance()
{
	if (!bReadyToDestroy && !bKeptAlive && !IsRegistered())
	{
		AddToRoot();
		bKeptAlive = true;
	}
}

bool UDirectiveUtilCancellableAsyncAction::IsRootedWithoutGameInstance() const
{
	return bKeptAlive;
}

bool UDirectiveUtilCancellableAsyncAction::IsReadyToDestroy() const
{
	return bReadyToDestroy;
}

void UDirectiveUtilCancellableAsyncAction::UnrootWithoutGameInstance()
{
	if (bKeptAlive)
	{
		RemoveFromRoot();
		bKeptAlive = false;
	}
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
