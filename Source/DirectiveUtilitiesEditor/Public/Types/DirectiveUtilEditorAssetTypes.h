// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilEditorAssetTypes.generated.h"

USTRUCT(BlueprintType)
struct FDirectiveUtilAssetKey
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset")
	FString AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset")
	FString AssetClass;

	FDirectiveUtilAssetKey(const FString& InAssetName, const FString& InAssetClass)
		: AssetName(InAssetName), AssetClass(InAssetClass) {}

	bool operator==(const FDirectiveUtilAssetKey& Other) const
	{
		return AssetName == Other.AssetName && AssetClass == Other.AssetClass;
	}

	friend uint32 GetTypeHash(const FDirectiveUtilAssetKey& Key)
	{
		return HashCombine(GetTypeHash(Key.AssetName), GetTypeHash(Key.AssetClass));
	}

	FDirectiveUtilAssetKey(): AssetName(FString()), AssetClass(FString()) {}
};

USTRUCT(BlueprintType)
struct FDirectiveUtilDuplicateAssetData
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset")
	FString AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset")
	FString AssetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset")
	TArray<FString> DuplicateAssetPaths;

	FDirectiveUtilDuplicateAssetData()
	{
		AssetName = FString();
		AssetClass = FString();
		DuplicateAssetPaths = TArray<FString>();
	}
};