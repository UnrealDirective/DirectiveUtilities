# Changelog

All notable changes to Directive Utilities are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Array sampling with optional replacement, seeded sampling, zero-based pagination, and natural sorting for string and name arrays.
- Editor slow tasks with progress, cancellation checks, explicit finish, and automatic cleanup when the task object is destroyed.
- Editor notifications with neutral, success, warning, and failure states.
- Read-only asset audits for unreferenced candidates, missing references, dependency cycles, disk use, dependency counts, referencer counts, primary assets, and CSV export.
- Read-only Blueprint inspection by compile status, parent class, interface, component class, and unused variables.
- Undoable actor alignment, distribution, and surface snapping with explicit actor inputs and changed/skipped results.
- Automation coverage for the new runtime and editor nodes.

### Changed
- Duplicate removal, distinct copies, and most-common lookup use property hashes when Unreal exposes them.
- Sparse sampling no longer allocates an index for every source item. POD rotation uses bulk moves, while managed values keep property-aware swaps.
- Natural sorting caches comparison data instead of rebuilding it during each comparison.
- Numeric medians use selection instead of sorting the full array. Fuzzy string matching reuses its working buffers.
- Gameplay Tag depth and ancestry queries walk parent tags instead of splitting names into arrays.
- Added repeatable runtime performance tests with CSV baselines for the optimized operations.
- Added table-driven and seeded correctness cases from empty inputs through 4,096 items.
- Runtime automation can now execute in packaged clients, and wildcard pin tests cover every generic array and map node.

### Fixed
- Array slice, distinct, sampling, and pagination functions now support using the same array as both input and output.
- Generic array outputs reject mismatched element types without modifying the destination array.

## [2.0.2] - 2026-07-15

### Fixed
- `Slice` and `Get Distinct` now support using the source array as the output, including arrays with non-trivial element types.
- Array mutators expose mutable pins correctly, and next/previous index helpers handle the full `int32` range without overflow.
- Duration formatting no longer emits `-0m`, handles large finite inputs safely, and weighted selection ignores non-finite weights.
- Bare filename validation rejects `.` and `..`, including when used for save-slot operations.
- Delay and movement tasks handle non-finite values, missing worlds, negative thresholds, and exact-radius completion safely.
- Gameplay Tag registry queries no longer advertise worker-thread safety.

### Changed
- Plugin modules build cleanly with explicit includes and unity builds disabled.

## [2.0.1] - 2026-07-15

### Fixed
- `Append` now resolves both map pins from either connected map, rejects mismatched key or value types, and upgrades existing nodes when their Blueprints load.

## [2.0] - 2026-07-03

### BREAKING: plugin renamed to Directive Utilities
- Plugin, modules, and C++ classes renamed (UDCore → DirectiveUtilities; modules `DirectiveUtilitiesRuntime`/`DirectiveUtilitiesEditor`; classes `UDirectiveUtil*`, async tasks `UDirectiveUtilTask_*`).
- Blueprint assets migrate automatically via bundled CoreRedirects.
- C++ consumers must update Build.cs module names, include paths, and API macros.
- Blueprint palette categories unified under `Directive Utilities|...` (previously `UDCore|...` and `Unreal Directive Toolkit|...`).
- See [Documentation/Migration-2.0.md](Documentation/Migration-2.0.md) for the full migration guide and rename tables.

