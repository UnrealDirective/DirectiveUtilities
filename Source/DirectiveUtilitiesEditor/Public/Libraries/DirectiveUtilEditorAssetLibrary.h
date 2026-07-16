// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "AssetRegistry/AssetData.h"
#include "CoreMinimal.h"
#include "EditorAssetLibrary.h"
#include "Types/DirectiveUtilEditorAssetTypes.h"
#include "Types/DirectiveUtilTypes.h"
#include "DirectiveUtilEditorAssetLibrary.generated.h"

/**
 * UDirectiveUtilEditorAssetLibrary
 *
 * Blueprint helpers for querying and managing editor assets.
 */
UCLASS()
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorAssetLibrary : public UEditorAssetLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Retrieve a list of asset data for the given directory.
	 * @param DirectoryPath	Directory path of the asset we want the list from. (e.g., /Game/MyFolder or /MyPluginName/MyFolder)
	 * @param bRecursive The search will be recursive and will look in subfolders. Defaults to true.
	 * @return TArray<FAssetData> List of asset data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities | Editor Scripting | Asset")
	static TArray<FAssetData> GetAssetDataListFromDirectory(const FString& DirectoryPath, bool bRecursive = true);

	/**
	 * Find and return a list of duplicate assets within the given directories.
	 * The criteria for duplication is based on the asset name and class.
	 * @param DirectoryPaths List of directory paths to search for duplicate assets.
	 * (e.g., /Game/MyFolder or /MyPluginName/MyFolder)
	 * @param bRecursive The search will be recursive and will look in subfolders. Defaults to true.
	 * @return TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> Mapped list of duplicate assets.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities | Editor Scripting | Asset")
	static TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> FindDuplicateAssets(
		const TArray<FString>& DirectoryPaths,
		bool bRecursive = true);

	/**
	 * Fixes up (and deletes) object redirectors found under the given directories, without loading every asset.
	 * Equivalent to the Content Browser's "Fix Up Redirectors in Folder", but scriptable and headless-friendly.
	 * @param DirectoryPaths Directories to scan for redirectors. If empty, the entire registry is scanned.
	 * @param OutRedirectorsProcessed [out] The number of redirectors submitted for fix-up (the engine does not report per-redirector success).
	 * @return Success if the operation ran (even if nothing needed fixing), Failure otherwise (e.g. a fixup is already in progress).
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "ReturnValue"), Category = "Directive Utilities | Editor Scripting | Asset")
	static EDirectiveUtilSuccessStatus FixUpRedirectorsInPaths(const TArray<FString>& DirectoryPaths, int32& OutRedirectorsProcessed);

	/**
	 * Finds all assets of the given class using the Asset Registry (no asset loading).
	 * @param AssetClass The class to search for.
	 * @param PackagePath An optional package path to scope the search (e.g. /Game/MyFolder). Empty searches everywhere.
	 * @param bRecursiveClasses If true, also matches subclasses of AssetClass.
	 * @param bRecursivePaths If true, also searches subfolders of PackagePath.
	 * @param OutStatus [out] Success if the registry was queried successfully.
	 * @return The matching asset data.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "OutStatus"), Category = "Directive Utilities | Editor Scripting | Asset")
	static TArray<FAssetData> GetAssetsByClass(
		UClass* AssetClass,
		const FString& PackagePath,
		bool bRecursiveClasses,
		bool bRecursivePaths,
		EDirectiveUtilSuccessStatus& OutStatus);

	/**
	 * Returns the package paths of the assets that the given asset depends on, using the Asset Registry dependency graph.
	 * @param Asset The asset whose dependencies to retrieve.
	 * @param bHardDependenciesOnly If true, only hard (always-loaded) dependencies are returned.
	 * @param OutStatus [out] Success if the registry was queried successfully.
	 * @return The dependency package paths.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "OutStatus"), Category = "Directive Utilities | Editor Scripting | Asset")
	static TArray<FString> GetAssetDependencies(const FAssetData& Asset, bool bHardDependenciesOnly, EDirectiveUtilSuccessStatus& OutStatus);

	/**
	 * Returns the package paths of the assets that reference the given asset, using the Asset Registry dependency graph.
	 * @param Asset The asset whose referencers to retrieve.
	 * @param bHardReferencesOnly If true, only hard (always-loaded) referencers are returned.
	 * @param OutStatus [out] Success if the registry was queried successfully.
	 * @return The referencer package paths.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "OutStatus"), Category = "Directive Utilities | Editor Scripting | Asset")
	static TArray<FString> GetAssetReferencers(const FAssetData& Asset, bool bHardReferencesOnly, EDirectiveUtilSuccessStatus& OutStatus);

	/**
	 * Returns the default name a new asset of the given class would receive, honoring the project's asset naming
	 * convention overrides where the engine supports them.
	 * @note Naming-convention overrides are only consulted on UE 5.8+; on 5.6/5.7 the engine's plain default name is returned.
	 * @param AssetClass The class to resolve a default asset name for.
	 * @param OutStatus [out] Success if a non-empty name was resolved.
	 * @return The default asset name, or an empty string on failure.
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "OutStatus"), Category = "Directive Utilities | Editor Scripting | Asset")
	static FString GetDefaultAssetNameForClass(UClass* AssetClass, EDirectiveUtilSuccessStatus& OutStatus);
};
