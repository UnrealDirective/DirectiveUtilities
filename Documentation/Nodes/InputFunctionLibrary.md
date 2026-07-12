# Input Function Library

> A function library that adds functionality for working with input in Unreal Engine.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilInputFunctionLibrary.h`

---

## Add Input Mapping Contexts
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static EDirectiveUtilSuccessStatus AddInputMappingContexts(
    AController* PlayerController,
    const TArray<FDirectiveUtilEnhancedInputContextData>& Contexts,
    bool bClearPrevious);
```

Apply multiple Input Mapping Contexts to the controller's local player.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller to add the contexts to. Will attempt to get the LocalPlayer from the controller. |
| Contexts | `const TArray<FDirectiveUtilEnhancedInputContextData>&` | The contexts to apply. |
| bClearPrevious | `bool` | Whether to clear all previous contexts before applying the new ones. Clearing only happens once at least one new context has loaded; if every context fails to load, existing mappings are left untouched. |

**Returns:** Returns Success if the contexts were successfully applied.

## Remove Input Mapping Contexts
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static EDirectiveUtilSuccessStatus RemoveInputMappingContexts(
    AController* PlayerController,
    const TArray<TSoftObjectPtr<UInputMappingContext>>& Contexts);
```

Remove multiple Input Mapping Contexts from the controller's local player.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller to remove the contexts from. Will attempt to get the LocalPlayer from the controller. |
| Contexts | `const TArray<TSoftObjectPtr<UInputMappingContext>>&` | The contexts to remove. |

**Returns:** Returns Success if the contexts were successfully removed.

## Swap Input Mapping Contexts
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static EDirectiveUtilSuccessStatus SwapInputMappingContexts(
    AController* PlayerController,
    TSoftObjectPtr<UInputMappingContext> PreviousContext,
    TSoftObjectPtr<UInputMappingContext> NewContext,
    int32 Priority,
    bool bUsePreviousPriority);
```

Swap a designated Input Mapping Context with a new one. If the previous context is found, it is removed and the new context is added; if it is not found, the new context is added at the specified priority.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller to swap the contexts on. Will attempt to get the LocalPlayer from the controller. |
| PreviousContext | `TSoftObjectPtr<UInputMappingContext>` | The context to swap out. |
| NewContext | `TSoftObjectPtr<UInputMappingContext>` | The context to swap in. |
| Priority | `int32` | The priority to set the new context to. |
| bUsePreviousPriority | `bool` | Whether to use the previous context's priority when adding the new context. |

**Returns:** Returns Success if the contexts were successfully swapped.

## Get Enhanced Input Subsystem
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* PlayerController);
```

Returns the Enhanced Input local player subsystem for the given controller, if available.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller; the Local Player's subsystem is retrieved. |

**Returns:** The Enhanced Input subsystem, or null if the controller has no associated local player subsystem.

## Is Input Mapping Context Active
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static bool IsInputMappingContextActive(AController* PlayerController, TSoftObjectPtr<UInputMappingContext> Context);
```

Returns whether the given input mapping context is currently active on the controller.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller to query. |
| Context | `TSoftObjectPtr<UInputMappingContext>` | The input mapping context to check. |

**Returns:** True if the context is currently applied.

## Clear All Input Mapping Contexts
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Input`

```cpp
static EDirectiveUtilSuccessStatus ClearAllInputMappingContexts(AController* PlayerController);
```

Removes all input mapping contexts from the controller.

| Parameter | Type | Description |
|-----------|------|-------------|
| PlayerController | `AController*` | The player controller to clear. |

**Returns:** Success if the contexts were cleared.