### Added
- **Array & Map function libraries**: wildcard utility nodes that extend the built-in container operations, including map reverse lookup (`Get Keys By Value`, `Has Value`), bulk removal (`Remove Keys`), and merging (`Append`).
- **Math function library**: easing curves, weighted random selection, rounding helpers, Perlin noise, angle utilities, numeric array statistics (`Sum`, `Average`, `Median`, `StandardDeviation` for int and float arrays), and humanized formatting (`FormatBytes`, `FormatDuration`, `FormatRelativeTime`).
- **Regex function library**: pattern matching, capture groups, and replacement helpers.
- **Save Game function library**: save-slot enumeration, slot timestamps, byte-based save/load helpers, and slot management (`DoesSaveSlotExist`, `DeleteSaveSlot`, and `RenameSaveSlot`; rename never loses the original slot on failure).
- **String function library**: `IsValidFileName` and `SanitizeFileName` (save-slot validation shares them), `FindBestStringMatch` fuzzy matching, case conversion (`SplitIntoWords`, `ToCamelCase`, `ToPascalCase`, `ToSnakeCase`, `ToKebabCase`), hex encoding/decoding for strings and byte arrays, and `Md5`/`Sha1`/`Crc32` hashing (UTF-8, integrity/cache use).
- **Text function library**: text manipulation helpers.
- **Gameplay Tag function library**: hierarchy navigation in both directions (depth, leaf name, segments, parents, `GetTagChildren`, `GetTagDirectChildren`, `GetTagCommonAncestor`, `GetTagAtDepth`, `GetTagSiblings`, `IsLeafTag`) and registry search (`FindRegisteredTags`).
- **Enhanced Input**: subsystem getters and input mapping context mutators (add, remove, swap, clear, query).
- **Async task nodes**: `Delay`, `Async Load Asset`, `Async Load Class`, `Async Load Assets` (batch, with progress and cancel), `Async Trace`, `Move To Location`, and `Move To Actor`.
- **Utility function library**: `HasCommandLineSwitch` and `GetCommandLineOption` for reading launch options from Blueprints.
- **Editor Asset utilities** (`DirectiveUtilEditorAssetLibrary`): editor scripting helpers for querying and managing assets.
- **Automation tests** covering all function libraries.
- Plugin icon (SVG source).

### Deprecated
- `SortStringArray` and `GetSortedStringArray`: UE 5.6+ ships a built-in `Sort String Array` node; both will be removed in the next major version.

### Changed
- Relicensed under the MIT License and standardized copyright headers across the codebase.
- Expanded the Editor Actor Subsystem with additional filters and utilities.
- Adopted a rolling "latest three engine versions" support policy (UE 5.6-5.8); dropped UE 5.5 support and removed its `UEditorActorSubsystem` regression workaround.
- Plugin Core Redirects load from `DefaultDirectiveUtilities.ini`, matching Unreal's current plugin config naming convention.
- `RoundToDecimals` and `RoundToDecimalsAsText` both round half away from zero.
- Static-mesh-name getters use case-insensitive substring matching, matching the filters.
- The mobility filter and getter both test the root component's mobility.
- `Push Override Materials To Source` runs as a single undoable transaction.
- `NetCore` is now a public dependency of the Directive Utilities runtime module.

### Fixed
- Crash-safety hardening: clamp the dot product in `AngleBetweenVectors` to prevent `NaN`, validate save-slot names against empty/path-traversal input, and null-guard editor subsystem access in headless contexts.
- Corrected operator-precedence and added missing null-guards in Editor Actor Subsystem filters.
- Added a null-check when resolving the player controller in the Enhanced Input helpers.
- Corrected the `SortStringArray` deprecation message to reference `GetSortedStringArray`.
- `TruncateString` safely handles maximum lengths shorter than the requested suffix.
- Weighted random selection ignores non-positive weights and never returns a zero-weight entry.
- Enhanced Input add/remove operations report failure when none of the requested mapping contexts load.
- Cancellable Delay resolves the supplied world context and completes for zero or negative durations.
- `Move To Location` completes when path-following stops, and `Completed` fires exactly once.
- `AddInputMappingContexts` no longer clears existing mappings when every new context fails to load.
- `GetSaveSlotTimestamp` returns local time as documented.
- Zero-width regex matches are ignored by `Find All` and `Replace All`.
- Editor Actor Subsystem filters apply Include/Exclude once per actor, including actors with multiple components.
- World-location filters use spherical distance; box and capsule containment respect component transforms and actual shape geometry.
- Texture filters recognize both Texture Sample and Texture Object material expressions.
- Vertex and triangle queries include child-actor mesh components and skip meshes without valid LOD0 render data.
- `FilterEmptyActors` Include/Exclude are now exact complements.
- `GetActorsByBoundingBox` matches bounds within the given box instead of exact equality.
- The lightmap-resolution filter honors `bOverrideLightMapRes`.
- Editor asset queries wait for the asset-registry scan to complete.
- Packaged builds now include the README, LICENSE, CHANGELOG, and Documentation folder.
- Gameplay Tag nodes that query the tag manager no longer claim worker-thread safety.
- Duplicate asset searches return each matching asset path once when directories overlap.
- Async action factories handle null world contexts without emitting engine-level warnings.
- CI runs the Directive Utilities automation suite for every supported engine version.

## [1.3] and earlier

See the [git tag history](https://github.com/UnrealDirective/DirectiveUtilities/tags) for prior releases.
