// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/DirectiveUtilAsyncActionBase.h"
#include "Containers/Ticker.h"
#include "DirectiveUtilTask_FileSystem.generated.h"

struct FFileStatData;

/** The kind of change Watch File reports for its file. */
UENUM(BlueprintType)
enum class EDirectiveUtilFileChangeType : uint8
{
	/** The file appeared where none existed at the previous poll. */
	Created,
	/** The file's size, modification time, or recent contents changed since the previous poll. */
	Modified,
	/** The file existed at the previous poll and is now gone. */
	Deleted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilAsyncTextFileResult, const FString&, Contents, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilAsyncBinaryFileResult, const TArray<uint8>&, Bytes, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDirectiveUtilAsyncFileWriteResult, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilFileChanged, EDirectiveUtilFileChangeType, ChangeType, const FString&, Path);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDirectiveUtilFileWatchFailed, const FString&, Error);

/** Reads a text file on the worker pool and reports the result on the game thread. */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Read Text File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_ReadTextFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Reads a complete text file with the same rules as Read Text File.
	 *
	 * @param WorldContextObject Any object in the world that owns the read. Without one, `Failed` fires from Activate.
	 * @param Path The file to read, absolute or relative to the project Saved directory.
	 * @return The read action. Exactly one of `Completed` or `Failed` fires unless the action is canceled.
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Read Text File"))
	static UDirectiveUtilTask_ReadTextFile* ReadTextFileAsync(UObject* WorldContextObject, const FString& Path);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	/** Fires with the file contents and an empty error after a successful read. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncTextFileResult Completed;

	/** Fires with empty contents and an error when the world, path, or read is invalid. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncTextFileResult Failed;

private:
	void Finish(bool bSuccess, FString Contents, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	bool bFinished = false;
	bool bActivated = false;
};

/** Reads a binary file on the worker pool and reports the result on the game thread. */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Read Binary File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_ReadBinaryFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Reads every byte of a file.
	 *
	 * @param WorldContextObject Any object in the world that owns the read. Without one, `Failed` fires from Activate.
	 * @param Path The file to read, absolute or relative to the project Saved directory.
	 * @return The read action. Exactly one of `Completed` or `Failed` fires unless the action is canceled.
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Read Binary File"))
	static UDirectiveUtilTask_ReadBinaryFile* ReadBinaryFileAsync(UObject* WorldContextObject, const FString& Path);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	/** Fires with the file bytes and an empty error after a successful read. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncBinaryFileResult Completed;

	/** Fires with no bytes and an error when the world, path, or read is invalid. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncBinaryFileResult Failed;

private:
	void Finish(bool bSuccess, TArray<uint8> Bytes, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	bool bFinished = false;
	bool bActivated = false;
};

/** Writes a text file on the worker pool and reports the result on the game thread. */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Write Text File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WriteTextFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Writes text as UTF-8 without a byte-order mark on the worker pool. Canceling suppresses the callback, but a write that has
	 * already started still finishes on disk.
	 *
	 * @param WorldContextObject Any object in the world that owns the write. Without one, `Failed` fires from Activate.
	 * @param Path The file to write, absolute or relative to the project Saved directory.
	 * @param Contents The text to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the write fails if the file already exists.
	 * @param bAtomic When `true`, writes a temporary sibling and moves it over the destination.
	 * @return The write action. Exactly one of `Completed` or `Failed` fires unless the action is canceled.
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Write Text File"))
	static UDirectiveUtilTask_WriteTextFile* WriteTextFileAsync(
		UObject* WorldContextObject,
		const FString& Path,
		const FString& Contents,
		bool bCreateDirectories = true,
		bool bAllowOverwrite = true,
		bool bAtomic = true);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	/** Fires with an empty error after the file is written. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Completed;

	/** Fires with an error when the world or path is invalid, overwriting was refused, or the write failed. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Failed;

private:
	void Finish(bool bSuccess, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	FString Contents;
	bool bCreateDirectories = true;
	bool bAllowOverwrite = true;
	bool bAtomic = true;
	bool bFinished = false;
	bool bActivated = false;
};

/** Writes a binary file on the worker pool and reports the result on the game thread. */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Write Binary File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WriteBinaryFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Writes bytes on the worker pool. Canceling suppresses the callback, but a write that has
	 * already started still finishes on disk.
	 *
	 * @param WorldContextObject Any object in the world that owns the write. Without one, `Failed` fires from Activate.
	 * @param Path The file to write, absolute or relative to the project Saved directory.
	 * @param Bytes The bytes to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the write fails if the file already exists.
	 * @param bAtomic When `true`, writes a temporary sibling and moves it over the destination.
	 * @return The write action. Exactly one of `Completed` or `Failed` fires unless the action is canceled.
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Write Binary File"))
	static UDirectiveUtilTask_WriteBinaryFile* WriteBinaryFileAsync(
		UObject* WorldContextObject,
		const FString& Path,
		const TArray<uint8>& Bytes,
		bool bCreateDirectories = true,
		bool bAllowOverwrite = true,
		bool bAtomic = true);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	/** Fires with an empty error after the file is written. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Completed;

	/** Fires with an error when the world or path is invalid, overwriting was refused, or the write failed. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Failed;

private:
	void Finish(bool bSuccess, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	TArray<uint8> Bytes;
	bool bCreateDirectories = true;
	bool bAllowOverwrite = true;
	bool bAtomic = true;
	bool bFinished = false;
	bool bActivated = false;
};

/** Polls one file on the core ticker, reading it on the worker pool, and reports creation, modification, and deletion on the game thread. */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Watch File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WatchFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Polls one file for creation, modification, and deletion until canceled or the world is cleaned up.
	 *
	 * @param WorldContextObject Any object in the world that owns the watcher.
	 * @param Path The file to watch, absolute or relative to the project Saved directory.
	 * @param PollInterval Real seconds between polls. Game pause and time dilation do not affect it. Values shorter than a frame poll once per frame. Non-positive and non-finite values fire `Failed`.
	 * @return The watcher. `Changed` reports each detected change with the requested path.
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Watch File"))
	static UDirectiveUtilTask_WatchFile* WatchFile(UObject* WorldContextObject, const FString& Path, float PollInterval = 0.25f);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;
	virtual void SetReadyToDestroy() override;

	/** Fires once per detected change with the change type and the path as it was passed in. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilFileChanged Changed;

	/** Fires from Activate when the world, path, or poll interval is invalid. No polls run afterward. */
	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilFileWatchFailed Failed;

private:
	bool Poll(float DeltaTime);
	void ApplyPoll(const FFileStatData& State, bool bExists, bool bHasContentHash, uint32 ContentHash);
	void RecordState(const FFileStatData& State, bool bExists, bool bHasContentHash, uint32 ContentHash);
	void Fail(FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	FString ResolvedPath;
	FDateTime PreviousModificationTime;
	int64 PreviousSize = -1;
	uint32 PreviousContentHash = 0;
	float PollInterval = 0.25f;
	bool bPreviouslyExists = false;
	bool bHasPreviousContentHash = false;
	bool bPollInFlight = false;
	bool bFinished = false;
	bool bActivated = false;
	FTSTicker::FDelegateHandle TickerHandle;
};
