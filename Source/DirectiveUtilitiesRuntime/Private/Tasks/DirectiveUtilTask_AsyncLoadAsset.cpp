// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Tasks/DirectiveUtilTask_AsyncLoadAsset.h"
#include "DirectiveUtilLogChannels.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UDirectiveUtilTask_AsyncLoadAsset* UDirectiveUtilTask_AsyncLoadAsset::AsyncLoadAsset(UObject* WorldContextObject, const TSoftObjectPtr<UObject> Asset)
{
	UDirectiveUtilTask_AsyncLoadAsset* Action = NewObject<UDirectiveUtilTask_AsyncLoadAsset>();
	Action->SoftAsset = Asset;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

void UDirectiveUtilTask_AsyncLoadAsset::Activate()
{
	if (SoftAsset.IsNull())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Asset failed to activate. The soft object reference is null."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	if (!UAssetManager::GetIfInitialized())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Asset failed to activate. The Asset Manager is not initialized."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	StreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftAsset.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UDirectiveUtilTask_AsyncLoadAsset::OnLoaded),
		FStreamableManager::DefaultAsyncLoadPriority);

	if (!StreamableHandle.IsValid())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Asset failed to start the load request."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
	}
}

void UDirectiveUtilTask_AsyncLoadAsset::OnLoaded()
{
	UObject* LoadedAsset = StreamableHandle.IsValid() ? StreamableHandle->GetLoadedAsset() : nullptr;
	if (LoadedAsset)
	{
		Completed.Broadcast(LoadedAsset);
	}
	else
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Asset completed but the asset could not be resolved."));
		Failed.Broadcast(nullptr);
	}
	SetReadyToDestroy();
}

void UDirectiveUtilTask_AsyncLoadAsset::Cancel()
{
	if (StreamableHandle.IsValid() && StreamableHandle->IsActive())
	{
		StreamableHandle->CancelHandle();
	}
	SetReadyToDestroy();
}

UDirectiveUtilTask_AsyncLoadClass* UDirectiveUtilTask_AsyncLoadClass::AsyncLoadClass(UObject* WorldContextObject, const TSoftClassPtr<UObject> AssetClass)
{
	UDirectiveUtilTask_AsyncLoadClass* Action = NewObject<UDirectiveUtilTask_AsyncLoadClass>();
	Action->SoftClass = AssetClass;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

void UDirectiveUtilTask_AsyncLoadClass::Activate()
{
	if (SoftClass.IsNull())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Class failed to activate. The soft class reference is null."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	if (!UAssetManager::GetIfInitialized())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Class failed to activate. The Asset Manager is not initialized."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
		return;
	}

	StreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UDirectiveUtilTask_AsyncLoadClass::OnLoaded),
		FStreamableManager::DefaultAsyncLoadPriority);

	if (!StreamableHandle.IsValid())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Class failed to start the load request."));
		Failed.Broadcast(nullptr);
		SetReadyToDestroy();
	}
}

void UDirectiveUtilTask_AsyncLoadClass::OnLoaded()
{
	UClass* LoadedClass = StreamableHandle.IsValid() ? Cast<UClass>(StreamableHandle->GetLoadedAsset()) : nullptr;
	if (LoadedClass)
	{
		Completed.Broadcast(LoadedClass);
	}
	else
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Class completed but the class could not be resolved."));
		Failed.Broadcast(nullptr);
	}
	SetReadyToDestroy();
}

void UDirectiveUtilTask_AsyncLoadClass::Cancel()
{
	if (StreamableHandle.IsValid() && StreamableHandle->IsActive())
	{
		StreamableHandle->CancelHandle();
	}
	SetReadyToDestroy();
}

UDirectiveUtilTask_AsyncLoadAssets* UDirectiveUtilTask_AsyncLoadAssets::AsyncLoadAssets(UObject* WorldContextObject, const TArray<TSoftObjectPtr<UObject>>& Assets)
{
	UDirectiveUtilTask_AsyncLoadAssets* Action = NewObject<UDirectiveUtilTask_AsyncLoadAssets>();
	Action->SoftAssets = Assets;

	if (WorldContextObject)
	{
		Action->RegisterWithGameInstance(WorldContextObject);
	}

	return Action;
}

void UDirectiveUtilTask_AsyncLoadAssets::Activate()
{
	// Unset references are filtered out of the request but keep their null slots in the output;
	// duplicates are requested once and resolved per slot.
	TArray<FSoftObjectPath> PathsToLoad;
	for (const TSoftObjectPtr<UObject>& SoftAsset : SoftAssets)
	{
		if (!SoftAsset.IsNull())
		{
			PathsToLoad.AddUnique(SoftAsset.ToSoftObjectPath());
		}
	}

	if (PathsToLoad.Num() == 0)
	{
		// Nothing to request; an empty request list is an error path in the streamable manager.
		OnLoaded();
		return;
	}

	if (!UAssetManager::GetIfInitialized())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Assets failed to activate. The Asset Manager is not initialized."));
		OnLoaded();
		return;
	}

	StreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		MoveTemp(PathsToLoad),
		FStreamableDelegate::CreateUObject(this, &UDirectiveUtilTask_AsyncLoadAssets::OnLoaded),
		FStreamableManager::DefaultAsyncLoadPriority);

	if (!StreamableHandle.IsValid())
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Async Load Assets failed to start the load request."));
		OnLoaded();
		return;
	}

	// Binding fails when the load already finished (all assets were in memory); complete directly,
	// with the guard keeping the broadcast exactly once.
	if (!StreamableHandle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateUObject(this, &UDirectiveUtilTask_AsyncLoadAssets::OnUpdate)))
	{
		OnLoaded();
	}
}

void UDirectiveUtilTask_AsyncLoadAssets::OnLoaded()
{
	if (bHasCompleted)
	{
		return;
	}
	bHasCompleted = true;

	TArray<UObject*> LoadedAssets;
	LoadedAssets.Reserve(SoftAssets.Num());
	for (const TSoftObjectPtr<UObject>& SoftAsset : SoftAssets)
	{
		LoadedAssets.Add(SoftAsset.Get());
	}

	Completed.Broadcast(LoadedAssets);
	SetReadyToDestroy();
}

void UDirectiveUtilTask_AsyncLoadAssets::OnUpdate(TSharedRef<FStreamableHandle> Handle)
{
	if (bHasCompleted)
	{
		return;
	}

	int32 LoadedCount = 0;
	int32 RequestedCount = 0;
	Handle->GetLoadedCount(LoadedCount, RequestedCount);
	Progress.Broadcast(LoadedCount, RequestedCount);
}

void UDirectiveUtilTask_AsyncLoadAssets::Cancel()
{
	bHasCompleted = true;
	if (StreamableHandle.IsValid() && StreamableHandle->IsActive())
	{
		StreamableHandle->CancelHandle();
	}
	SetReadyToDestroy();
}
