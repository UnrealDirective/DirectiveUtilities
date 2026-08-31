# Changelog

All notable changes to Directive Utilities are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Added cancellable async text and binary file reads and writes, with atomic writes enabled by default.
- Added runtime file watching for created, modified, and deleted changes in packaged desktop games.
- Added the File System function library for text and binary reads, writes, appends, atomic replacement, file sizes, and last-modified timestamps. Paths accept absolute forms or forms relative to the project `Saved` directory. Existing engine nodes remain the source for path parsing, existence checks, copy, move, delete, directory operations, and enumeration.
- Added the CSV function library with RFC-style quoted-field parsing and writing, header-based access, keyed upsert and removal, header and shape validation, and keyed diffs. Blank lines are skipped, explicit empty rows round-trip as one empty cell, and a failed parse yields an empty document rather than a partial one.
- Added `EDirectiveUtilCsvDelimiter`, `FDirectiveUtilCsvRow`, and `FDirectiveUtilCsvDocument` for the CSV library.
- Added runtime DataTable support for strict CSV import and export, rollback-safe in-place replacement, and reflected row diffs. Native `FTableRowBase` types and Blueprint Structure assets support Boolean, finite numeric, enumeration, string, name, text, and value-only struct properties. Imports run Unreal's row-import and table-change lifecycle.
- Added the Config function library for .ini files: reads with defaults and writes for string, int32, int64, float, bool, string array, Vector2D, Vector, Rotator, and Color values, plus key and section queries and removal. Writes atomically replace files, create missing directories, and refuse malformed existing content.
- Added the Date Time function library: custom pattern formatting and parsing (`yyyy`, `MM`, `dd`, `HH`, `mm`, `ss`, `fff` tokens with quoted literals), UTC-to-local and local-to-UTC conversion using per-instant timezone rules, and clock-style time span formatting. Conversions return the input unchanged when the platform's time routines reject a value.
- Added the Bytes function library: standard zlib and gzip streams, bounded compression and decompression of byte arrays, SHA-256 hashing of bytes and UTF-8 strings via a self-contained FIPS 180-4 implementation, and HMAC-SHA256 signing. Uncompressed input and output are limited to 1,000,000 bytes.
- Added `EDirectiveUtilCompressionFormat` (Zlib, Gzip) for the Bytes library.

## [2.2.0] - 2026-08-11

### Added
- Added `Append Array Optimized`, a wildcard array node that bulk-copies plain-data elements and retains Unreal's property-aware path for managed values.
- Added `Insert Array Optimized`, a wildcard array node that inserts a source array with one structural move.
- Added `Remove At Indices`, a stable wildcard array node that removes multiple indices in one pass.
- Added `Remove All Occurrences`, a stable wildcard array node that removes matching values in one pass, with same-build comparisons against Unreal's `Remove Item`.
- Added keyed runtime stopwatches for measuring elapsed real time in milliseconds from Blueprints.
- Added transform variants for grid, hex, circle, arc, and spline point generation, plus facing transforms from locations.
- Added distance-weighted sampling of location and transform arrays, including closed loops.
- Added hex coordinate queries for rectangular grids, rings, ranges, lines, and cell corners.
- Added Perlin noise offsets for location and transform arrays.
- Added transform easing and element-wise location/transform array easing with optional per-element alphas.
- Added `Linear` to `EDirectiveUtilEaseType` (appended so existing Blueprint ordinals stay stable).
- Added `EDirectiveUtilSplineSpacingMode` (Fixed/Even) and `EDirectiveUtilRadialOrientation` for generated transforms.
- Added spline sampling by count, coordinate space, and start/end distance range.
- Added a local release gate for Python checks, Fab validation, UE 5.6-5.8 editor and packaged tests, Shipping coverage, performance regression checks, and release-text scanning.
- `Repeat with Interval` accepts `Count = -1` to repeat until Cancel; `Remaining` stays `-1` while running.

### Changed
- Callable array, random-stream, and collection-generator nodes no longer carry ineffective `BlueprintThreadSafe` metadata. Pure functions retain thread-safe metadata where Unreal can use it.
- `Generate Points Along Spline` keeps `Include Endpoint` as the third pin; Spacing Mode and later options follow it so existing Blueprint and C++ call sites keep their meaning.
- Point, transform, grid, and multi-cell hex generators use execution pins so their allocation cost stays explicit in Blueprint graphs.
- Keyed stopwatches share synchronized state across threads.
- Math formatting, statistics, easing, and random implementations build in focused translation units without changing the public Blueprint library.

