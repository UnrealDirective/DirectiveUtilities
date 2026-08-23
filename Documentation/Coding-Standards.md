# Coding standards

This document defines the requirements for code contributed to Directive Utilities. It applies to C++, Unreal Build Tool files, the plugin descriptor, tests, scripts, configuration, and developer documentation.

The words "must", "must not", "required", and "prohibited" are review requirements. A pull request cannot merge while a requirement is unmet. "Prefer" identifies the normal choice. Use another choice only when the change explains and tests the reason. "May" states permission.

## Authority and precedence

Apply these sources in this order:

1. UnrealHeaderTool, Unreal Build Tool, the supported compilers, and packaged runtime behavior.
2. This document.
3. Epic's [C++ coding standard](https://dev.epicgames.com/documentation/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine).
4. The surrounding Directive Utilities source when neither source above specifies a rule.

Epic's [module guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules), [plugin guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine), and [automation test framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine) define the engine behavior behind the project rules below.

Existing code can predate this standard. It does not establish an exception. When a required change touches nonconforming code, bring the affected lines into compliance. Keep unrelated formatting changes out of the pull request.

The supported engine versions and platforms are defined in [Compatibility](Compatibility.md). A change must build and behave correctly throughout that matrix. Do not use the newest engine as the sole reference for an API that ships on older supported versions.

## Module ownership

Choose the module before writing the type. Unreal's module type controls where the code can load, so moving an include does not make editor code safe for a packaged game.

| Module | Owns | Must not own |
|--------|------|--------------|
| `DirectiveUtilitiesRuntime` | Code that can load in a packaged game, including runtime libraries, shared runtime types, and async actions | Editor headers, editor APIs, asset editing, or code guarded only by `WITH_EDITOR` |
| `DirectiveUtilitiesBlueprintNodes` | `UK2Node` classes, wildcard pin handling, node reconstruction, validation, and migration that the Blueprint compiler needs | Runtime behavior or data required after cooking |
| `DirectiveUtilitiesEditor` | Editor subsystems, editor libraries, asset tools, notifications, transactions, and slow tasks | Any type or function required by a game target |
| `DirectiveUtilitiesTests` | Automation tests, fixtures, and test-only reflected types | Production behavior or APIs used by another module |

The dependency direction is fixed:

```text
DirectiveUtilitiesTests -> DirectiveUtilitiesEditor
DirectiveUtilitiesTests -> DirectiveUtilitiesBlueprintNodes
DirectiveUtilitiesTests -> DirectiveUtilitiesRuntime
DirectiveUtilitiesEditor -> DirectiveUtilitiesRuntime
DirectiveUtilitiesBlueprintNodes -> DirectiveUtilitiesRuntime
```

`DirectiveUtilitiesRuntime` must not depend on the other plugin modules. `DirectiveUtilitiesEditor` and `DirectiveUtilitiesBlueprintNodes` must not depend on `DirectiveUtilitiesTests`.

Use a public module dependency only when a public header exposes a type from that module. Use a private dependency when only implementation files need it. Production modules must not include another module's `Private` directory. The test module may reach private code only when public behavior cannot exercise the implementation and the test module declares the include path explicitly.

Keep the module types in `DirectiveUtilities.uplugin` consistent with their ownership:

- `DirectiveUtilitiesRuntime` is `Runtime`.
- `DirectiveUtilitiesBlueprintNodes` is `UncookedOnly`.
- `DirectiveUtilitiesEditor` is `Editor` and allows only editor targets.
- `DirectiveUtilitiesTests` is `DeveloperTool` and allows only editor targets.

When adding an engine plugin dependency, declare it in `DirectiveUtilities.uplugin` with the narrowest target allow list. When adding an engine module dependency, declare it in the owning `.Build.cs` file. Do not use `AdditionalDependencies` in the descriptor as a substitute for `.Build.cs`.

## Source layout

Each module follows Unreal's `Public` and `Private` layout.

