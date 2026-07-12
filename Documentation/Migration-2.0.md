# Migrating from UDCore 1.x to Directive Utilities 2.0

In 2.0 the plugin was renamed from **UDCore** to **Directive Utilities**, joining
the Directive plugin family. Everything was renamed: the plugin descriptor,
modules, C++ classes, types, and Blueprint categories. This guide covers what
changed and what (if anything) you need to do.

**Short version: Blueprint-only projects migrate on load.** The plugin bundles
Core Redirects that map every 1.x name to its 2.0 equivalent, so existing
Blueprints load and compile unchanged. C++ projects need a small, mechanical
update (tables below).

## What was renamed

| | UDCore 1.x | Directive Utilities 2.0 |
|---|---|---|
| Plugin descriptor | `UDCore.uplugin` | `DirectiveUtilities.uplugin` |
| Runtime module | `UDCore` | `DirectiveUtilitiesRuntime` |
| Editor module | `UDCoreEditor` | `DirectiveUtilitiesEditor` |
| Test module | `UDCoreTests` | `DirectiveUtilitiesTests` |
| Function libraries | `UUDCore<X>FunctionLibrary` | `UDirectiveUtil<X>FunctionLibrary` |
| Async task nodes | `UUDAT_<X>` | `UDirectiveUtilTask_<X>` (in `Tasks/`) |
| Editor subsystem / asset library | `UUDCoreEditor…` | `UDirectiveUtilEditor…` |
| Structs | `FUDCore…`, `FUDAssetKey`, `FUDDuplicateAssetData` | `FDirectiveUtil…` |
| Enums | `EUD…` (Inclusivity, SuccessStatus, EaseType, …) | `EDirectiveUtil…` |
| Log categories | `LogUDCore`, `LogUDCoreEditor` | `LogDirectiveUtil`, `LogDirectiveUtilEditor` |
| Blueprint categories | `UDCore\|…` and `Unreal Directive Toolkit\|…` | `Directive Utilities\|…` |
| API macros | `UDCORE_API`, `UDCOREEDITOR_API` | `DIRECTIVEUTILITIESRUNTIME_API`, `DIRECTIVEUTILITIESEDITOR_API` |

Blueprint **function names, pins, and behavior did not change**: only the
classes that own them.

## Blueprint projects

1. Remove the old plugin folder (`Plugins/UDCore`) and install
   `Plugins/DirectiveUtilities` in its place.
2. In your `.uproject`, change the plugin entry `"Name": "UDCore"` to
   `"Name": "DirectiveUtilities"`.
3. Open the project. Existing Blueprints fix themselves up on load via the
   bundled Core Redirects: nodes stay wired and compile as before.
4. Recommended: **re-save the affected Blueprints** once they're open. Saving
   bakes the new names into the assets, so they no longer depend on the
   redirects at load time.

### How the redirects work

`Config/DefaultDirectiveUtilities.ini` inside the plugin carries a
`[CoreRedirects]` table (2 package, 19 class, 3 struct, and 5 enum redirects)
mapping every Blueprint-visible 1.x name to its 2.0 equivalent. Unreal reads
these when the plugin loads, so assets saved against `/Script/UDCore.*` resolve
to `/Script/DirectiveUtilitiesRuntime.*` transparently. The file ships in
packaged builds (it is listed in `Config/FilterPlugin.ini`) and will remain
bundled for the 2.x lifetime, but re-saving assets is still the durable fix.

If a node ever appears as an unknown/ghost node after migrating, check the
Output Log for `LogCoreRedirects` warnings and report it on the issue tracker.
That would indicate a missing redirect row.

## C++ projects

1. `Build.cs`: replace module references: `"UDCore"` →
   `"DirectiveUtilitiesRuntime"`, `"UDCoreEditor"` → `"DirectiveUtilitiesEditor"`.
2. Includes: library headers moved with their classes, e.g.
   `#include "Libraries/UDCoreStringFunctionLibrary.h"` →
   `#include "Libraries/DirectiveUtilStringFunctionLibrary.h"`. Async task
   headers now live under `Tasks/`, for example
   `#include "Tasks/DirectiveUtilTask_MoveToLocation.h"`.
3. Rename symbols per the table above (a project-wide find/replace of
   `UUDCore` → `UDirectiveUtil`, `UUDAT_` → `UDirectiveUtilTask_`,
   `FUDCore` → `FDirectiveUtil`, and `EUD` → `EDirectiveUtil` covers almost
   everything).

Core Redirects do not help C++: the compiler errors will point at anything the
find/replace missed.

## Also in 2.0

`SortStringArray` and `GetSortedStringArray` are deprecated (UE 5.6+ ships a
built-in `Sort String Array` node) and will be removed in the next major
version. See the [CHANGELOG](../CHANGELOG.md) for the full 2.0 release notes.
