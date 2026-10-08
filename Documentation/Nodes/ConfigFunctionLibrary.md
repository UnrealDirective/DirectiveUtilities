# Config Function Library

> Configuration file (.ini) utilities: reading values with defaults, writing and removing keys, clearing sections, and listing sections and keys. The engine keeps GConfig behind C++.

All path parameters accept absolute paths, or paths relative to the project `Saved` directory. This keeps relative writes valid in packaged builds, where the app bundle is read-only. Every read loads the file fresh from disk, so external edits are visible immediately; every write flushes to disk before returning, so a write followed by an external read sees the new value. Missing files behave as empty configuration: reads return their default, writes create the file.

Section and key matching is case-insensitive, matching engine config behavior. Key names are stored as `FName`. Editor builds keep the casing you pass, but packaged builds use the casing of the first time that name was created anywhere in the process. A key written as `volume` can therefore appear in the file as `Volume` in a packaged game. Lookups still match either spelling; only the file text and the names returned by Get Config Keys In Section differ.

Reads return the stored text exactly as written. Engine macros such as `%GAME%` and `%GAMEDIR%` are not expanded. When a key holds several entries, such as one written by Write Config String Array, the scalar reads return the last entry. A malformed or unreadable file reads as missing: reads return their default, and the query functions return false.

A write regenerates the whole file from the parsed model and atomically replaces the destination. The file is always written as UTF-8 without a byte-order mark, so rewriting a UTF-16 file changes its encoding. Comments and blank lines in a hand-authored `.ini` are not preserved. Empty sections are kept, including hand-authored ones. Clearing the last section leaves an empty file rather than deleting it.

A string value is written in double quotes, with `\n`, `\r`, `\t`, `\"`, `\'`, and `\\` escapes, when it starts or ends with whitespace, ends with a backslash, or contains a double quote, a brace, `//`, or a line break. Every string therefore reads back unchanged, including values with leading or trailing tabs.

A malformed or unreadable existing file is refused and left unchanged. This includes files with a line that the engine parser would join with the next line, such as a line ending in `\` or an unquoted `{` with no `}` on the same line. It also includes values with an unquoted `{` or `}`, which the engine parser drops, an indented line that starts with `;` and holds a key, and section headers with leading whitespace.

File paths cannot be empty or contain embedded null characters. Section and key names cannot be empty, exceed Unreal's `FName` limit, have surrounding whitespace, or contain line breaks, `{`, `}`, `"`, or `//`. Section names also cannot contain `[` or `]`. Key names also cannot contain `=` or start with `;`, `~`, `[`, or one of the engine config command characters `+`, `-`, `.`, `!`, `@`, `*`, and `^`. Invalid inputs fail without changing the file.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilConfigFunctionLibrary.h`

---

## Read Config String / Int / Int 64 / Float / Bool

```cpp
static FString ReadConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& DefaultValue);
static int32 ReadConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 DefaultValue);
static int64 ReadConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 DefaultValue);
static float ReadConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float DefaultValue);
static bool ReadConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool DefaultValue);
```
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

Each returns the stored value, or `DefaultValue` when the file, section, or key is missing. A key that is present but holds text the type cannot parse reads as the engine's parse result for that text (`FCString::Atoi`, `Atoi64`, `Atof`, or `ToBool`), not as `DefaultValue`.

## Read Config String Array
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool ReadConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, TArray<FString>& OutValues);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| OutValues | `TArray<FString>&` | [out] The stored entries, or an empty array when missing. |

**Returns:** True when the key was found.

## Write Config String / Int / Int 64 / Float / Bool / String Array

```cpp
static bool WriteConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& Value);
static bool WriteConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 Value);
static bool WriteConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 Value);
static bool WriteConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float Value);
static bool WriteConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool Value);
static bool WriteConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, const TArray<FString>& Values);
```
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

Writes one value and atomically replaces the file on disk. Missing sections and parent directories are created. A scalar replaces every previous entry for the key. Non-finite float values and string values containing embedded null characters are rejected. A string array replaces every entry for the key. An empty array removes the key and leaves the section in place; when the section was missing, it is created empty.

**Returns:** True when the file was written to disk. Empty file, section, or key arguments fail without writing.

## Vector and color values

```cpp
static FVector2D ReadConfigVector2D(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FVector2D& DefaultValue);
static FVector ReadConfigVector(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FVector& DefaultValue);
static FRotator ReadConfigRotator(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FRotator& DefaultValue);
static FColor ReadConfigColor(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FColor& DefaultValue);
static bool WriteConfigVector2D(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FVector2D& Value);
static bool WriteConfigVector(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FVector& Value);
static bool WriteConfigRotator(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FRotator& Value);
static bool WriteConfigColor(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FColor& Value);
```

Matching write nodes store Unreal's standard text representation. Non-finite vectors and rotators are rejected. Reads return the supplied default when a required component is missing or a component is not a finite number. Color components must be whole numbers from 0 to 255. A missing alpha defaults to 255; an invalid alpha returns the supplied default.

## Has Config Key

```cpp
static bool HasConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName);
```

Returns true when the key exists. Matching ignores section and key case.

## Remove Config Key
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool RemoveConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName);
```

Removes one key and flushes. Removing the last key of a section leaves the empty section in the file, so Has Config Section still returns true for it.

**Returns:** True when the key existed and was removed.

## Clear Config Section
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool ClearConfigSection(const FString& FilePath, const FString& SectionName);
```

Removes every key of one section and drops the empty section itself. Clearing the last remaining section leaves an empty file on disk rather than deleting it.

**Returns:** True when the section existed and was cleared.

## Has Config Section
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool HasConfigSection(const FString& FilePath, const FString& SectionName);
```

**Returns:** True when the section exists in the file.

## Get Config Section Names
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool GetConfigSectionNames(const FString& FilePath, TArray<FString>& OutSectionNames);
```

**Returns:** True when the file exists and parses; receives the section names without brackets, sorted case-insensitively with a case-sensitive tie-break.

## Get Config Keys In Section
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Config`

```cpp
static bool GetConfigKeysInSection(const FString& FilePath, const FString& SectionName, TArray<FString>& OutKeyNames);
```

**Returns:** True when the file parses and the section exists; receives each distinct key name once, sorted case-insensitively with a case-sensitive tie-break.