- Put consumer-facing declarations in `Public/`.
- Put implementations and module-private declarations in `Private/`.
- Mirror subdirectories across `Public/` and `Private/` for a public header and its implementation.
- Put Blueprint function libraries in `Libraries/`, async actions in `Tasks/`, shared reflected data in `Types/`, subsystems in `Subsystems/`, and custom Blueprint nodes in `Nodes/`.
- Keep a header focused on one API or one closely related family. Related async proxy classes may share a header when they use the same behavior and implementation file.
- Do not add a public header for an implementation detail.

New source files must begin with this exact line, with the current copyright year:

```cpp
// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.
```

The notice must be the first bytes in the file. Do not place a byte-order mark, blank line, or generated comment before it.

Source files use UTF-8 without a byte-order mark, LF line endings, and tabs for indentation. A tab displays as four spaces. Spaces may align text after the first non-whitespace character. Do not use spaces for leading indentation in new or modified code, including tests and `.Build.cs` files.

## Headers and includes

Headers must compile from their direct includes. Do not rely on unity builds, the module precompiled header, or an unrelated transitive include.

- Every header uses `#pragma once`.
- A header that declares reflected types includes its `*.generated.h` file last.
- A `.cpp` file includes its matching header first. Add a blank line before all other includes.
- Include plugin headers before engine headers. Sort each group by path.
- Include the narrowest engine header that defines the required symbol.
- Prefer a forward declaration when the compiler does not need a complete type.
- Never include another module's `Private` header from production code.
- Never put a `using` declaration in global scope.

Use module-relative include paths:

```cpp
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Misc/AutomationTest.h"
```

Do not use relative traversal such as `../`, an absolute path, or a path that starts with `Source/`.

## Naming

Follow Epic's Unreal prefixes and use `DirectiveUtil` as the project stem for public plugin types.

| Symbol | Form | Example |
|--------|------|---------|
| Runtime or editor UObject type | `UDirectiveUtil<Name>` | `UDirectiveUtilEditorSlowTask` |
| Blueprint function library | `UDirectiveUtil<Area>FunctionLibrary` | `UDirectiveUtilStringFunctionLibrary` |
| Async action | `UDirectiveUtilTask_<Name>` | `UDirectiveUtilTask_AsyncTrace` |
| Subsystem | `UDirectiveUtil<Name>Subsystem` | `UDirectiveUtilEditorActorSubsystem` |
| Custom Blueprint node | `UK2Node_DirectiveUtil<Name>` | `UK2Node_DirectiveUtilMapAppend` |
| Struct | `FDirectiveUtil<Name>` | `FDirectiveUtilAssetAuditReport` |
| Enum | `EDirectiveUtil<Name>` | `EDirectiveUtilEaseType` |
| Delegate type | `FOn<Name>` | `FOnAsyncTraceCompleted` |
| Automation fixture | `FDirectiveUtil<Subject>Test` | `FDirectiveUtilStringFunctionLibraryTest` |

Apply these rules to members and functions:

- Use PascalCase for functions, variables, and parameters.
- Prefix every Boolean with `b`, including local variables and parameters.
- Prefix new output parameters with `Out`. Use `InOut` only when the caller supplies a value that the function can replace. Do not rename an existing reflected pin only to correct its prefix.
- Name a function with a verb that states its effect. Use `Get` for a value returned without changing externally visible state, `Set` for replacement, `Add` for insertion without replacement, `Remove` for deletion, and `Is`, `Has`, or `Can` for Boolean queries.
- Keep Unreal's required `Array_` and `Map_` prefixes on wildcard custom-thunk declarations.
- Name a file after its primary type without the Unreal type prefix. `UDirectiveUtilStringFunctionLibrary` belongs in `DirectiveUtilStringFunctionLibrary.h` and `.cpp`.
- Use the owning module's export macro on every public class or non-inline public symbol: `DIRECTIVEUTILITIESRUNTIME_API`, `DIRECTIVEUTILITIESBLUEPRINTNODES_API`, or `DIRECTIVEUTILITIESEDITOR_API`.
- Use `LogDirectiveUtil` for runtime logging and `LogDirectiveUtilEditor` for editor logging. Do not add `LogTemp` calls.

Do not create a synonym for an established term. The plugin uses "save slot", "mapping context", "async action", "runtime module", and "editor module" consistently in code and documentation.

## C++ rules

