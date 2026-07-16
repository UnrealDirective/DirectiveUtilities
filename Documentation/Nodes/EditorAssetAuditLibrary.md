# Editor Asset Audit Library

> Read-only content checks backed by the Asset Registry.

**Module:** `DirectiveUtilitiesEditor (Editor)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesEditor/Public/Libraries/DirectiveUtilEditorAssetAuditLibrary.h`

Every scan waits for the Asset Registry's initial load. The default scan path is `/Game`. Developer content, external actor and object packages, redirectors, and test folders are excluded. Add project-specific exclusions through `FDirectiveUtilAssetAuditOptions`.

Unreferenced results are candidates. The Asset Registry cannot see every runtime string lookup or indirect load, so these nodes do not claim that an asset is safe to delete. The library does not delete, rename, consolidate, save, or modify assets.

---

## Find Unreferenced Asset Candidates
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Asset Audit`

```cpp
static TArray<FAssetData> FindUnreferencedAssetCandidates(
    const FDirectiveUtilAssetAuditOptions& Options);
```

Returns assets with no hard, soft, searchable-name, or management referencers recorded by the Asset Registry. Primary assets are excluded unless `bIncludePrimaryAssets` is enabled.

## Find Missing Asset References
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Asset Audit`

```cpp
static TArray<FDirectiveUtilMissingAssetReference> FindMissingAssetReferences(
    const FDirectiveUtilAssetAuditOptions& Options);
```

Returns the referencing asset, missing package, and dependency type for recorded dependencies whose package is absent. Dependency types distinguish hard packages, soft packages, searchable names, direct management, and indirect management.

## Find Asset Dependency Cycles
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Asset Audit`

```cpp
static TArray<FDirectiveUtilAssetDependencyCycle> FindAssetDependencyCycles(
    const FDirectiveUtilAssetAuditOptions& Options);
```

Returns groups of scanned packages that form dependency cycles. Packages outside the scan are not included in cycle traversal.

## Build Asset Audit Report
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Asset Audit`

```cpp
static FDirectiveUtilAssetAuditReport BuildAssetAuditReport(
    const FDirectiveUtilAssetAuditOptions& Options);
```

Builds one report containing asset rows, missing references, and dependency cycles. Each asset row includes asset data, package name, package path, class, on-disk size, dependency count, referencer count, primary-asset state, and findings.

## Asset Audit Report To CSV
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Asset Audit`

```cpp
static FString AssetAuditReportToCsv(const FDirectiveUtilAssetAuditReport& Report);
```

Returns CSV text with separate asset, missing-reference, and dependency-cycle sections. Values are quoted and embedded quotes are escaped.
