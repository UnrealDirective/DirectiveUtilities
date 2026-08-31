// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_FileSystem.h"

#include "Async/Async.h"
#include "DirectiveUtilRuntimeHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"

namespace
{
	bool IsAsyncFilePathUsable(const FString& Path)
	{
		return !Path.IsEmpty() && Path.Len() == FCString::Strlen(*Path);
	}

	bool HasUsableWorld(const UObject* WorldContextObject)
	{
		return WorldContextObject && GEngine
			&& GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	template <typename ActionType>
	ActionType* CreateAction(UObject* WorldContextObject)
	{
		ActionType* Action = NewObject<ActionType>();
		if (WorldContextObject)
		{
			Action->RegisterWithGameInstance(WorldContextObject);
		}
		return Action;
	}
}

UDirectiveUtilTask_ReadTextFile* UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(UObject* WorldContextObject, const FString& Path)
{
	UDirectiveUtilTask_ReadTextFile* Action = CreateAction<UDirectiveUtilTask_ReadTextFile>(WorldContextObject);
	Action->WorldContextObject = WorldContextObject;
	Action->Path = Path;
	return Action;
}

void UDirectiveUtilTask_ReadTextFile::Activate()
{
	if (!HasUsableWorld(WorldContextObject) || !IsAsyncFilePathUsable(Path))
	{
		Finish(false, FString(), TEXT("A valid world and file path are required."));
		return;
	}

	const FString RequestedPath = Path;
	TWeakObjectPtr<UDirectiveUtilTask_ReadTextFile> WeakAction(this);
	Async(EAsyncExecution::ThreadPool, [WeakAction, RequestedPath]()
	{
		FString Contents;
		const bool bSuccess = UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(RequestedPath, Contents);
		FString Error = bSuccess ? FString() : FString::Printf(TEXT("Could not read text file: %s"), *RequestedPath);
		AsyncTask(ENamedThreads::GameThread, [WeakAction, bSuccess, Contents = MoveTemp(Contents), Error = MoveTemp(Error)]() mutable
		{
			if (UDirectiveUtilTask_ReadTextFile* Action = WeakAction.Get())
			{
				Action->Finish(bSuccess, MoveTemp(Contents), MoveTemp(Error));
			}
		});
	});
}

void UDirectiveUtilTask_ReadTextFile::Cancel()
{
	bFinished = true;
	Completed.Clear();
	Failed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_ReadTextFile::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_ReadTextFile::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_ReadTextFile::Finish(const bool bSuccess, FString Contents, FString Error)
{
	if (ShouldBroadcastDelegates())
	{
		(bSuccess ? Completed : Failed).Broadcast(Contents, Error);
	}
	bFinished = true;
	SetReadyToDestroy();
}

UDirectiveUtilTask_ReadBinaryFile* UDirectiveUtilTask_ReadBinaryFile::ReadBinaryFileAsync(UObject* WorldContextObject, const FString& Path)
{
	UDirectiveUtilTask_ReadBinaryFile* Action = CreateAction<UDirectiveUtilTask_ReadBinaryFile>(WorldContextObject);
	Action->WorldContextObject = WorldContextObject;
	Action->Path = Path;
	return Action;
}

void UDirectiveUtilTask_ReadBinaryFile::Activate()
{
	if (!HasUsableWorld(WorldContextObject) || !IsAsyncFilePathUsable(Path))
	{
		Finish(false, TArray<uint8>(), TEXT("A valid world and file path are required."));
		return;
	}

	const FString RequestedPath = Path;
	TWeakObjectPtr<UDirectiveUtilTask_ReadBinaryFile> WeakAction(this);
	Async(EAsyncExecution::ThreadPool, [WeakAction, RequestedPath]()
	{
		TArray<uint8> Bytes;
		const bool bSuccess = UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(RequestedPath, Bytes);
		FString Error = bSuccess ? FString() : FString::Printf(TEXT("Could not read binary file: %s"), *RequestedPath);
		AsyncTask(ENamedThreads::GameThread, [WeakAction, bSuccess, Bytes = MoveTemp(Bytes), Error = MoveTemp(Error)]() mutable
		{
			if (UDirectiveUtilTask_ReadBinaryFile* Action = WeakAction.Get())
			{
				Action->Finish(bSuccess, MoveTemp(Bytes), MoveTemp(Error));
			}
		});
	});
}

void UDirectiveUtilTask_ReadBinaryFile::Cancel()
{
	bFinished = true;
	Completed.Clear();
	Failed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_ReadBinaryFile::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_ReadBinaryFile::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_ReadBinaryFile::Finish(const bool bSuccess, TArray<uint8> Bytes, FString Error)
{
	if (ShouldBroadcastDelegates())
	{
		(bSuccess ? Completed : Failed).Broadcast(Bytes, Error);
	}
	bFinished = true;
	SetReadyToDestroy();
}

UDirectiveUtilTask_WriteTextFile* UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
	UObject* WorldContextObject,
	const FString& Path,
	const FString& Contents,
	const bool bCreateDirectories,
	const bool bAllowOverwrite,
	const bool bAtomic)
{
	UDirectiveUtilTask_WriteTextFile* Action = CreateAction<UDirectiveUtilTask_WriteTextFile>(WorldContextObject);
	Action->WorldContextObject = WorldContextObject;
	Action->Path = Path;
	Action->Contents = Contents;
	Action->bCreateDirectories = bCreateDirectories;
	Action->bAllowOverwrite = bAllowOverwrite;
	Action->bAtomic = bAtomic;
	return Action;
}

void UDirectiveUtilTask_WriteTextFile::Activate()
{
	if (!HasUsableWorld(WorldContextObject) || !IsAsyncFilePathUsable(Path))
	{
		Finish(false, TEXT("A valid world and file path are required."));
		return;
	}

	const FString RequestedPath = Path;
	const FString RequestedContents = Contents;
	const bool bRequestedCreateDirectories = bCreateDirectories;
	const bool bRequestedAllowOverwrite = bAllowOverwrite;
	const bool bRequestedAtomic = bAtomic;
	TWeakObjectPtr<UDirectiveUtilTask_WriteTextFile> WeakAction(this);
	Async(EAsyncExecution::ThreadPool, [WeakAction, RequestedPath, RequestedContents, bRequestedCreateDirectories, bRequestedAllowOverwrite, bRequestedAtomic]()
	{
		const bool bSuccess = bRequestedAtomic
			? UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(RequestedPath, RequestedContents, bRequestedCreateDirectories, bRequestedAllowOverwrite)
			: UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(RequestedPath, RequestedContents, bRequestedCreateDirectories, bRequestedAllowOverwrite);
		FString Error = bSuccess ? FString() : FString::Printf(TEXT("Could not write text file: %s"), *RequestedPath);
		AsyncTask(ENamedThreads::GameThread, [WeakAction, bSuccess, Error = MoveTemp(Error)]() mutable
		{
			if (UDirectiveUtilTask_WriteTextFile* Action = WeakAction.Get())
			{
				Action->Finish(bSuccess, MoveTemp(Error));
			}
		});
	});
}

void UDirectiveUtilTask_WriteTextFile::Cancel()
{
	bFinished = true;
	Completed.Clear();
	Failed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_WriteTextFile::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_WriteTextFile::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_WriteTextFile::Finish(const bool bSuccess, FString Error)
{
	if (ShouldBroadcastDelegates())
	{
		(bSuccess ? Completed : Failed).Broadcast(Error);
	}
	bFinished = true;
	SetReadyToDestroy();
}

UDirectiveUtilTask_WriteBinaryFile* UDirectiveUtilTask_WriteBinaryFile::WriteBinaryFileAsync(
	UObject* WorldContextObject,
	const FString& Path,
	const TArray<uint8>& Bytes,
	const bool bCreateDirectories,
	const bool bAllowOverwrite,
	const bool bAtomic)
{
	UDirectiveUtilTask_WriteBinaryFile* Action = CreateAction<UDirectiveUtilTask_WriteBinaryFile>(WorldContextObject);
	Action->WorldContextObject = WorldContextObject;
	Action->Path = Path;
	Action->Bytes = Bytes;
	Action->bCreateDirectories = bCreateDirectories;
	Action->bAllowOverwrite = bAllowOverwrite;
	Action->bAtomic = bAtomic;
	return Action;
}

void UDirectiveUtilTask_WriteBinaryFile::Activate()
{
	if (!HasUsableWorld(WorldContextObject) || !IsAsyncFilePathUsable(Path))
	{
		Finish(false, TEXT("A valid world and file path are required."));
		return;
	}

	const FString RequestedPath = Path;
	TArray<uint8> RequestedBytes = MoveTemp(Bytes);
	const bool bRequestedCreateDirectories = bCreateDirectories;
	const bool bRequestedAllowOverwrite = bAllowOverwrite;
	const bool bRequestedAtomic = bAtomic;
	TWeakObjectPtr<UDirectiveUtilTask_WriteBinaryFile> WeakAction(this);
	Async(EAsyncExecution::ThreadPool, [WeakAction, RequestedPath, RequestedBytes = MoveTemp(RequestedBytes), bRequestedCreateDirectories, bRequestedAllowOverwrite, bRequestedAtomic]()
	{
		const bool bSuccess = bRequestedAtomic
			? UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(RequestedPath, RequestedBytes, bRequestedCreateDirectories, bRequestedAllowOverwrite)
			: UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(RequestedPath, RequestedBytes, bRequestedCreateDirectories, bRequestedAllowOverwrite);
		FString Error = bSuccess ? FString() : FString::Printf(TEXT("Could not write binary file: %s"), *RequestedPath);
		AsyncTask(ENamedThreads::GameThread, [WeakAction, bSuccess, Error = MoveTemp(Error)]() mutable
		{
			if (UDirectiveUtilTask_WriteBinaryFile* Action = WeakAction.Get())
			{
				Action->Finish(bSuccess, MoveTemp(Error));
			}
		});
	});
}

void UDirectiveUtilTask_WriteBinaryFile::Cancel()
{
	bFinished = true;
	Completed.Clear();
	Failed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_WriteBinaryFile::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_WriteBinaryFile::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_WriteBinaryFile::Finish(const bool bSuccess, FString Error)
{
	if (ShouldBroadcastDelegates())
	{
		(bSuccess ? Completed : Failed).Broadcast(Error);
	}
	bFinished = true;
	SetReadyToDestroy();
}

UDirectiveUtilTask_WatchFile* UDirectiveUtilTask_WatchFile::WatchFile(UObject* WorldContextObject, const FString& Path, const float PollInterval)
{
	UDirectiveUtilTask_WatchFile* Action = CreateAction<UDirectiveUtilTask_WatchFile>(WorldContextObject);
	Action->WorldContextObject = WorldContextObject;
	Action->Path = Path;
	Action->PollInterval = PollInterval;
	return Action;
}

void UDirectiveUtilTask_WatchFile::Activate()
{
	UWorld* World = WorldContextObject && GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	FTimerManager* TimerManager = GetTimerManager();
	if (!World || !TimerManager || !IsAsyncFilePathUsable(Path) || !FMath::IsFinite(PollInterval) || PollInterval <= 0.0f)
	{
		Fail(TEXT("A valid world, file path, and positive finite poll interval are required."));
		return;
	}

	ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	bPreviouslyExists = IFileManager::Get().FileExists(*ResolvedPath);
	const FFileStatData InitialState = IFileManager::Get().GetStatData(*ResolvedPath);
	PreviousSize = bPreviouslyExists ? InitialState.FileSize : -1;
	PreviousModificationTime = bPreviouslyExists ? InitialState.ModificationTime : FDateTime();
	TimerManager->SetTimer(TimerHandle, this, &UDirectiveUtilTask_WatchFile::Poll, PollInterval, true);
}

void UDirectiveUtilTask_WatchFile::Cancel()
{
	bFinished = true;
	if (FTimerManager* TimerManager = GetTimerManager())
	{
		TimerManager->ClearTimer(TimerHandle);
	}
	Changed.Clear();
	Failed.Clear();
	Super::Cancel();
}

bool UDirectiveUtilTask_WatchFile::IsActive() const
{
	return !bFinished && Super::IsActive();
}

bool UDirectiveUtilTask_WatchFile::ShouldBroadcastDelegates() const
{
	return !bFinished && Super::ShouldBroadcastDelegates();
}

void UDirectiveUtilTask_WatchFile::Poll()
{
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	const bool bExists = FileManager.FileExists(*ResolvedPath);
	const FFileStatData CurrentState = bExists ? FileManager.GetStatData(*ResolvedPath) : FFileStatData();

	if (!bPreviouslyExists && bExists)
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Created, Path);
	}
	else if (bPreviouslyExists && !bExists)
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Deleted, Path);
	}
	else if (bExists && (CurrentState.FileSize != PreviousSize || CurrentState.ModificationTime != PreviousModificationTime))
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Modified, Path);
	}

	bPreviouslyExists = bExists;
	PreviousSize = bExists ? CurrentState.FileSize : -1;
	PreviousModificationTime = bExists ? CurrentState.ModificationTime : FDateTime();
}

void UDirectiveUtilTask_WatchFile::Fail(FString Error)
{
	if (ShouldBroadcastDelegates())
	{
		Failed.Broadcast(Error);
	}
	bFinished = true;
	SetReadyToDestroy();
}
