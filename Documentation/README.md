# Directive Utilities Documentation

This folder contains setup steps, compatibility details, and node documentation for the checked-out branch or tag.

## Choose Your Path

| Level | Read First | Goal |
|-------|------------|------|
| Beginner | [Installation](Installation.md) | Add the plugin to a project and confirm the nodes are available. |
| Intermediate | [Node Reference](#node-reference) | Look up Blueprint categories, parameters, return values, and edge cases. |
| Expert | [Compatibility](Compatibility.md), [Migration 2.0](Migration-2.0.md), module headers, and the changelog | Check supported engines, module boundaries, packaged-game behavior, and migration details. |

## Setup and Compatibility
- [Installation](Installation.md) - add Directive Utilities to a project, verify the install, and troubleshoot common setup problems.
- [Compatibility](Compatibility.md) - supported engine versions, platforms, module layout, and packaged-game behavior.
- [Migration 2.0](Migration-2.0.md) - rename tables and upgrade steps for projects moving from UDCore 1.x.
- [Coding Standards](Coding-Standards.md) - rules for contributing source, tests, and documentation.

## Node Reference
Per-library reference for every Blueprint-exposed node. Each page lists the module, header, Blueprint category, C++ signature, parameters, outputs, and return behavior.

### Runtime
- [Function Library](Nodes/FunctionLibrary.md) - runtime context, project, clipboard, class, command-line, and keyed profiling helpers.
- [Array Function Library](Nodes/ArrayFunctionLibrary.md) - wildcard array helpers.
- [Map Function Library](Nodes/MapFunctionLibrary.md) - wildcard map helpers.
- [String Function Library](Nodes/StringFunctionLibrary.md) - validation, filtering, matching, formatting, hashing, and case conversion.
- [Text Function Library](Nodes/TextFunctionLibrary.md) - text helpers.
- [Regex Function Library](Nodes/RegexFunctionLibrary.md) - pattern matching and replacement.
- [Math Function Library](Nodes/MathFunctionLibrary.md) - angles, easing, ranges, point generation, weighted random, formatting, noise, and array statistics.
- [Gameplay Tag Function Library](Nodes/GameplayTagFunctionLibrary.md) - tag hierarchy helpers and registry search.
- [Save Game Function Library](Nodes/SaveGameFunctionLibrary.md) - save-slot enumeration, timestamps, byte serialization, and slot management.
- [Input Function Library](Nodes/InputFunctionLibrary.md) - Enhanced Input helpers.
- [File System Function Library](Nodes/FileSystemFunctionLibrary.md) - text and binary I/O, atomic replacement, size, and timestamps.
- [CSV Function Library](Nodes/CsvFunctionLibrary.md) - quoted-field parsing, keyed edits, validation, and diffing.
- [Data Table Function Library](Nodes/DataTableFunctionLibrary.md) - runtime CSV import, safe replacement, export, and row diffing.
- [Config Function Library](Nodes/ConfigFunctionLibrary.md) - .ini reads and writes for scalar, vector, rotation, color, array, key, and section values.
- [Date Time Function Library](Nodes/DateTimeFunctionLibrary.md) - custom pattern format and parse, timezone conversion, and duration formatting.
- [Bytes Function Library](Nodes/BytesFunctionLibrary.md) - zlib and gzip compression, SHA-256 hashing, and HMAC-SHA256 signing.
- [Async Tasks](Nodes/AsyncTasks.md) - timed flow, asset loading, traces, and movement.

### Editor
- [Editor Asset Library](Nodes/EditorAssetLibrary.md) - asset query and management helpers.
- [Editor Asset Audit Library](Nodes/EditorAssetAuditLibrary.md) - read-only content audits and CSV reports.
- [Editor Blueprint Library](Nodes/EditorBlueprintLibrary.md) - read-only Blueprint inspection.
- [Editor Task Library](Nodes/EditorTaskLibrary.md) - progress tasks and notifications.
- [Editor Actor Subsystem](Nodes/EditorActorSubsystem.md) - actor filters, selection helpers, layout operations, and viewport utilities.

## Contributing to the docs
When behavior changes, update the matching page in `Nodes/` in the same pull request. Include the Blueprint category, C++ signature, parameter table, output pins, and edge cases that affect how developers use the node.