Unreal Engine uses C++20, but code must remain portable across every compiler in the support matrix.

- Use Unreal integer types such as `int32` and `uint64` when width matters. Width always matters for serialization, reflection, file formats, networking, and arithmetic bounds.
- Use `nullptr`. Do not use `NULL` or `0` as a null pointer.
- Use `enum class` instead of an unscoped enum.
- Add `override` to every override. Add `final` only when further overrides would violate the type's contract.
- Mark methods `const` when they do not change object state. Pass nontrivial read-only inputs by `const` reference.
- Avoid `auto` unless the type cannot be written, such as a lambda, or Epic's coding standard permits the specific use.
- Do not use structured bindings.
- Use range-based `for` loops when the index or iterator is not part of the operation.
- Use `Cast`, `CastChecked`, and `CastField` for Unreal reflected types. Use `static_cast` for an intentional native conversion. Do not use C-style casts.
- Use Unreal containers and strings in plugin APIs. A public reflected API must not expose a standard-library type.
- Use `MoveTemp` only when the source is not read afterward.
- Keep platform-specific APIs out of public headers.

Put every opening brace on a new line. Always use braces, including for a one-line conditional or loop.

```cpp
if (!World)
{
	return false;
}
```

Each `switch` case must end with `break`, `return`, or another explicit control transfer. Mark intentional fallthrough with a one-line comment. Every `switch` must have a `default` case, even when all current enum values are listed.

Use an anonymous namespace for helpers that exist only in one `.cpp` file. Reflected types cannot live in a namespace. Do not add mutable global state. If state must outlive a call, give it a clear owner and define its synchronization and shutdown behavior.

## UObject ownership and lifetime

Unreal's garbage collector does not discover a raw pointer unless reflection or another engine-owned reference keeps the object alive.

- Store an owned UObject member as `TObjectPtr<T>` and mark it with `UPROPERTY()` when garbage collection must retain it.
- Store a non-owning UObject reference as `TWeakObjectPtr<T>` when the object can disappear independently.
- Use `TSoftObjectPtr<T>` or `TSoftClassPtr<T>` when loading must remain deferred.
- Use a raw UObject pointer for a transient parameter, return value, or local value whose lifetime is already protected.
- Do not use `TSharedPtr` or `TUniquePtr` to own a UObject.
- Remove delegate bindings and timers before their owner becomes unreachable.
- Do not capture a raw UObject pointer in deferred work unless the lifetime is guaranteed. Prefer a weak pointer and validate it when the work runs.

An async action must register with its game instance before it starts deferred work. It must release timers, streamable handles, path-following requests, and delegate handles on completion, cancellation, world cleanup, and destruction. Completion and failure delegates must fire at most once. After calling `SetReadyToDestroy`, the action must not schedule or broadcast more work.

## Public API design

Treat every public header and reflected symbol as a compatibility commitment. Blueprint assets store function names, pin names, property names, enum ordinals, and some metadata.

Before adding a public function, establish all of the following:

- The correct module.
- Whether the operation is runtime-safe or editor-only.
- Whether the operation mutates state, performs I/O, schedules work, or can be pure.
- The result on empty, invalid, null, non-finite, and out-of-range input.
- The order of returned values when more than one result is possible.
- The upper bound on work and allocation.
- The thread on which callers may use it.
- The compatibility effect of every parameter, pin, and enum value.

Do not add a wrapper that only renames an engine function. A new node must close a documented usability, safety, performance, or capability gap.

Prefer one purpose per function. When a function needs many related options, define a named parameter struct instead of adding a sequence of Boolean flags. A Boolean is appropriate when the two states are obvious at the call site and are unlikely to grow into more modes.

Return a value directly when failure is impossible or the type has an unambiguous empty result. Return `bool` with output parameters when callers need to distinguish failure from an empty value. Use `EDirectiveUtilSuccessStatus` with `ExpandEnumAsExecs` when a Blueprint execution branch is part of the node's contract.

Inputs come before outputs. Required inputs come before optional inputs. New optional parameters go at the end so existing C++ calls keep their meaning. Give Blueprint inputs useful defaults when one choice is a safe common case.

## Reflection and Blueprint nodes

