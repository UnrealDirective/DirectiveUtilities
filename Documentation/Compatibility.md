# Compatibility

Use this page to decide whether the plugin fits a project before adding it to source control or a build pipeline.

## Engine versions

Directive Utilities follows a rolling **"latest three engine versions"** support policy. The currently supported range is **Unreal Engine 5.6 - 5.8**.

| Engine | Status | Notes |
|--------|--------|-------|
| 5.8 | Supported | Primary local build/test target. |
| 5.7 | Supported | Built by CI. |
| 5.6 | Supported | Built by CI. |

UE 5.5 and earlier are **not supported**. The workaround for the `UEditorActorSubsystem` regression tracked in [#6](https://github.com/UnrealDirective/DirectiveUtilities/issues/6) was removed when 5.5 dropped out of the support window.

The plugin descriptor intentionally omits an `EngineVersion` field so the source distribution installs on any of the supported engines.

## Platforms

| Platform | Runtime | Editor |
|----------|---------|--------|
| Win64 | Yes | Yes |
| Mac | Yes | Yes |
| Linux | Yes | Yes |

## Module layout

| Module | Type | Ships in packaged game? |
|--------|------|--------------------------|
| `DirectiveUtilitiesRuntime` | Runtime | Yes: this is the only module that ships. |
| `DirectiveUtilitiesEditor` | Editor | No: editor builds only. |
| `DirectiveUtilitiesTests` | DeveloperTool (Editor target only) | No: automation tests, excluded from all packaged games. |

## Build Targets

- Runtime gameplay code should depend on `DirectiveUtilitiesRuntime`.
- Editor tooling should depend on `DirectiveUtilitiesEditor`.
- Packaged-game targets must not depend on `DirectiveUtilitiesEditor` or `DirectiveUtilitiesTests`.
- The test module is for editor automation only and is not part of the installed runtime surface.

## Languages

Core functionality is written in C++ and exposed to **Blueprints** and **Python**, so it is usable from any of the three.
