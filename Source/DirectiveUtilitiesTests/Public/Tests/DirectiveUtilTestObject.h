#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameFramework/SaveGame.h"
#include "Engine/HitResult.h"
#include "DirectiveUtilTestObject.generated.h"

class UWorld;

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
	TMap<int32, int32> TestMap;

	UPROPERTY()
	TMap<int32, int32> TestMap2;

	UPROPERTY()
	TMap<FString, int32> TestStringKeyMap;
};

/** Concrete save-game subclass for tests (USaveGame itself is abstract and cannot be instantiated). */
UCLASS()
class UDirectiveUtilTestSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 TestValue = 0;
};

/**
 * Listener for exercising the async-task dynamic multicast delegates from automation tests.
 * Dynamic delegates can only bind to UFUNCTIONs, so the handlers live on a UObject. The
 * Keepalive property lets a rooted listener keep the async task itself alive across frames.
 */
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

	/** Number of hits reported by the most recent trace completion. */
	UPROPERTY()
	int32 HitCount = 0;

	/** Success flag from the most recent bool-payload completion (e.g. move-to-location). */
	UPROPERTY()
	bool bLastSuccess = false;

	UPROPERTY()
	TObjectPtr<UObject> LastObject = nullptr;

	/** The object array from the most recent batch completion (e.g. async load assets). */
	UPROPERTY()
	TArray<TObjectPtr<UObject>> LastObjects;

	/** Holds a strong reference to the async task so it survives GC while the test waits. */
	UPROPERTY()
	TObjectPtr<UObject> Keepalive = nullptr;

	/** The transient world a latent delay scenario ticks and tears down when it settles. */
	UPROPERTY()
	TObjectPtr<UWorld> ScenarioWorld = nullptr;

	/** Handler for parameterless completion delegates (e.g. the cancellable delay). */
	UFUNCTION()
	void OnCompleted() { bCompleted = true; ++CompletedCount; }

	/** Handler for object-payload completion delegates (e.g. async load asset). */
	UFUNCTION()
	void OnObjectCompleted(UObject* Object) { bCompleted = true; ++CompletedCount; LastObject = Object; }

	/** Handler for object-payload failure delegates. */
	UFUNCTION()
	void OnObjectFailed(UObject* Object) { bFailed = true; }

	/** Handler for object-array-payload completion delegates (e.g. async load assets). */
	UFUNCTION()
	void OnObjectsCompleted(const TArray<UObject*>& Objects) { bCompleted = true; ++CompletedCount; LastObjects.Reset(); LastObjects.Append(Objects); }

	/** Handler for class-payload completion delegates (e.g. async load class). */
	UFUNCTION()
	void OnClassCompleted(UClass* Class) { bCompleted = true; ++CompletedCount; LastObject = Class; }

	/** Handler for class-payload failure delegates. */
	UFUNCTION()
	void OnClassFailed(UClass* Class) { bFailed = true; }

	/** Handler for trace completion delegates. */
	UFUNCTION()
	void OnTraceCompleted(const TArray<FHitResult>& Hits) { bCompleted = true; ++CompletedCount; HitCount = Hits.Num(); }

	/** Handler for bool-payload completion delegates (e.g. move-to-location). */
	UFUNCTION()
	void OnBoolCompleted(bool bSuccess) { bCompleted = true; ++CompletedCount; bLastSuccess = bSuccess; }
};