Use the canonical category format with no spaces around the separators:

```cpp
Category = "Directive Utilities|String"
```

Every new or changed `UFUNCTION` category must equal `Directive Utilities` or begin with `Directive Utilities|`. Choose an existing area before creating another. A nested category uses another pipe, such as `Directive Utilities|Math|Random`.

Choose the exposure specifier by behavior:

- Use `BlueprintPure` only when the result depends on inputs or read-only state and the call has no externally visible side effect. Keep pure calls bounded enough that Blueprint reevaluation is acceptable.
- Use `BlueprintCallable` when the function mutates state, performs I/O, changes editor data, starts async work, or has a cost that graph authors must schedule explicitly.
- Use an async proxy for work that completes after the current Blueprint execution path.
- Do not expose an internal helper to Blueprint to avoid writing a native call.

Metadata is part of the API. Apply it only when its behavior is tested.

- `WorldContext` names the exact world-context parameter.
- `BlueprintInternalUseOnly` hides factory functions that an async proxy or custom node owns.
- `AutoCreateRefTerm` is required when a reference input must accept an unconnected literal.
- `AdvancedDisplay` may hide optional tuning inputs. It must not hide a value required for correct results.
- `ExpandEnumAsExecs` requires an enum output that is assigned on every return path.
- `BlueprintThreadSafe` is allowed only when the entire implementation and every function it calls can run off the game thread. Reading a UObject, subsystem, global registry, world, or unsynchronized mutable state disqualifies the function.

For reflected data:

- Declare Blueprint enums as `UENUM(BlueprintType)` and `enum class ... : uint8`.
- Add enum values only at the end. Do not reorder existing values or insert a value between them.
- Use `UMETA(DisplayName = "...")` when the C++ name is not the correct editor label.
- Use `UMETA(Tooltip = "...")` when two values need more than their labels to distinguish them.
- Declare Blueprint structs as `USTRUCT(BlueprintType)`.
- Give every reflected property an explicit default.
- Keep shared structs data-oriented. Put operations in the owning library unless the behavior is intrinsic to the type.

### Wildcard containers and custom nodes

Wildcard arrays and maps require more than a templated C++ helper. Keep their responsibilities separate:

- The public `CustomThunk` declaration defines the reflected pins.
- The `DECLARE_FUNCTION` thunk reads and validates VM properties.
- A `GenericArray_` or `GenericMap_` function implements property-aware behavior.
- A `UK2Node_DirectiveUtil*` class handles wildcard pin conformance only when metadata cannot express the node.
- Runtime behavior remains in `DirectiveUtilitiesRuntime`. The uncooked node module must not be required after Blueprint compilation.

The typed body of a `CustomThunk` stub is unreachable by design and must use `checkNoEntry()`. Caller-controlled input must never reach `check`, `checkf`, or `ensure`. Validate property classes, element types, addresses, aliasing, and output compatibility before reading or writing VM memory.

A wildcard change requires tests for pin reconstruction, mismatched types, disconnected pins, compiled Blueprint VM execution, and each supported property family. Include Boolean, numeric, string, object-reference, and reflected-struct cases when the operation claims general container support.

## Failure handling and logging

Caller input is not an invariant. Validate it and return a documented failure result.

- Initialize every output before the first possible failure return.
- Do not expose partial output unless the function contract names it as partial.
- Use `check` only for an internal condition that cannot fail in a valid build.
- Use `ensure` only when execution can continue after an internal programming error and one diagnostic is useful.
- Do not use either macro to reject a null Blueprint input, an invalid path, an empty array, a bad range, or another caller-controlled value.
- Do not crash, assert, or emit repeated warnings for an expected failure mode.

Log through the module category. A warning must tell the developer what operation failed and why. Use `Verbose` for normal diagnostic progress. Do not log every element in a collection or every tick. Do not include secrets, access tokens, or full user-specific paths in logs.

Return values and delegates remain the primary error channel for Blueprint APIs. A log line does not replace a failure pin, status, or return value.

## Bounds, arithmetic, and performance

Reject unsafe work before allocation or iteration.

