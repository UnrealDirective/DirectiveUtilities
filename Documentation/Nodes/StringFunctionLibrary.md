# String Function Library

> A collection of helpful string utility functions that improve the usability of strings in Blueprints.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilStringFunctionLibrary.h`

---

## Contains Letters
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool ContainsLetters(const FString& String);
```

Detects whether the provided string contains any letters.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to check. |

**Returns:** True if the string contains letters.

## Contains Numbers
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool ContainsNumbers(const FString& String);
```

Detects whether the provided string contains any numbers.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to check. |

**Returns:** True if the string contains numbers.

## Contains Spaces
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool ContainsSpaces(const FString& String);
```

Detects whether the provided string contains any spaces.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to check. |

**Returns:** True if the string contains spaces.

## Contains Special Characters
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool ContainsSpecialCharacters(const FString& String);
```

Detects whether the provided string contains any special characters.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to check. |

**Returns:** True if the string contains special characters.

## Filter Characters
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString FilterCharacters(const FString& String, const bool bLetters, const bool bNumbers, const bool bSpecialCharacters, const bool bSpaces);
```

Filters out the selected character types from the string.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to filter. |
| bLetters | `const bool` | If true, filter out letters. |
| bNumbers | `const bool` | If true, filter out numbers. |
| bSpecialCharacters | `const bool` | If true, filter out special characters. |
| bSpaces | `const bool` | If true, filter out spaces. |

**Returns:** The filtered string.

## Sort String Array
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static TArray<FString> SortStringArray(TArray<FString> StringArray);
```

**Deprecated.** Use the engine's Sort String Array node (UE 5.6+) instead. Sorts a string array alphabetically.

| Parameter | Type | Description |
|-----------|------|-------------|
| StringArray | `TArray<FString>` | The string array to sort. |

**Returns:** The sorted string array.

## Truncate String
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString TruncateString(const FString& String, int32 MaxLength, const FString& Suffix = TEXT("..."));
```

Truncates a string to the specified length and appends a suffix.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to truncate. |
| MaxLength | `int32` | The maximum length of the resulting string including the suffix. |
| Suffix | `const FString&` | The suffix to append when truncated. |

**Returns:** The truncated string.

## To Title Case
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString ToTitleCase(const FString& String);
```

Converts a string to title case, capitalizing the first letter of each word.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to convert. |

**Returns:** The title-cased string.

## Split Into Words
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static TArray<FString> SplitIntoWords(const FString& String);
```

Splits a string into its component words. Words are delimited by any non-alphanumeric character (which is consumed), a lower-to-upper transition (`fooBar` -> `foo`, `Bar`), the end of an acronym (`XMLParser` -> `XML`, `Parser`), or a letter/digit transition (`version2Beta` -> `version`, `2`, `Beta`). Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to split. |

**Returns:** The words in order, or an empty array for an empty input.

## To Camel Case
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString ToCamelCase(const FString& String);
```

Converts a string to camelCase: the first word lowercased, every following word capitalized, joined without separators (`"my var name"` -> `myVarName`). Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to convert. |

**Returns:** The camelCased string.

## To Pascal Case
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString ToPascalCase(const FString& String);
```

Converts a string to PascalCase: every word capitalized, joined without separators (`"my var name"` -> `MyVarName`). Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to convert. |

**Returns:** The PascalCased string.

## To Snake Case
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString ToSnakeCase(const FString& String);
```

Converts a string to snake_case: every word lowercased, joined with underscores (`"my var name"` -> `my_var_name`). Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to convert. |

**Returns:** The snake_cased string.

## To Kebab Case
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString ToKebabCase(const FString& String);
```

Converts a string to kebab-case: every word lowercased, joined with hyphens (`"my var name"` -> `my-var-name`). Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to convert. |

**Returns:** The kebab-cased string.

