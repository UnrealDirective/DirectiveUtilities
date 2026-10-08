// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "GameFramework/SaveGame.h"
#include "Engine/HitResult.h"
#include "Tasks/DirectiveUtilTask_MoveToLocation.h"
#include "Tasks/DirectiveUtilTask_FileSystem.h"
#include "DirectiveUtilTestObject.generated.h"

class UWorld;
class UGameInstance;

UINTERFACE()
class UDirectiveUtilTestInterface : public UInterface
{
	GENERATED_BODY()
};

class IDirectiveUtilTestInterface
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FDirectiveUtilCollisionValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	int32 Value = 0;

	bool operator==(const FDirectiveUtilCollisionValue& Other) const
	{
		return Value == Other.Value;
	}

	friend uint32 GetTypeHash(const FDirectiveUtilCollisionValue&)
	{
		return 0;
	}
};

template <>
struct TStructOpsTypeTraits<FDirectiveUtilCollisionValue> : TStructOpsTypeTraitsBase2<FDirectiveUtilCollisionValue>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
};

USTRUCT()
struct FDirectiveUtilPodValue
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Index;

	UPROPERTY()
	float Weight;
};

static_assert(TIsPODType<FDirectiveUtilPodValue>::Value);

UCLASS()
class UDirectiveUtilTestObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<int32> TestArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<FString> TestStringArray;

	UPROPERTY()
	TArray<FName> TestNameArray;

	UPROPERTY()
	TArray<FText> TestTextArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<bool> TestBoolArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<FDirectiveUtilCollisionValue> TestCollisionArray;

	UPROPERTY()
	TArray<FDirectiveUtilPodValue> TestPodArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<TObjectPtr<UObject>> TestObjectArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<bool> TestBoolSourceArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<FString> TestStringSourceArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<FDirectiveUtilCollisionValue> TestCollisionSourceArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<TObjectPtr<UObject>> TestObjectSourceArray;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TArray<int32> TestIndices;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	bool TestBoolItem = false;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	FString TestStringItem;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	FDirectiveUtilCollisionValue TestCollisionItem;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	TObjectPtr<UObject> TestObjectItem;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	bool TestInsertResult = false;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	int32 TestRemovedCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Directive Utilities|Tests")
	bool TestRemoveAllResult = false;

	UFUNCTION(BlueprintImplementableEvent)
	void RunArrayThunkScenario();

	UPROPERTY()
	TMap<int32, int32> TestMap;

	UPROPERTY()
	TMap<int32, int32> TestMap2;

	UPROPERTY()
	TMap<FString, int32> TestStringKeyMap;

	UPROPERTY()
	TMap<FString, FString> TestStringMap;

	UPROPERTY()
	TMap<FString, FString> TestStringMap2;

	UPROPERTY()
	TMap<FName, FDirectiveUtilCollisionValue> TestStructValueMap;
};

UCLASS()
class UDirectiveUtilTestSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 TestValue = 0;
};