- Compute collection sizes in a type wide enough to detect overflow.
- Check multiplication and addition before narrowing to `int32`.
- Reject non-finite floating-point inputs before using them in a loop bound, index, duration, transform, or allocation.
- A public request that constructs a new collection must not produce more than 1,000,000 elements. Spatial generators use `UDirectiveUtilMathFunctionLibrary::MaximumGeneratedElementCount`. Wildcard sampling enforces the same limit in its generic implementation.
- The generation limit does not authorize copying an arbitrary caller-owned million-element collection when the operation has worse than linear cost.
- Preserve input order unless the function name and documentation state another order.
- Define tie behavior for sorting, selection, nearest-value, and most-common operations.
- Avoid hidden asset loads and repeated registry scans in query functions.

Use Unreal's property API for reflected values. A byte copy is allowed only when the property type is proven safe for bulk relocation or copying. Strings, text, object references, and structs with managed members require property-aware construction, copying, and destruction.

A performance claim requires a same-machine baseline and candidate run from [runtime performance tests](../Tests/Performance/README.md). Correctness must pass before timing. Do not trade reflected-type support, deterministic results, or failure safety for a faster benchmark.

## Platform and engine-version code

Keep version and platform differences in the smallest possible implementation block.

- Use engine version macros around the API call that differs. Do not duplicate the public function.
- Keep public signatures identical across supported versions.
- Put platform code behind an engine abstraction when Unreal provides one.
- If no abstraction exists, isolate the branch in a named private helper.
- Add a test for each branch that can run in the support matrix.
- Document a user-visible version difference on the matching node page and in [Compatibility](Compatibility.md).

Do not lower the supported matrix in a feature pull request. A support-policy change requires its own compatibility review, descriptor updates, release-gate updates, and migration notes where applicable.

## Comments and API documentation

Code must explain the implementation through names and structure. Add an inline comment only for a constraint, invariant, engine defect, or compatibility choice that the code cannot express. Do not narrate the next line, record change history, or add section-heading comments.

Every public reflected class, function, delegate, enum, struct, and property requires an Unreal documentation comment. The first sentence states the contract in editor-facing language. Do not repeat the symbol name as the description.

A function comment must document:

- Every parameter, using `@param Name Description.`
- Every output, including its failure value.
- A return value when the purpose sentence does not already define it, using `@return`.
- Units, valid ranges, ordering, limits, ownership, thread restrictions, and failure behavior when they affect use.
- Version-specific behavior with `@note`.
- Deprecation with `@deprecated` and the replacement symbol.

Example:

```cpp
/**
 * Decodes a Base64 string.
 *
 * @param Source The Base64 input.
 * @param OutDecoded Receives the decoded text, or an empty string on failure.
 * @return `true` when `Source` is valid Base64; otherwise, `false`.
 */
UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
static bool Base64Decode(const FString& Source, FString& OutDecoded);
```

## Compatibility and deprecation

Do not rename or remove a public reflected symbol in a minor or patch release. Do not reorder parameters, rename pins, change defaults, change enum ordinals, or change a return type without treating the change as a compatibility break.

When replacing a Blueprint function:

1. Keep the old signature compiling.
2. Add `DeprecatedFunction`.
3. Add a `DeprecationMessage` that names the replacement and any required action.
4. Route the old implementation through the supported path when behavior can remain correct.
5. Add a changelog entry and update the node reference.
6. Remove the old function only in a planned major release with a migration document.

When a reflected rename is unavoidable, add and test the required Core Redirects. Verify an asset saved with the old symbol in every supported engine version. Source compatibility for C++ consumers still requires a migration note because Core Redirects do not rewrite C++.

## Tests

Production behavior and its tests belong in the same pull request. A fix requires a test that fails before the fix unless reproducing the failure is impossible in automation. If automation is impossible, the pull request must record the exact manual reproduction and result.

