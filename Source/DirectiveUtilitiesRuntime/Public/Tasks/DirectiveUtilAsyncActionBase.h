// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DirectiveUtilAsyncActionBase.generated.h"

UCLASS(Abstract)
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void RegisterWithGameInstance(const UObject* WorldContextObject) override;
	virtual void SetReadyToDestroy() override;

protected:
	virtual void BeginDestroy() override;

	// Worlds without a game instance cannot hold the action, so a derived action roots itself until it is ready to destroy.
	void RootWithoutGameInstance();
	bool IsRootedWithoutGameInstance() const;
	bool IsReadyToDestroy() const;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void UnbindWorldCleanup();
	void UnrootWithoutGameInstance();

	TWeakObjectPtr<UWorld> RegisteredWorld;
	FDelegateHandle WorldCleanupHandle;
	bool bKeptAlive = false;
	bool bReadyToDestroy = false;
};

UCLASS(Abstract)
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilCancellableAsyncAction : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	virtual void RegisterWithGameInstance(const UObject* WorldContextObject) override;
	virtual void SetReadyToDestroy() override;
	virtual bool ShouldBroadcastDelegates() const override;

protected:
	virtual void BeginDestroy() override;

	// Worlds without a game instance cannot hold the action, so a derived action roots itself until it is ready to destroy.
	void RootWithoutGameInstance();
	bool IsRootedWithoutGameInstance() const;
	bool IsReadyToDestroy() const;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void UnbindWorldCleanup();
	void UnrootWithoutGameInstance();

	TWeakObjectPtr<UWorld> RegisteredWorld;
	FDelegateHandle WorldCleanupHandle;
	bool bKeptAlive = false;
	bool bReadyToDestroy = false;
};