### Fixed
- `IsNotEmpty` accepts a Blueprint text literal (`AutoCreateRefTerm`).
- `LerpAngle` returns A at Alpha 0 and no longer wraps the result, so extrapolation does not jump the seam.
- `DeltaAngle` returns +180 for an exactly opposite pair regardless of how the inputs are spelled.
- `AngleBetweenVectors` returns 0 for a zero or non-finite vector instead of 90.
- `RangeFalloff` is full strength at the origin when both radii are 0.
- `EaseAlpha` Back curves are exactly 0 and 1 at the endpoints.
- Save-slot names reject Windows reserved device names (CON, PRN, AUX, NUL, COM1-9, LPT1-9) and trailing dots or spaces.
- `RenameSaveSlot` treats a case-only name change as a rewrite of the existing slot instead of reporting a collision.
- `GetSaveSlotTimestamp` converts UTC to local using the timezone rules for that instant.
- `GetAllSaveSlotNames` enumerates through `ISaveGameSystem` so it agrees with the other slot nodes.
- Fab packages omit repository release files, unused source artwork, and empty local directories while retaining the required code-plugin structure.
- Material queries include any actor with a static mesh component, including custom and Blueprint actor classes.
- Input mapping removal and active-state queries no longer load missing soft assets, and swaps load only the new context after controller validation.
- Distinct and most-common grouping avoid quadratic scans for reflected structs that use field equality without a native hash.
- Push Override Materials To Source copies only explicit component overrides into valid source slots.
- Movement tasks fail cleanly when a world has no navigation system instead of emitting engine warnings.
- Plugin metadata now declares its target platforms, documentation, and editor-only dependency scope.
- Generated collections reject requests above 1,000,000 elements before allocating or iterating.
- Infinite interval repeats wrap their Blueprint index to zero after `MAX_int32` without signed overflow.
- Delta-angle and angle-interpolation nodes remove any number of full turns consistently on UE 5.6-5.8.
- Wildcard append, insert, indexed removal, and value removal now run through compiled Blueprint VM tests for Boolean, string, object, and struct arrays.
- Local release checks reject warning-bearing reports, retain Development and Shipping results separately, run packaged Shipping behavior checks, compile generated Fab archives, and record commit-bound evidence.

## [2.1.0] - 2026-07-22

### Added
- Array sampling with optional replacement, seeded sampling, zero-based pagination, and natural sorting for string and name arrays.
- Weighted array sampling with optional replacement and deterministic random-stream support.
- Cancellable duration updates and fixed-count interval repeats for runtime Blueprints.
- Typed runtime queries for world type, build configuration, and build target.
- Editor slow tasks with progress, cancellation checks, and explicit finish.
- Editor notifications with neutral, success, warning, and failure states.
- Read-only asset audits for unreferenced candidates, missing references, dependency cycles, disk use, dependency counts, referencer counts, primary assets, and CSV export.
- Read-only Blueprint inspection by compile status, parent class, interface, component class, and unused variables.
- Undoable actor alignment, distribution, and surface snapping with explicit actor inputs and changed/skipped results.
- Automation coverage for the new runtime and editor nodes.
- Runtime flow tests and weighted sampling benchmarks across varied collection sizes and sample counts.
- A local runtime test host that runs automation in Unreal Editor and a packaged Development game without changing the released plugin descriptor.

### Changed
- Duplicate removal, distinct copies, and most-common lookup use property hashes when Unreal exposes them.
- Sparse sampling no longer allocates an index for every source item. POD rotation uses bulk moves, while managed values keep property-aware swaps.
- Natural sorting caches comparison data instead of rebuilding it during each comparison.
- Numeric medians use selection instead of sorting the full array. Fuzzy string matching reuses its working buffers.
- Gameplay Tag depth and ancestry queries walk parent tags instead of splitting names into arrays.
- Added repeatable runtime performance tests with CSV baselines for the optimized operations.
- Added table-driven and seeded correctness cases from empty inputs through 4,096 items.
- Runtime automation can now execute in packaged clients, and wildcard pin tests cover every generic array and map node.
- Added a cross-platform packager for engine-specific Fab source archives.
- Declared the supported platforms on every plugin module and aligned the module documentation.
- Save Game slot operations now document their portable flat-filename requirement.

### Fixed
- Array slice, distinct, sampling, and pagination functions now support using the same array as both input and output.
- Generic array outputs reject mismatched element types without modifying the destination array.
- Weighted sampling without replacement stays unique across the full range of finite float weights.
- Movement tasks clear their timers when the controller or pawn becomes invalid and cannot reactivate after completion.
- Editor slow tasks reject non-finite work and overlapping tasks, remain alive until finished, and close during module shutdown.
- Legacy Map Append migration preserves the original node and its links when conversion fails.
- Actor filters skip invalid actors, and missing-texture filtering detects unset texture expressions.
- Actor layout operations reject non-finite inputs and report failed transform changes as skipped.
- Asset Registry queries start the full scan before waiting, and dependency cycle audits handle deep graphs without recursive traversal.
- Windows runtime verification invokes the packaged build command correctly and waits for the packaged game to exit.
- Array sampling rejects unsafe allocation sizes, pagination avoids integer overflow, and float medians return `NaN` for inputs containing `NaN`.
- Async actions stop on world cleanup, and immediate asset-load completions resolve without relying on an assigned streamable handle.
- AI movement cancellation stops active path following, and acceptance checks use horizontal distance.
- Viewport focus skips invalid actors and uses the current level viewport during PIE.
- Asset audits recognize loaded packages, allow default path exclusions to be disabled, and avoid copying dependency arrays during cycle detection.
- Unused Blueprint variable queries omit externally accessible variables by default.
- Clipboard reads are callable nodes so Blueprint execution order remains explicit.
- Fab packaging rejects dirty working trees unless explicitly overridden.
- Added the required publisher notice to every C++ source and header file.

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

## [1.3] and earlier

See the [git tag history](https://github.com/UnrealDirective/DirectiveUtilities/tags) for prior releases.
