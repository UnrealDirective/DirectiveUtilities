// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "DirectiveUtilTask_AsyncLoadAsset.generated.h"

struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncLoadAssetCompleted, UObject*, LoadedAsset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncLoadClassCompleted, UClass*, LoadedClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncLoadAssetsCompleted, const TArray<UObject*>&, LoadedAssets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAsyncLoadAssetsProgress, int32, LoadedCount, int32, TotalCount);

/**
 * DirectiveUtilTask_AsyncLoadAsset
 * Asynchronously loads a soft object reference and broadcasts the loaded asset, with a cancel option.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Load Asset"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_AsyncLoadAsset : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Asynchronously loads the asset referenced by a soft object pointer.
	 * The Completed delegate is called with the loaded asset on success; the Failed delegate is called with null on failure. A manual Cancel() does not broadcast Failed.
	 *
	 * @param WorldContextObject The world context object.
	 * @param Asset The soft object reference to load.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|AssetManagement",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Load Asset"
			))
	static UDirectiveUtilTask_AsyncLoadAsset* AsyncLoadAsset(UObject* WorldContextObject, const TSoftObjectPtr<UObject> Asset);

	/**
	 * Cancels the in-progress load. The Failed delegate is not broadcast for a manual cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|AssetManagement")
	void Cancel();

	virtual void Activate() override;

	// Called when the asset has finished loading. The loaded asset is valid.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadAssetCompleted Completed;

	// Called when the load failed. The loaded asset is null.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadAssetCompleted Failed;

protected:

	TSoftObjectPtr<UObject> SoftAsset;
	TSharedPtr<FStreamableHandle> StreamableHandle;

	void OnLoaded();
};

/**
 * DirectiveUtilTask_AsyncLoadClass
 * Asynchronously loads a soft class reference and broadcasts the loaded class, with a cancel option.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Load Class"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_AsyncLoadClass : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Asynchronously loads the class referenced by a soft class pointer.
	 * The Completed delegate is called with the loaded class on success; the Failed delegate is called with null on failure. A manual Cancel() does not broadcast Failed.
	 *
	 * @param WorldContextObject The world context object.
	 * @param AssetClass The soft class reference to load.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|AssetManagement",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Load Class"
			))
	static UDirectiveUtilTask_AsyncLoadClass* AsyncLoadClass(UObject* WorldContextObject, const TSoftClassPtr<UObject> AssetClass);

	/**
	 * Cancels the in-progress load. The Failed delegate is not broadcast for a manual cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|AssetManagement")
	void Cancel();

	virtual void Activate() override;

	// Called when the class has finished loading. The loaded class is valid.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadClassCompleted Completed;

	// Called when the load failed. The loaded class is null.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadClassCompleted Failed;

protected:

	TSoftClassPtr<UObject> SoftClass;
	TSharedPtr<FStreamableHandle> StreamableHandle;

	void OnLoaded();
};

/**
 * DirectiveUtilTask_AsyncLoadAssets
 * Asynchronously loads a batch of soft object references in a single request and broadcasts the
 * loaded assets, with progress updates and a cancel option.
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Load Assets"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_AsyncLoadAssets : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Asynchronously loads the assets referenced by an array of soft object pointers in a single request.
	 * The Completed delegate is called exactly once with the loaded assets in input order; entries that were
	 * unset or failed to resolve are null. An empty input array completes immediately with an empty array.
	 * A manual Cancel() does not broadcast Completed.
	 *
	 * @param WorldContextObject The world context object.
	 * @param Assets The soft object references to load.
	 */
	UFUNCTION(
		BlueprintCallable,
		meta=(
			BlueprintInternalUseOnly = "true",
			Category = "Directive Utilities|AssetManagement",
			WorldContext = "WorldContextObject",
			DisplayName = "Async Load Assets"
			))
	static UDirectiveUtilTask_AsyncLoadAssets* AsyncLoadAssets(UObject* WorldContextObject, const TArray<TSoftObjectPtr<UObject>>& Assets);

	/**
	 * Cancels the in-progress load. The Completed delegate is not broadcast for a manual cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|AssetManagement")
	void Cancel();

	virtual void Activate() override;

	// Called exactly once when the batch has finished loading. The assets are in input order with null
	// entries for references that were unset or failed to resolve, and are only guaranteed alive during
	// this broadcast.
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadAssetsCompleted Completed;

	// Called as assets arrive, with the number loaded so far and the total requested. Not called when
	// the request finishes before the first update (e.g. all assets were already in memory).
	UPROPERTY(BlueprintAssignable)
	FOnAsyncLoadAssetsProgress Progress;

protected:

	TArray<TSoftObjectPtr<UObject>> SoftAssets;
	TSharedPtr<FStreamableHandle> StreamableHandle;

	/* Whether the task has already completed, guarding against a second broadcast. */
	bool bHasCompleted = false;

	void OnLoaded();
	void OnUpdate(TSharedRef<FStreamableHandle> Handle);
};
