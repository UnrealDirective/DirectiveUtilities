// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_FileSystem.h"

#include "Async/Async.h"
#include "DirectiveUtilRuntimeHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"

namespace
{
	constexpr int64 MaximumHashedWatchFileSize = 1024 * 1024;
	constexpr double RecentModificationSeconds = 2.0;

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

	bool IsRecentlyModified(const FDateTime& ModificationTime)
	{
		return FMath::Abs((FDateTime::UtcNow() - ModificationTime).GetTotalSeconds()) <= RecentModificationSeconds;
	}

	bool TryHashFile(const FString& ResolvedPath, const int64 FileSize, uint32& OutHash)
	{
		TArray<uint8> Bytes;
		if (FileSize > MaximumHashedWatchFileSize || !FFileHelper::LoadFileToArray(Bytes, *ResolvedPath, FILEREAD_Silent))
		{
			return false;
		}
		OutHash = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
		return true;
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
	if (bActivated || bFinished)
	{
		return;
	}
	bActivated = true;
	RootWithoutGameInstance();
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
	if (bActivated || bFinished)
	{
		return;
	}
	bActivated = true;
	RootWithoutGameInstance();
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
	if (bActivated || bFinished)
	{
		return;
	}
	bActivated = true;
	RootWithoutGameInstance();
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
	if (bActivated || bFinished)
	{
		return;
	}
	bActivated = true;
	RootWithoutGameInstance();
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
	if (bActivated || bFinished)
	{
		return;
	}
	bActivated = true;
	RootWithoutGameInstance();
	if (!HasUsableWorld(WorldContextObject) || !IsAsyncFilePathUsable(Path) || !FMath::IsFinite(PollInterval) || PollInterval <= 0.0f)
	{
		Fail(TEXT("A valid world, file path, and positive finite poll interval are required."));
		return;
	}

	ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	const FFileStatData InitialState = IFileManager::Get().GetStatData(*ResolvedPath);
	const bool bExists = InitialState.bIsValid && !InitialState.bIsDirectory;
	uint32 ContentHash = 0;
	const bool bHasContentHash = bExists && IsRecentlyModified(InitialState.ModificationTime)
		&& TryHashFile(ResolvedPath, InitialState.FileSize, ContentHash);
	RecordState(InitialState, bExists, bHasContentHash, ContentHash);
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UDirectiveUtilTask_WatchFile::Poll), PollInterval);
}

void UDirectiveUtilTask_WatchFile::Cancel()
{
	bFinished = true;
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

void UDirectiveUtilTask_WatchFile::SetReadyToDestroy()
{
	FTSTicker::RemoveTicker(TickerHandle);
	TickerHandle.Reset();
	Super::SetReadyToDestroy();
}

bool UDirectiveUtilTask_WatchFile::Poll(float)
{
	if (!ShouldBroadcastDelegates())
	{
		return false;
	}
	if (bPollInFlight)
	{
		return true;
	}

	bPollInFlight = true;
	const FString PolledPath = ResolvedPath;
	const bool bKnownToExist = bPreviouslyExists;
	const int64 KnownSize = PreviousSize;
	const FDateTime KnownModificationTime = PreviousModificationTime;
	const bool bHasKnownContentHash = bHasPreviousContentHash;
	TWeakObjectPtr<UDirectiveUtilTask_WatchFile> WeakAction(this);
	Async(EAsyncExecution::ThreadPool,
		[WeakAction, PolledPath, bKnownToExist, KnownSize, KnownModificationTime, bHasKnownContentHash]()
	{
		const FFileStatData State = IFileManager::Get().GetStatData(*PolledPath);
		const bool bExists = State.bIsValid && !State.bIsDirectory;
		const bool bMetadataUnchanged = bExists && bKnownToExist
			&& State.FileSize == KnownSize && State.ModificationTime == KnownModificationTime;

		// Mac and Linux report whole-second modification times, so a same-size rewrite inside that second
		// only shows up in the contents.
		const bool bShouldHash = bExists
			&& (IsRecentlyModified(State.ModificationTime) || (bMetadataUnchanged && bHasKnownContentHash));
		uint32 ContentHash = 0;
		const bool bHasContentHash = bShouldHash && TryHashFile(PolledPath, State.FileSize, ContentHash);
		AsyncTask(ENamedThreads::GameThread, [WeakAction, State, bExists, bHasContentHash, ContentHash]()
		{
			if (UDirectiveUtilTask_WatchFile* Action = WeakAction.Get())
			{
				Action->ApplyPoll(State, bExists, bHasContentHash, ContentHash);
			}
		});
	});
	return true;
}

void UDirectiveUtilTask_WatchFile::ApplyPoll(
	const FFileStatData& State,
	const bool bExists,
	const bool bHasContentHash,
	const uint32 ContentHash)
{
	bPollInFlight = false;
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	const bool bMetadataUnchanged = bExists && bPreviouslyExists
		&& State.FileSize == PreviousSize && State.ModificationTime == PreviousModificationTime;
	const bool bContentChanged = bMetadataUnchanged && bHasContentHash && bHasPreviousContentHash
		&& ContentHash != PreviousContentHash;
	const bool bCreated = !bPreviouslyExists && bExists;
	const bool bDeleted = bPreviouslyExists && !bExists;
	const bool bModified = bPreviouslyExists && bExists && (!bMetadataUnchanged || bContentChanged);
	RecordState(State, bExists, bHasContentHash, ContentHash);

	if (bCreated)
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Created, Path);
	}
	else if (bDeleted)
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Deleted, Path);
	}
	else if (bModified)
	{
		Changed.Broadcast(EDirectiveUtilFileChangeType::Modified, Path);
	}
}

void UDirectiveUtilTask_WatchFile::RecordState(
	const FFileStatData& State,
	const bool bExists,
	const bool bHasContentHash,
	const uint32 ContentHash)
{
	bPreviouslyExists = bExists;
	PreviousSize = bExists ? State.FileSize : -1;
	PreviousModificationTime = bExists ? State.ModificationTime : FDateTime();
	bHasPreviousContentHash = bHasContentHash && IsRecentlyModified(State.ModificationTime);
	PreviousContentHash = bHasPreviousContentHash ? ContentHash : 0;
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
