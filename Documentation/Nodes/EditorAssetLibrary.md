# Editor Asset Library

> Blueprint helpers for querying and managing editor assets.

**Module:** `DirectiveUtilitiesEditor (Editor)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesEditor/Public/Libraries/DirectiveUtilEditorAssetLibrary.h`

Query functions wait for any in-progress asset-registry scan to complete before returning, so results are never silently partial during editor startup or after large imports.

---

## Get Asset Data List From Directory
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static TArray<FAssetData> GetAssetDataListFromDirectory(const FString& DirectoryPath, bool bRecursive = true);
```

Retrieves a list of asset data for the given directory.

| Parameter | Type | Description |
|-----------|------|-------------|
| DirectoryPath | `const FString&` | Directory path of the asset we want the list from. (e.g., /Game/MyFolder or /MyPluginName/MyFolder) |
| bRecursive | `bool` | The search will be recursive and will look in subfolders. Defaults to true. |

**Returns:** `TArray<FAssetData>`: List of asset data.

## Find Duplicate Assets
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> FindDuplicateAssets(
    const TArray<FString>& DirectoryPaths,
    bool bRecursive = true);
```

Finds and returns a list of duplicate assets within the given directories. The criteria for duplication is based on the asset name and class.

| Parameter | Type | Description |
|-----------|------|-------------|
| DirectoryPaths | `const TArray<FString>&` | List of directory paths to search for duplicate assets. (e.g., /Game/MyFolder or /MyPluginName/MyFolder) |
| bRecursive | `bool` | The search will be recursive and will look in subfolders. Defaults to true. |

**Returns:** `TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData>`: Mapped list of duplicate assets.

## Fix Up Redirectors In Paths
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static EDirectiveUtilSuccessStatus FixUpRedirectorsInPaths(const TArray<FString>& DirectoryPaths, int32& OutRedirectorsProcessed);
```

Fixes up (and deletes) object redirectors found under the given directories, without loading every asset. Equivalent to the Content Browser's "Fix Up Redirectors in Folder", but scriptable and headless-friendly.

| Parameter | Type | Description |
|-----------|------|-------------|
| DirectoryPaths | `const TArray<FString>&` | Directories to scan for redirectors. If empty, the entire registry is scanned. |
| OutRedirectorsProcessed | `int32&` | [out] The number of redirectors submitted for fix-up (the engine does not report per-redirector success). |

**Returns:** `EDirectiveUtilSuccessStatus` (exposed as exec pins): Success if the operation ran (even if nothing needed fixing), Failure otherwise (e.g. a fixup is already in progress).

## Get Assets By Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static TArray<FAssetData> GetAssetsByClass(
    UClass* AssetClass,
    const FString& PackagePath,
    bool bRecursiveClasses,
    bool bRecursivePaths,
    EDirectiveUtilSuccessStatus& OutStatus);
```

Finds all assets of the given class using the Asset Registry (no asset loading).

| Parameter | Type | Description |
|-----------|------|-------------|
| AssetClass | `UClass*` | The class to search for. |
| PackagePath | `const FString&` | An optional package path to scope the search (e.g. /Game/MyFolder). Empty searches everywhere. |
| bRecursiveClasses | `bool` | If true, also matches subclasses of AssetClass. |
| bRecursivePaths | `bool` | If true, also searches subfolders of PackagePath. |
| OutStatus | `EDirectiveUtilSuccessStatus&` | [out] Success if the registry was queried successfully. (Exposed as exec pins.) |

**Returns:** `TArray<FAssetData>`: The matching asset data.

## Get Asset Dependencies
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static TArray<FString> GetAssetDependencies(const FAssetData& Asset, bool bHardDependenciesOnly, EDirectiveUtilSuccessStatus& OutStatus);
```

Returns the package paths of the assets that the given asset depends on, using the Asset Registry dependency graph.

| Parameter | Type | Description |
|-----------|------|-------------|
| Asset | `const FAssetData&` | The asset whose dependencies to retrieve. |
| bHardDependenciesOnly | `bool` | If true, only hard (always-loaded) dependencies are returned. |
| OutStatus | `EDirectiveUtilSuccessStatus&` | [out] Success if the registry was queried successfully. (Exposed as exec pins.) |

**Returns:** `TArray<FString>`: The dependency package paths.

## Get Asset Referencers
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static TArray<FString> GetAssetReferencers(const FAssetData& Asset, bool bHardReferencesOnly, EDirectiveUtilSuccessStatus& OutStatus);
```

Returns the package paths of the assets that reference the given asset, using the Asset Registry dependency graph.

| Parameter | Type | Description |
|-----------|------|-------------|
| Asset | `const FAssetData&` | The asset whose referencers to retrieve. |
| bHardReferencesOnly | `bool` | If true, only hard (always-loaded) referencers are returned. |
| OutStatus | `EDirectiveUtilSuccessStatus&` | [out] Success if the registry was queried successfully. (Exposed as exec pins.) |

**Returns:** `TArray<FString>`: The referencer package paths.

## Get Default Asset Name For Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities | Editor Scripting | Asset`

```cpp
static FString GetDefaultAssetNameForClass(UClass* AssetClass, EDirectiveUtilSuccessStatus& OutStatus);
```

Returns the default name a new asset of the given class would receive, honoring the project's asset naming convention overrides where the engine supports them. Naming-convention overrides are only consulted on UE 5.8+; on 5.6/5.7 the engine's plain default name is returned.

| Parameter | Type | Description |
|-----------|------|-------------|
| AssetClass | `UClass*` | The class to resolve a default asset name for. |
| OutStatus | `EDirectiveUtilSuccessStatus&` | [out] Success if a non-empty name was resolved. (Exposed as exec pins.) |

**Returns:** `FString`: The default asset name, or an empty string on failure.
