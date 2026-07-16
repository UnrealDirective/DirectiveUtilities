// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilEditorAssetLibrary.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Algo/Transform.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "IAssetTools.h"
#include "Misc/AssetRegistryInterface.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/ObjectRedirector.h"

namespace
{
	void WaitForAssetRegistry()
	{
		if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
		{
			if (AssetRegistry->IsLoadingAssets())
			{
				AssetRegistry->WaitForCompletion();
			}
		}
	}
}

TArray<FAssetData> UDirectiveUtilEditorAssetLibrary::GetAssetDataListFromDirectory(
	const FString& DirectoryPath,
	const bool bRecursive)
{
	WaitForAssetRegistry();

	TArray<FAssetData> AssetDataList;

	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>() : nullptr;
	if (!EditorAssetSubsystem)
	{
		return AssetDataList;
	}

	const TArray<FString> AssetPaths = EditorAssetSubsystem->ListAssets(DirectoryPath, bRecursive, false);

	Algo::Transform(AssetPaths, AssetDataList, [EditorAssetSubsystem](const FString& AssetPath) {
		return EditorAssetSubsystem->FindAssetData(AssetPath);
	});

	AssetDataList.RemoveAll([](const FAssetData& AssetData) {
		return !AssetData.IsValid();
	});

	return AssetDataList;
}

TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> UDirectiveUtilEditorAssetLibrary::FindDuplicateAssets(
	const TArray<FString>& DirectoryPaths,
	const bool bRecursive)
{
	TArray<FAssetData> CombinedAssetDataList;

	for (const FString& DirectoryPath : DirectoryPaths)
	{
		const TArray<FAssetData> AssetDataList = GetAssetDataListFromDirectory(DirectoryPath, bRecursive);
		CombinedAssetDataList.Append(AssetDataList);
	}

	TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> DuplicateAssetsMap;
	for (const FAssetData& AssetData : CombinedAssetDataList)
	{
		if (!AssetData.IsValid()) { continue; }

		const FString AssetName = AssetData.AssetName.ToString();
		const FString AssetClass = AssetData.AssetClassPath.ToString();
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();
		const FDirectiveUtilAssetKey AssetKey(AssetName, AssetClass);

		if (FDirectiveUtilDuplicateAssetData* ExistingData = DuplicateAssetsMap.Find(AssetKey))
		{
			ExistingData->DuplicateAssetPaths.AddUnique(AssetPath);
		}
		else
		{
			FDirectiveUtilDuplicateAssetData DuplicateAssetData;
			DuplicateAssetData.AssetName = AssetName;
			DuplicateAssetData.AssetClass = AssetClass;
			DuplicateAssetData.DuplicateAssetPaths.Add(AssetPath);
			DuplicateAssetsMap.Emplace(AssetKey, MoveTemp(DuplicateAssetData));
		}
	}

	TArray<FDirectiveUtilAssetKey> KeysToRemove;
	for (const auto& Pair : DuplicateAssetsMap)
	{
		if (Pair.Value.DuplicateAssetPaths.Num() <= 1)
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const FDirectiveUtilAssetKey& Key : KeysToRemove)
	{
		DuplicateAssetsMap.Remove(Key);
	}

	return DuplicateAssetsMap;
}

EDirectiveUtilSuccessStatus UDirectiveUtilEditorAssetLibrary::FixUpRedirectorsInPaths(const TArray<FString>& DirectoryPaths, int32& OutRedirectorsProcessed)
{
	OutRedirectorsProcessed = 0;

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	if (AssetRegistry->IsLoadingAssets())
	{
		AssetRegistry->WaitForCompletion();
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();
	if (AssetTools.IsFixupReferencersInProgress())
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	FARFilter Filter;
	Filter.bRecursiveClasses = false;
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
	if (DirectoryPaths.Num() > 0)
	{
		Filter.bRecursivePaths = true;
		for (const FString& DirectoryPath : DirectoryPaths)
		{
			Filter.PackagePaths.Add(FName(*DirectoryPath));
		}
	}

	TArray<FAssetData> RedirectorAssets;
	if (!AssetRegistry->GetAssets(Filter, RedirectorAssets))
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	TArray<UObjectRedirector*> Redirectors;
	Redirectors.Reserve(RedirectorAssets.Num());
	for (const FAssetData& RedirectorData : RedirectorAssets)
	{
		if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			Redirectors.Add(Redirector);
		}
	}

	if (Redirectors.Num() == 0)
	{
		return EDirectiveUtilSuccessStatus::Success;
	}

	AssetTools.FixupReferencers(Redirectors, false, ERedirectFixupMode::DeleteFixedUpRedirectors);
	OutRedirectorsProcessed = Redirectors.Num();
	return EDirectiveUtilSuccessStatus::Success;
}

