# Function Library

> The primary function library for the Directive Utilities plugin.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilFunctionLibrary.h`

---

## Get Child Classes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static void GetChildClasses(const UClass* BaseClass, bool bRecursive, TArray<UClass*>& DerivedClasses);
```

Returns a list of classes derived from the given base class (not limited to Actors), exposing the built-in GetDerivedClasses function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| BaseClass | `const UClass*` | The base class to get the derived classes for. |
| bRecursive | `bool` | Whether to include derived classes of derived classes. |
| DerivedClasses | `TArray<UClass*>&` | The list of derived classes. |

**Returns:** `DerivedClasses` (out param): the list of derived classes.

## Copy Text To Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void CopyTextToClipboard(const FText& Text);
```

Copies the provided text to the clipboard.

| Parameter | Type | Description |
|-----------|------|-------------|
| Text | `const FText&` | The text to copy to the clipboard. |

## Copy String To Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void CopyStringToClipboard(const FString& String);
```

Copies the provided string to the clipboard.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to copy to the clipboard. |

## Get Text From Clipboard
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static FText GetTextFromClipboard();
```

Gets the content from the clipboard as FText.

**Returns:** The text from the clipboard.

## Get String From Clipboard
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static FString GetStringFromClipboard();
```

Gets the content from the clipboard as an FString.

**Returns:** The content from the clipboard as a string.

## Clear Clipboard
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Clipboard`

```cpp
static void ClearClipboard();
```

Clears the clipboard.

## Get Project Version
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static FString GetProjectVersion();
```

Gets the project version as a string.

**Returns:** The project version as a string.

## Is Running In Editor
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool IsRunningInEditor();
```

Returns true if the game is running in the Unreal Editor. Returns false in packaged/standalone builds.

**Returns:** True if running in the editor.

## Has Command Line Switch
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool HasCommandLineSwitch(const FString& Switch);
```

Checks whether a switch (e.g. `MySwitch` matching `-MySwitch`) was passed on the process command line. Matching is case-insensitive.

| Parameter | Type | Description |
|-----------|------|-------------|
| Switch | `const FString&` | The switch name, without the leading dash. |

**Returns:** True if the switch is present.

## Get Command Line Option
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Utility`

```cpp
static bool GetCommandLineOption(const FString& Key, FString& OutValue);
```

Reads a key=value option (e.g. `MyKey` matching `-MyKey=Value`) from the process command line. Matching is case-insensitive; quoted values are returned without the quotes.

| Parameter | Type | Description |
|-----------|------|-------------|
| Key | `const FString&` | The key name, without the leading dash or trailing equals sign. |
| OutValue | `FString&` | [out] The option's value, or empty if the key is missing. |

**Returns:** True if the key was present.