Place plugin automation tests under `Source/DirectiveUtilitiesTests/Private/Tests/`. Use a fixture name beginning with `FDirectiveUtil` and a test path under `DirectiveUtilities.`. Performance tests use the `Performance.DirectiveUtilities.` root.

Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` for one fixed case set. Use a complex test or latent commands when the test must enumerate cases, wait for engine work, or cross frames. Normal behavior tests use `EditorContext | ClientContext | EngineFilter` when they are valid in both hosts. Editor-only tests omit `ClientContext`. Performance tests use `PerfFilter`.

Each test must be:

- Independent of execution order.
- Safe to run more than once.
- Independent of files, assets, editor selection, and world state left by another test.
- Responsible for deleting files, objects, delegates, timers, and settings that it creates.
- Deterministic. Seed random input and report the seed in a failing case.
- Specific about expected results. Do not assert only that a call did not crash.

Cover valid behavior and the contract boundary. Include empty input, the smallest valid value, the largest accepted value, one rejected value on each relevant side, null objects, aliasing between input and output, overflow paths, and round trips when the API supports them.

Use an independent reference implementation for algorithms where practical. Do not copy the production loop into the test and call the duplicate an oracle.

Update the API boundary tests when the reflected API changes:

- `DirectiveUtilRuntimeSurfaceTest` lists every reflected runtime class and verifies that packaged targets do not load editor or uncooked modules.
- `DirectiveUtilBlueprintCategoryTest` verifies the category root for reflected runtime and editor functions.
- Wildcard pin and VM tests verify container metadata and compiled graph behavior.

Do not weaken a census, warning check, timeout, or expected module boundary to make a change pass. Update an expected count only after accounting for every added or removed test.

## Documentation and changelog

Every Blueprint-exposed API must have a matching page under `Documentation/Nodes/`. Update that page in the same pull request as the API.

The node page must state the module, public header, Blueprint category, C++ signature, parameter behavior, output behavior, failure behavior, limits, and version differences. Add a new page to `Documentation/README.md`. Add a new library or feature area to the root `README.md` when users need it to discover the API.

Write documentation in present tense with direct, literal sentences. Use the exact symbol, path, command, unit, and limit. Do not use promotional claims. Do not describe a change as new outside the changelog or a versioned migration page.

Add each user-visible change under `[Unreleased]` in `CHANGELOG.md`. Use the existing `Added`, `Changed`, `Fixed`, and `Deprecated` sections. Start each bullet with its action. A breaking change also requires a migration page and a semantic major version.

## Verification

Run the narrowest relevant checks while developing. Before merge, run every required check for the changed code or API.

| Change | Required verification |
|--------|-----------------------|
| Documentation only | `git diff --check`, link and path review, and confirmation that the release-text scan in `Tools/Release/run-local-release-gate.sh` finds no match |
| Python packaging or release tools | The matching tests under `Tests/Packaging/` or `Tests/Release/`, plus `python3 Tools/Packaging/package_fab.py --check` when package contents can change |
| Runtime, Blueprint node, editor, or test C++ | The runtime host in the editor and packaged Development for every supported engine version |
| Runtime performance path | Correctness tests, then baseline and candidate runs through `Tests/Performance/run-runtime-benchmarks.*` |
| Async action | Editor and packaged tests for success, failure, cancellation where supported, duplicate completion, and world cleanup |
| Wildcard node | Pin tests, node reconstruction tests, and compiled Blueprint VM tests |
| Release candidate | The complete local release gate on a clean working tree |

Use the checked-in runners rather than assembling an equivalent command by hand:

```sh
Tests/RuntimeHost/Scripts/run-unix.sh "/path/to/UE_5.8" Development
Tools/Release/run-local-release-gate.sh "/path/to/UE_5.6" "/path/to/UE_5.7" "/path/to/UE_5.8"
```

Windows provides the matching `.ps1` runners. A successful compile on one host is not proof of packaged behavior or support for the other engine versions. A release is not verified until the local release gate records evidence for the exact clean commit.

## Commits and pull requests

Commit subjects use the repository's Conventional Commit form:

```text
feat(runtime): add keyed stopwatches
fix(editor): clamp selection bounds
docs: define contribution standards
```

Use a terse, single-line subject. Add a body only when the reason or compatibility effect is not clear from the diff. Do not add attribution trailers.

Open pull requests against `dev`. State the affected modules, public API changes, Blueprint asset compatibility, engine-version branches, and verification commands with their results. Keep generated build output, local reports, editor state, and IDE files out of the commit.