TArray<FAssetData> UDirectiveUtilEditorAssetLibrary::GetAssetsByClass(
	UClass* AssetClass,
	const FString& PackagePath,
	const bool bRecursiveClasses,
	const bool bRecursivePaths,
	EDirectiveUtilSuccessStatus& OutStatus)
{
	WaitForAssetRegistry();

	TArray<FAssetData> Result;
	OutStatus = EDirectiveUtilSuccessStatus::Failure;

	if (!IsValid(AssetClass))
	{
		return Result;
	}

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return Result;
	}

	FARFilter Filter;
	Filter.ClassPaths.Add(AssetClass->GetClassPathName());
	Filter.bRecursiveClasses = bRecursiveClasses;
	if (!PackagePath.IsEmpty())
	{
		Filter.PackagePaths.Add(FName(*PackagePath));
		Filter.bRecursivePaths = bRecursivePaths;
	}

	if (AssetRegistry->GetAssets(Filter, Result))
	{
		OutStatus = EDirectiveUtilSuccessStatus::Success;
	}
	return Result;
}

TArray<FString> UDirectiveUtilEditorAssetLibrary::GetAssetDependencies(const FAssetData& Asset, const bool bHardDependenciesOnly, EDirectiveUtilSuccessStatus& OutStatus)
{
	using namespace UE::AssetRegistry;

	WaitForAssetRegistry();

	TArray<FString> Result;
	OutStatus = EDirectiveUtilSuccessStatus::Failure;

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry || !Asset.IsValid())
	{
		return Result;
	}

	TArray<FName> Dependencies;
	const FDependencyQuery Query = bHardDependenciesOnly ? FDependencyQuery(EDependencyQuery::Hard) : FDependencyQuery();
	if (AssetRegistry->GetDependencies(Asset.PackageName, Dependencies, EDependencyCategory::Package, Query))
	{
		Result.Reserve(Dependencies.Num());
		for (const FName& Dependency : Dependencies)
		{
			Result.Add(Dependency.ToString());
		}
		OutStatus = EDirectiveUtilSuccessStatus::Success;
	}
	return Result;
}

TArray<FString> UDirectiveUtilEditorAssetLibrary::GetAssetReferencers(const FAssetData& Asset, const bool bHardReferencesOnly, EDirectiveUtilSuccessStatus& OutStatus)
{
	using namespace UE::AssetRegistry;

	WaitForAssetRegistry();

	TArray<FString> Result;
	OutStatus = EDirectiveUtilSuccessStatus::Failure;

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry || !Asset.IsValid())
	{
		return Result;
	}

	TArray<FName> Referencers;
	const FDependencyQuery Query = bHardReferencesOnly ? FDependencyQuery(EDependencyQuery::Hard) : FDependencyQuery();
	if (AssetRegistry->GetReferencers(Asset.PackageName, Referencers, EDependencyCategory::Package, Query))
	{
		Result.Reserve(Referencers.Num());
		for (const FName& Referencer : Referencers)
		{
			Result.Add(Referencer.ToString());
		}
		OutStatus = EDirectiveUtilSuccessStatus::Success;
	}
	return Result;
}

FString UDirectiveUtilEditorAssetLibrary::GetDefaultAssetNameForClass(UClass* AssetClass, EDirectiveUtilSuccessStatus& OutStatus)
{
	OutStatus = EDirectiveUtilSuccessStatus::Failure;

	if (!IsValid(AssetClass))
	{
		return FString();
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 8
	const TOptional<FString> ResolvedName = AssetTools.GetDefaultAssetNameForClass(AssetClass, nullptr, nullptr);
#else
	const TOptional<FString> ResolvedName = AssetTools.GetDefaultAssetNameForClass(AssetClass);
#endif

	if (ResolvedName.IsSet() && !ResolvedName.GetValue().IsEmpty())
	{
		OutStatus = EDirectiveUtilSuccessStatus::Success;
		return ResolvedName.GetValue();
	}
	return FString();
}
