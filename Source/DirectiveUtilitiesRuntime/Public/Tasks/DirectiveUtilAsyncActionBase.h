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

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void UnbindWorldCleanup();

	TWeakObjectPtr<UWorld> RegisteredWorld;
	FDelegateHandle WorldCleanupHandle;
};

UCLASS(Abstract)
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilCancellableAsyncAction : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	virtual void RegisterWithGameInstance(const UObject* WorldContextObject) override;
	virtual void SetReadyToDestroy() override;

protected:
	virtual void BeginDestroy() override;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void UnbindWorldCleanup();

	TWeakObjectPtr<UWorld> RegisteredWorld;
	FDelegateHandle WorldCleanupHandle;
};
