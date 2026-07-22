# Fab technical information

Directive Utilities is an open-source Unreal Engine plugin that fills gaps in the Blueprint library and editor scripting APIs. It ships as readable C++ source, and its Blueprint nodes can also be called from C++.

Source Code: https://github.com/UnrealDirective/DirectiveUtilities

## Features

- Runtime Blueprint libraries for arrays, maps, strings, text, regular expressions, math, Gameplay Tags, save games, input, clipboard access, and project context.
- Async Blueprint actions for timed updates, repeated work, asset loading, traces, and AI movement.
- Read-only editor searches and content audits for assets and Blueprints.
- Actor layout, filtering, selection, viewport, material, and mesh utilities for editor scripts.
- Progress tasks and notifications for Editor Utility Widgets.

## Code modules

- `DirectiveUtilitiesRuntime` (Runtime)
- `DirectiveUtilitiesBlueprintNodes` (UncookedOnly)
- `DirectiveUtilitiesEditor` (Editor)
- `DirectiveUtilitiesTests` (DeveloperTool, Editor targets only)

Number of Blueprints: 0

Number of C++ Classes: 28 production UCLASS types

Network Replicated: No. The utilities run locally and can be called from replicated gameplay code.

Supported Development Platforms: Win64, Mac, Linux

Supported Target Build Platforms: Win64, Mac, Linux

Documentation Link: https://github.com/UnrealDirective/DirectiveUtilities/tree/main/Documentation

Example Project: Not included

## Requirements

Supported Unreal Engine versions: 5.6, 5.7, and 5.8

Engine plugin dependencies:

- Enhanced Input, used by the runtime input utilities
- Editor Scripting Utilities, used by the editor-only utilities

Both dependencies ship with Unreal Engine and are enabled by the plugin descriptor. The plugin has no third-party libraries or external services. Source compilation requires the C++ toolchain supported by the selected Unreal Engine installation.

## Important notes

The runtime module is available in packaged projects. Blueprint compiler, editor, and test modules are excluded from packaged games.