UCLASS()
class UDirectiveUtilDelegateListener : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bCompleted = false;

	UPROPERTY()
	bool bFailed = false;

	UPROPERTY()
	int32 CompletedCount = 0;

	UPROPERTY()
	int32 HitCount = 0;

	UPROPERTY()
	int32 UpdatedCount = 0;

	UPROPERTY()
	int32 IterationCount = 0;

	UPROPERTY()
	float LastElapsedTime = 0.0f;

	UPROPERTY()
	float LastDeltaTime = 0.0f;

	UPROPERTY()
	float LastAlpha = 0.0f;

	UPROPERTY()
	TArray<int32> IterationIndices;

	UPROPERTY()
	TArray<int32> IterationRemaining;

	UPROPERTY()
	TArray<float> UpdateElapsedTimes;

	UPROPERTY()
	TArray<float> UpdateDeltaTimes;

	UPROPERTY()
	TArray<float> UpdateAlphas;

	UPROPERTY()
	bool bLastSuccess = false;

	UPROPERTY()
	TObjectPtr<UObject> LastObject = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LastObjects;

	UPROPERTY()
	FString LastString;

	UPROPERTY()
	FString LastError;

	UPROPERTY()
	TArray<uint8> LastBytes;

	UPROPERTY()
	TArray<EDirectiveUtilFileChangeType> FileChangeTypes;

	UPROPERTY()
	TArray<FString> FileChangePaths;

	UPROPERTY()
	TObjectPtr<UObject> Keepalive = nullptr;

	UPROPERTY()
	TObjectPtr<UWorld> ScenarioWorld = nullptr;

	UPROPERTY()
	TObjectPtr<UGameInstance> ScenarioGameInstance = nullptr;

	UFUNCTION()
	void OnCompleted() { bCompleted = true; ++CompletedCount; }

	UFUNCTION()
	void OnObjectCompleted(UObject* Object) { bCompleted = true; ++CompletedCount; LastObject = Object; }

	UFUNCTION()
	void OnObjectFailed(UObject* Object) { bFailed = true; }

	UFUNCTION()
	void OnObjectsCompleted(const TArray<UObject*>& Objects) { bCompleted = true; ++CompletedCount; LastObjects.Reset(); LastObjects.Append(Objects); }

	UFUNCTION()
	void OnClassCompleted(UClass* Class) { bCompleted = true; ++CompletedCount; LastObject = Class; }

	UFUNCTION()
	void OnClassFailed(UClass* Class) { bFailed = true; }

	UFUNCTION()
	void OnTraceCompleted(const TArray<FHitResult>& Hits) { bCompleted = true; ++CompletedCount; HitCount = Hits.Num(); }

	UFUNCTION()
	void OnBoolCompleted(bool bSuccess) { bCompleted = true; ++CompletedCount; bLastSuccess = bSuccess; }

	UFUNCTION()
	void OnTextFileCompleted(const FString& Contents, const FString& Error) { bCompleted = true; ++CompletedCount; LastString = Contents; LastError = Error; }

	UFUNCTION()
	void OnTextFileFailed(const FString& Contents, const FString& Error) { bFailed = true; LastString = Contents; LastError = Error; }

	UFUNCTION()
	void OnBinaryFileCompleted(const TArray<uint8>& Bytes, const FString& Error) { bCompleted = true; ++CompletedCount; LastBytes = Bytes; LastError = Error; }

	UFUNCTION()
	void OnBinaryFileFailed(const TArray<uint8>& Bytes, const FString& Error) { bFailed = true; LastBytes = Bytes; LastError = Error; }

	UFUNCTION()
	void OnFileWriteCompleted(const FString& Error) { bCompleted = true; ++CompletedCount; LastError = Error; }

	UFUNCTION()
	void OnFileWriteFailed(const FString& Error) { bFailed = true; LastError = Error; }

	UFUNCTION()
	void OnFileChanged(EDirectiveUtilFileChangeType ChangeType, const FString& Path) { FileChangeTypes.Add(ChangeType); FileChangePaths.Add(Path); }

	UFUNCTION()
	void OnFileWatchFailed(const FString& Error) { bFailed = true; LastError = Error; }

	UFUNCTION()
	void OnDurationUpdated(float ElapsedTime, float DeltaTime, float Alpha) { ++UpdatedCount; LastElapsedTime = ElapsedTime; LastDeltaTime = DeltaTime; LastAlpha = Alpha; UpdateElapsedTimes.Add(ElapsedTime); UpdateDeltaTimes.Add(DeltaTime); UpdateAlphas.Add(Alpha); }

	UFUNCTION()
	void OnRepeatIteration(int32 Index, int32 Remaining) { ++IterationCount; IterationIndices.Add(Index); IterationRemaining.Add(Remaining); }

};

UCLASS()
class UDirectiveUtilTestMoveToLocationTask : public UDirectiveUtilTask_MoveToLocation
{
	GENERATED_BODY()

public:
	void Configure(AController* InController, const FVector InDestination, const bool bInCheckStuckMovement)
	{
		Controller = InController;
		Destination = InDestination;
		bCheckStuckMovement = bInCheckStuckMovement;
	}

	void ClearController()
	{
		Controller = nullptr;
	}

	void Complete()
	{
		ExecuteCompleted(false);
	}

	void RegisterTimersForTest(UWorld* World);
	bool HasRegisteredTimers() const;
};

UCLASS()
class UDirectiveUtilTestMoveToActorTask : public UDirectiveUtilTask_MoveToActor
{
	GENERATED_BODY()

public:
	void Configure(AController* InController, AActor* InGoal, const bool bInCheckStuckMovement)
	{
		Controller = InController;
		Goal = InGoal;
		bCheckStuckMovement = bInCheckStuckMovement;
	}

	void ClearController()
	{
		Controller = nullptr;
	}

	void Complete()
	{
		ExecuteCompleted(false);
	}

	void RegisterTimersForTest(UWorld* World);
	bool HasRegisteredTimers() const;
};
