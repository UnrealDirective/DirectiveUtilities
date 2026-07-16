# Directive Utilities

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6%20to%205.8-0E1128?logo=unrealengine)](Documentation/Compatibility.md)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

Directive Utilities is an open-source Unreal Engine plugin that fills gaps in the Blueprint library and editor scripting APIs. It ships as readable C++ source, and its Blueprint nodes can also be called from C++.

Version 2.0 replaces UDCore 1.x. Existing Blueprint assets migrate through bundled Core Redirects. C++ projects need to update module names, include paths, classes, types, and API macros as described in the [2.0 migration guide](Documentation/Migration-2.0.md).

## What is included

| Area | Utilities |
|------|-----------|
| Arrays and maps | Wildcard container nodes for indexing, lookup, removal, merging, sampling, pagination, natural sorting, and duplicate handling. |
| Strings, text, and regex | Validation, filtering, case conversion, fuzzy matching, hashing, encoding, capture groups, and replacement. |
| Math | Easing, weighted random selection, rounding, angle helpers, Perlin noise, formatting, and array statistics. |
| Gameplay Tags | Hierarchy navigation, ancestry checks, sibling queries, and registry search. |
| Save games | Slot enumeration, timestamps, byte serialization, validation, deletion, and renaming. |
| Enhanced Input | Subsystem access and mapping-context operations. |
| Async work | Delay, asset and class loading, traces, and navigation movement. |
| Editor actors | Actor filters, selection tools, viewport controls, layout operations, surface snapping, material operations, and mesh queries. |
| Editor assets | Asset Registry queries, read-only content audits, and asset management helpers for editor scripts. |
| Editor Blueprints | Read-only searches for compile status, inheritance, interfaces, components, and unused variables. |
| Editor tasks | Progress dialogs and notifications for Editor Utility Widgets and scripts. |

Blueprint nodes appear under `Directive Utilities` in the Blueprint palette. The [node reference](Documentation/README.md#node-reference) lists every exposed function with its category, parameters, outputs, and edge cases.

## Compatibility

| | Support |
|---|---|
| Unreal Engine | 5.6, 5.7, and 5.8 |
| Platforms | Win64, Mac, and Linux |
| Distribution | Source plugin |
| License | MIT |

Module boundaries keep editor dependencies out of packaged games.

| Module | Use |
|--------|-----|
| `DirectiveUtilitiesRuntime` | Runtime libraries and async actions. This is the only module that ships in packaged games. |
| `DirectiveUtilitiesEditor` | Asset and actor tools for editor targets. |
| `DirectiveUtilitiesTests` | Editor automation tests. |

See [Compatibility](Documentation/Compatibility.md) for the support policy and build-target details.

## Installation

The full [installation guide](Documentation/Installation.md) covers upgrades and troubleshooting.

### GitHub release

Prebuilt plugin packages are attached to [GitHub releases](https://github.com/UnrealDirective/DirectiveUtilities/releases). Choose the plugin version you need, then download the archive for your Unreal Engine version and extract it to `YourProject/Plugins/DirectiveUtilities`. A matching release package does not need to be compiled locally.

If the release does not include your engine version, install the source with Git.

### Git

```sh
git clone https://github.com/UnrealDirective/DirectiveUtilities.git DirectiveUtilities
```

Copy the cloned folder to `YourProject/Plugins/DirectiveUtilities`. The plugin descriptor should be at:

```text
YourProject/Plugins/DirectiveUtilities/DirectiveUtilities.uplugin
```

Open the project, enable `Directive Utilities` from the Plugins window, and restart the editor. Unreal builds the plugin with C++ projects. Blueprint-only projects may prompt you to compile it on first launch.

### Updating from UDCore

Remove `Plugins/UDCore` before installing `Plugins/DirectiveUtilities`. Blueprint references migrate when the project loads. C++ consumers must update their dependencies and renamed symbols before compiling. Follow the [2.0 migration guide](Documentation/Migration-2.0.md) for the complete rename table.

## Usage

### Blueprint

Open a Blueprint graph and search for `Directive Utilities`. Subcategories separate containers, math, strings, input, save games, async actions, and editor tools. Editor-only nodes are unavailable in runtime Blueprints and packaged games.

### C++

Add the runtime module to your target's `.Build.cs` file:

```csharp
PublicDependencyModuleNames.Add("DirectiveUtilitiesRuntime");
```

Include the library you need and call its static functions:

```cpp
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"

const FString SaveName = TEXT("Campaign_01");
const bool bIsValidSaveName = UDirectiveUtilStringFunctionLibrary::IsValidFileName(SaveName);
```

Editor modules and editor-only targets can depend on `DirectiveUtilitiesEditor`. Runtime targets should never reference that module.

## Documentation

- [Installation](Documentation/Installation.md) covers setup, verification, updates, and common failures.
- [Node reference](Documentation/README.md#node-reference) links to every Blueprint library and async action.
- [Compatibility](Documentation/Compatibility.md) defines supported engines, platforms, module boundaries, and packaged-game behavior.
- [Migration 2.0](Documentation/Migration-2.0.md) maps UDCore names to their Directive Utilities replacements.
- [Changelog](CHANGELOG.md) tracks releases and behavior changes.

The hosted documentation is available at [udcore.unrealdirective.com](https://udcore.unrealdirective.com/).

## Contributing

When behavior changes, update its implementation and tests together. Revise the matching page under `Documentation/Nodes/` in the same pull request, then build the plugin for each affected engine version.

Open pull requests against `dev` and describe any Blueprint compatibility impact.

## Support

Use [GitHub Issues](https://github.com/UnrealDirective/DirectiveUtilities/issues) for bug reports and feature requests.

## License

Directive Utilities is available under the [MIT License](LICENSE).

## Maintainers

Directive Utilities is maintained by [Unreal Directive](https://unrealdirective.com/) and [Dylan "Tezenari" Amos](https://dylanamos.com/).