## Get Sorted String Array
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static TArray<FString> GetSortedStringArray(TArray<FString> StringArray);
```

**Deprecated.** Use the engine's Sort String Array node (UE 5.6+) instead. Returns a sorted copy of the provided string array.

| Parameter | Type | Description |
|-----------|------|-------------|
| StringArray | `TArray<FString>` | The array of strings to sort. |

**Returns:** A sorted copy of the provided string array.

## Get Levenshtein Distance
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static int32 GetLevenshteinDistance(const FString& A, const FString& B, bool bCaseSensitive = true);
```

Returns the Levenshtein (edit) distance between two strings: the minimum number of single-character insertions, deletions, or substitutions needed to turn one string into the other.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FString&` | The first string. |
| B | `const FString&` | The second string. |
| bCaseSensitive | `bool` | Whether the comparison is case-sensitive. Defaults to true. |

**Returns:** The edit distance (0 means identical).

## Get String Similarity
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static float GetStringSimilarity(const FString& A, const FString& B, bool bCaseSensitive = true);
```

Returns a normalized similarity score between two strings, from 0 (completely different) to 1 (identical), derived from the Levenshtein distance. Two empty strings are considered identical.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FString&` | The first string. |
| B | `const FString&` | The second string. |
| bCaseSensitive | `bool` | Whether the comparison is case-sensitive. Defaults to true. |

**Returns:** The similarity in the [0, 1] range.

## Contains Any
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool ContainsAny(const FString& Source, const TArray<FString>& SearchTerms, bool bCaseSensitive = true);
```

Returns true if the source string contains any of the provided search terms. Empty terms are ignored.

| Parameter | Type | Description |
|-----------|------|-------------|
| Source | `const FString&` | The string to search. |
| SearchTerms | `const TArray<FString>&` | The substrings to look for. |
| bCaseSensitive | `bool` | Whether the search is case-sensitive. Defaults to true. |

**Returns:** True if at least one non-empty term is found.

## Find First Of Any
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool FindFirstOfAny(const FString& Source, const TArray<FString>& SearchTerms, bool bCaseSensitive, int32& OutFoundIndex, int32& OutTermIndex);
```

Finds the earliest occurrence in the source string of any of the provided search terms.

| Parameter | Type | Description |
|-----------|------|-------------|
| Source | `const FString&` | The string to search. |
| SearchTerms | `const TArray<FString>&` | The substrings to look for. Empty terms are ignored. |
| bCaseSensitive | `bool` | Whether the search is case-sensitive. Defaults to true. |
| OutFoundIndex | `int32&` | [out] The index in Source of the earliest match, or INDEX_NONE if none. |
| OutTermIndex | `int32&` | [out] The index into SearchTerms of the matched term, or INDEX_NONE if none. |

**Returns:** True if any term was found.

## Base64 Encode
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString Base64Encode(const FString& Source);
```

Encodes a string to Base64.

| Parameter | Type | Description |
|-----------|------|-------------|
| Source | `const FString&` | The string to encode. |

**Returns:** The Base64-encoded string.

## Base64 Decode
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool Base64Decode(const FString& Source, FString& OutDecoded);
```

Decodes a Base64 string.

| Parameter | Type | Description |
|-----------|------|-------------|
| Source | `const FString&` | The Base64 string to decode. |
| OutDecoded | `FString&` | [out] The decoded string, or empty on failure. |

**Returns:** True if the input was valid Base64 and was decoded.

## Hex Encode
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString HexEncode(const FString& String);
```

Encodes a string as lowercase hex. The string is converted to UTF-8 bytes first.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to encode. |

**Returns:** The lowercase hex encoding of the string's UTF-8 bytes.

## Hex Decode
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool HexDecode(const FString& Hex, FString& OutString);
```

Decodes a hex string (either case accepted) into the string its bytes spell in UTF-8. Fails on odd-length input or non-hex characters.

| Parameter | Type | Description |
|-----------|------|-------------|
| Hex | `const FString&` | The hex string to decode. |
| OutString | `FString&` | [out] The decoded string, or empty on failure. |

**Returns:** True if the input was valid even-length hex and was decoded.

## Hex Encode Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString HexEncodeBytes(const TArray<uint8>& Bytes);
```

