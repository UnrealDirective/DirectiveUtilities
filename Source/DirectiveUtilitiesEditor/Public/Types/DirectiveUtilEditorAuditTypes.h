// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "DirectiveUtilEditorAuditTypes.generated.h"

UENUM(BlueprintType)
enum class EDirectiveUtilAssetDependencyType : uint8
{
	HardPackage UMETA(DisplayName = "Hard Package"),
	SoftPackage UMETA(DisplayName = "Soft Package"),
	SearchableName UMETA(DisplayName = "Searchable Name"),
	DirectManagement UMETA(DisplayName = "Direct Management"),
	IndirectManagement UMETA(DisplayName = "Indirect Management"),
};

USTRUCT(BlueprintType)
struct FDirectiveUtilAssetAuditOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directive Utilities|Asset Audit")
	TArray<FName> PackagePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directive Utilities|Asset Audit")
	TArray<FName> ExcludedPackagePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directive Utilities|Asset Audit")
	bool bIncludePrimaryAssets = false;

	FDirectiveUtilAssetAuditOptions()
	{
		PackagePaths.Add(TEXT("/Game"));
	}
};

USTRUCT(BlueprintType)
struct FDirectiveUtilMissingAssetReference
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FAssetData ReferencingAsset;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FName MissingPackage;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	EDirectiveUtilAssetDependencyType DependencyType = EDirectiveUtilAssetDependencyType::SoftPackage;
};

USTRUCT(BlueprintType)
struct FDirectiveUtilAssetDependencyCycle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	TArray<FName> Packages;
};

USTRUCT(BlueprintType)
struct FDirectiveUtilAssetAuditEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FAssetData Asset;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FName PackageName;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FName PackagePath;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	FString AssetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	int64 DiskSize = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	int32 DependencyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	int32 ReferencerCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	bool bPrimaryAsset = false;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	TArray<FString> Findings;
};

USTRUCT(BlueprintType)
struct FDirectiveUtilAssetAuditReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	TArray<FDirectiveUtilAssetAuditEntry> Assets;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	TArray<FDirectiveUtilMissingAssetReference> MissingReferences;

	UPROPERTY(BlueprintReadOnly, Category = "Directive Utilities|Asset Audit")
	TArray<FDirectiveUtilAssetDependencyCycle> DependencyCycles;
};
