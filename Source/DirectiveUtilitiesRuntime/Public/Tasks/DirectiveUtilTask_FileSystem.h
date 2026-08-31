// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/DirectiveUtilAsyncActionBase.h"
#include "TimerManager.h"
#include "DirectiveUtilTask_FileSystem.generated.h"

UENUM(BlueprintType)
enum class EDirectiveUtilFileChangeType : uint8
{
	Created,
	Modified,
	Deleted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilAsyncTextFileResult, const FString&, Contents, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilAsyncBinaryFileResult, const TArray<uint8>&, Bytes, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDirectiveUtilAsyncFileWriteResult, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDirectiveUtilFileChanged, EDirectiveUtilFileChangeType, ChangeType, const FString&, Path);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDirectiveUtilFileWatchFailed, const FString&, Error);

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Read Text File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_ReadTextFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Read Text File"))
	static UDirectiveUtilTask_ReadTextFile* ReadTextFileAsync(UObject* WorldContextObject, const FString& Path);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncTextFileResult Completed;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncTextFileResult Failed;

private:
	void Finish(bool bSuccess, FString Contents, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	bool bFinished = false;
};

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Read Binary File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_ReadBinaryFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Async Read Binary File"))
	static UDirectiveUtilTask_ReadBinaryFile* ReadBinaryFileAsync(UObject* WorldContextObject, const FString& Path);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncBinaryFileResult Completed;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncBinaryFileResult Failed;

private:
	void Finish(bool bSuccess, TArray<uint8> Bytes, FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	bool bFinished = false;
};

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Write Text File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WriteTextFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
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

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Completed;

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
};

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Async Write Binary File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WriteBinaryFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
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

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilAsyncFileWriteResult Completed;

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
};

UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask, DisplayName="Watch File"))
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilTask_WatchFile : public UDirectiveUtilCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Category="Directive Utilities|FileSystem", WorldContext="WorldContextObject", DisplayName="Watch File"))
	static UDirectiveUtilTask_WatchFile* WatchFile(UObject* WorldContextObject, const FString& Path, float PollInterval = 0.25f);

	virtual void Activate() override;
	virtual void Cancel() override;
	virtual bool IsActive() const override;
	virtual bool ShouldBroadcastDelegates() const override;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilFileChanged Changed;

	UPROPERTY(BlueprintAssignable)
	FDirectiveUtilFileWatchFailed Failed;

private:
	void Poll();
	void Fail(FString Error);

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	FString Path;
	FString ResolvedPath;
	FDateTime PreviousModificationTime;
	int64 PreviousSize = -1;
	float PollInterval = 0.25f;
	bool bPreviouslyExists = false;
	bool bFinished = false;
	FTimerHandle TimerHandle;
};
