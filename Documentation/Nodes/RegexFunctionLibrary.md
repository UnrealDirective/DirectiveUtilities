# Regex Function Library

> Exposes the engine's regular-expression matching (FRegexPattern/FRegexMatcher) to Blueprints and Python.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilRegexFunctionLibrary.h`

---

## Regex Matches
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Regex`

```cpp
static bool RegexMatches(const FString& Input, const FString& Pattern, bool bCaseSensitive = true);
```

Returns true if the pattern matches anywhere within the input string.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to search. |
| Pattern | `const FString&` | The regular expression pattern. |
| bCaseSensitive | `bool` | Whether matching is case-sensitive. Defaults to true. |

**Returns:** True if at least one match was found.

## Regex Find First
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Regex`

```cpp
static bool RegexFindFirst(const FString& Input, const FString& Pattern, FString& OutMatch, int32& OutMatchStart, int32& OutMatchEnd, bool bCaseSensitive = true);
```

Finds the first match of the pattern within the input string.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to search. |
| Pattern | `const FString&` | The regular expression pattern. |
| OutMatch | `FString&` | [out] The matched substring, or empty if no match. |
| OutMatchStart | `int32&` | [out] The start index of the match, or INDEX_NONE if no match. |
| OutMatchEnd | `int32&` | [out] The index just past the end of the match, or INDEX_NONE if no match. |
| bCaseSensitive | `bool` | Whether matching is case-sensitive. Defaults to true. |

**Returns:** True if a match was found. Out params provide the matched substring and its start/end indices.

## Regex Find All
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Regex`

```cpp
static TArray<FString> RegexFindAll(const FString& Input, const FString& Pattern, bool bCaseSensitive = true);
```

Returns every (non-overlapping) match of the pattern within the input string. Zero-width (empty) matches are ignored.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to search. |
| Pattern | `const FString&` | The regular expression pattern. |
| bCaseSensitive | `bool` | Whether matching is case-sensitive. Defaults to true. |

**Returns:** The matched substrings, in order.

## Regex Replace All
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Regex`

```cpp
static FString RegexReplaceAll(const FString& Input, const FString& Pattern, const FString& Replacement, bool bCaseSensitive = true);
```

Replaces every match of the pattern in the input string with a literal replacement. The replacement is literal text; capture-group back-references (e.g. $1) are not expanded. Zero-width (empty) matches are ignored.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to operate on. |
| Pattern | `const FString&` | The regular expression pattern. |
| Replacement | `const FString&` | The literal text to substitute for each match. |
| bCaseSensitive | `bool` | Whether matching is case-sensitive. Defaults to true. |

**Returns:** The string with all matches replaced.

## Regex Get Capture Group
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Regex`

```cpp
static bool RegexGetCaptureGroup(const FString& Input, const FString& Pattern, int32 GroupIndex, FString& OutGroup, bool bCaseSensitive = true);
```

Returns a specific capture group from the first match of the pattern.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to search. |
| Pattern | `const FString&` | The regular expression pattern. |
| GroupIndex | `int32` | The capture group to retrieve. 0 is the entire match; 1+ are the parenthesized groups. |
| OutGroup | `FString&` | [out] The captured substring, or empty if the group did not participate in the match. |
| bCaseSensitive | `bool` | Whether matching is case-sensitive. Defaults to true. |

**Returns:** True if a match was found and the requested group participated in it.