Encodes a byte array as lowercase hex.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `const TArray<uint8>&` | The bytes to encode. |

**Returns:** The lowercase hex encoding of the bytes.

## Hex Decode Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool HexDecodeBytes(const FString& Hex, TArray<uint8>& OutBytes);
```

Decodes a hex string (either case accepted) into a byte array. Fails on odd-length input or non-hex characters.

| Parameter | Type | Description |
|-----------|------|-------------|
| Hex | `const FString&` | The hex string to decode. |
| OutBytes | `TArray<uint8>&` | [out] The decoded bytes, or empty on failure. |

**Returns:** True if the input was valid even-length hex and was decoded.

## Md5 Hash String
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString Md5HashString(const FString& String);
```

Returns the MD5 digest of the string as 32 lowercase hex characters. The string is converted to UTF-8 bytes first. For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to hash. |

**Returns:** The MD5 digest as lowercase hex.

## Md5 Hash Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString Md5HashBytes(const TArray<uint8>& Bytes);
```

Returns the MD5 digest of a byte array as 32 lowercase hex characters. For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `const TArray<uint8>&` | The bytes to hash. |

**Returns:** The MD5 digest as lowercase hex.

## Sha1 Hash String
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString Sha1HashString(const FString& String);
```

Returns the SHA-1 digest of the string as 40 lowercase hex characters. The string is converted to UTF-8 bytes first. For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to hash. |

**Returns:** The SHA-1 digest as lowercase hex.

## Sha1 Hash Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString Sha1HashBytes(const TArray<uint8>& Bytes);
```

Returns the SHA-1 digest of a byte array as 40 lowercase hex characters. For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `const TArray<uint8>&` | The bytes to hash. |

**Returns:** The SHA-1 digest as lowercase hex.

## Crc32 String
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static int32 Crc32String(const FString& String);
```

Returns the CRC32 checksum of the string (seed 0). The string is converted to UTF-8 bytes first. For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to checksum. |

**Returns:** The CRC32 checksum.

## Crc32 Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static int32 Crc32Bytes(const TArray<uint8>& Bytes);
```

Returns the CRC32 checksum of a byte array (seed 0). For integrity checks and cache keys, not for security or password storage.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `const TArray<uint8>&` | The bytes to checksum. |

**Returns:** The CRC32 checksum.

## Is Valid File Name
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static bool IsValidFileName(const FString& String);
```

Checks whether the string is safe to use as a bare file name: not empty, no path separators or relative segments, and no characters invalid in file names.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to check. |

**Returns:** True if the string is a valid bare file name.

## Sanitize File Name
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static FString SanitizeFileName(const FString& String, const FString& Replacement = TEXT(""));
```

Returns the string with path separators and characters invalid in file names removed (or replaced when a replacement character is provided). May return an empty string.

| Parameter | Type | Description |
|-----------|------|-------------|
| String | `const FString&` | The string to sanitize. |
| Replacement | `const FString&` | Optional single-character replacement for stripped characters. |

**Returns:** The sanitized file name.

## Find Best String Match
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|String`

```cpp
static int32 FindBestStringMatch(const FString& Input, const TArray<FString>& Candidates, float& OutSimilarity, bool bCaseSensitive = false);
```

Finds the entry in Candidates most similar to Input (by Get String Similarity). Ties resolve to the earliest index. Cost grows with array size and string lengths, so avoid very large arrays per frame. Note that bCaseSensitive defaults to false (matching user input), unlike the pairwise comparison functions where it defaults to true.

| Parameter | Type | Description |
|-----------|------|-------------|
| Input | `const FString&` | The string to match. |
| Candidates | `const TArray<FString>&` | The candidate strings. |
| OutSimilarity | `float&` | [out] The winning similarity in [0, 1], 0 when empty. |
| bCaseSensitive | `bool` | Whether comparison is case-sensitive. Defaults to false. |

**Returns:** The index of the best match, or -1 if Candidates is empty.
