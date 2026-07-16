#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameFramework/SaveGame.h"
#include "Engine/HitResult.h"
#include "DirectiveUtilTestObject.generated.h"

class UWorld;

USTRUCT()
struct FDirectiveUtilCollisionValue
{
	GENERATED_BODY()

	UPROPERTY()
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

UCLASS()
class UDirectiveUtilTestObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<int32> TestArray;

	UPROPERTY()
	TArray<FString> TestStringArray;

	UPROPERTY()
	TArray<FName> TestNameArray;

	UPROPERTY()
	TArray<FText> TestTextArray;

	UPROPERTY()
	TArray<FDirectiveUtilCollisionValue> TestCollisionArray;

	UPROPERTY()
	TArray<TObjectPtr<UObject>> TestObjectArray;

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
	bool bLastSuccess = false;

	UPROPERTY()
	TObjectPtr<UObject> LastObject = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LastObjects;

	UPROPERTY()
	TObjectPtr<UObject> Keepalive = nullptr;

	UPROPERTY()
	TObjectPtr<UWorld> ScenarioWorld = nullptr;

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
};
