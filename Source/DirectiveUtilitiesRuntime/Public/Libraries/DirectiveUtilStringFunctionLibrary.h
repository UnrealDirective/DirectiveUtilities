// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilTypes.h"
#include "DirectiveUtilStringFunctionLibrary.generated.h"

/**
 * UDirectiveUtilStringFunctionLibrary
 * A collection of helpful string utility functions that improve the usability of strings in Blueprints.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilStringFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Detect if the provided string contains any letters.
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to check.
	 * @returns True if the string contains letters.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String" )
	static bool ContainsLetters(const FString& String);

	/**
	 * Detect if the provided string contains any numbers.
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to check.
	 * @returns True if the string contains numbers.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String" )
	static bool ContainsNumbers(const FString& String);

	/**
	 * Detect if the provided string contains any spaces.
	 * @param String - The string to check.
	 * @returns True if the string contains spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String" )
	static bool ContainsSpaces(const FString& String);

	/**
	 * Detect if the provided string contains any special characters.
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to check.
	 * @returns True if the string contains special characters.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String" )
	static bool ContainsSpecialCharacters(const FString& String);

	/**
	 * Filter out characters types from the string.
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to filter.
	 * @param bLetters - If true, filter out letters.
	 * @param bNumbers - If true, filter out numbers.
	 * @param bSpecialCharacters - If true, filter out special characters.
	 * @param bSpaces - If true, filter out spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String" )
	static FString FilterCharacters(const FString& String, const bool bLetters, const bool bNumbers, const bool bSpecialCharacters, const bool bSpaces);

	/**
	* Sort a string array alphabetically.
	* @param StringArray - The string array to sort.
	*/
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String", meta = (DeprecatedFunction, DeprecationMessage = "Use the engine's Sort String Array node (UE 5.6+) instead."))
	static TArray<FString> SortStringArray(TArray<FString> StringArray);

	/**
	 * Truncates a string to the specified length and appends a suffix.
	 * @param String - The string to truncate.
	 * @param MaxLength - The maximum length of the resulting string including the suffix. If MaxLength is smaller than the suffix length, the full suffix is still returned.
	 * @param Suffix - The suffix to append when truncated.
	 * @returns The truncated string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString TruncateString(const FString& String, int32 MaxLength, const FString& Suffix = TEXT("..."));

	/**
	 * Converts a string to title case, capitalizing the first letter of each word.
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to convert.
	 * @returns The title-cased string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString ToTitleCase(const FString& String);

	/**
	 * Splits a string into its component words. Words are delimited by any non-alphanumeric
	 * character (which is consumed), a lower-to-upper transition ("fooBar" -> "foo", "Bar"),
	 * the end of an acronym ("XMLParser" -> "XML", "Parser"), or a letter/digit transition
	 * ("version2Beta" -> "version", "2", "Beta").
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to split.
	 * @returns The words in order, or an empty array for an empty input.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static TArray<FString> SplitIntoWords(const FString& String);

	/**
	 * Converts a string to camelCase: the first word lowercased, every following word capitalized,
	 * joined without separators ("my var name" -> "myVarName").
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to convert.
	 * @returns The camelCased string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString ToCamelCase(const FString& String);

	/**
	 * Converts a string to PascalCase: every word capitalized, joined without separators
	 * ("my var name" -> "MyVarName").
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to convert.
	 * @returns The PascalCased string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString ToPascalCase(const FString& String);

	/**
	 * Converts a string to snake_case: every word lowercased, joined with underscores
	 * ("my var name" -> "my_var_name").
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to convert.
	 * @returns The snake_cased string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString ToSnakeCase(const FString& String);

	/**
	 * Converts a string to kebab-case: every word lowercased, joined with hyphens
	 * ("my var name" -> "my-var-name").
	 * @note Operates on ASCII characters only; non-ASCII letters/digits are not classified or cased.
	 * @param String - The string to convert.
	 * @returns The kebab-cased string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString ToKebabCase(const FString& String);

	/**
	* Returns a sorted copy of the provided string array.
	* @param StringArray - The array of strings to sort.
	* @returns A sorted copy of the provided string array.
	*/
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|String", meta = (DeprecatedFunction, DeprecationMessage = "Use the engine's Sort String Array node (UE 5.6+) instead."))
	static TArray<FString> GetSortedStringArray(TArray<FString> StringArray);

	/**
	 * Returns the Levenshtein (edit) distance between two strings: the minimum number of single-character
	 * insertions, deletions, or substitutions needed to turn one string into the other.
	 * @param A - The first string.
	 * @param B - The second string.
	 * @param bCaseSensitive - Whether the comparison is case-sensitive. Defaults to true.
	 * @returns The edit distance (0 means identical).
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static int32 GetLevenshteinDistance(const FString& A, const FString& B, bool bCaseSensitive = true);

	/**
	 * Returns a normalized similarity score between two strings, from 0 (completely different) to 1 (identical),
	 * derived from the Levenshtein distance. Two empty strings are considered identical.
	 * @param A - The first string.
	 * @param B - The second string.
	 * @param bCaseSensitive - Whether the comparison is case-sensitive. Defaults to true.
	 * @returns The similarity in the [0, 1] range.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static float GetStringSimilarity(const FString& A, const FString& B, bool bCaseSensitive = true);

	/**
	 * Returns true if the source string contains any of the provided search terms. Empty terms are ignored.
	 * @param Source - The string to search.
	 * @param SearchTerms - The substrings to look for.
	 * @param bCaseSensitive - Whether the search is case-sensitive. Defaults to true.
	 * @returns True if at least one non-empty term is found.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool ContainsAny(const FString& Source, const TArray<FString>& SearchTerms, bool bCaseSensitive = true);

	/**
	 * Finds the earliest occurrence in the source string of any of the provided search terms.
	 * @param Source - The string to search.
	 * @param SearchTerms - The substrings to look for. Empty terms are ignored.
	 * @param bCaseSensitive - Whether the search is case-sensitive. Defaults to true.
	 * @param OutFoundIndex - [out] The index in Source of the earliest match, or INDEX_NONE if none.
	 * @param OutTermIndex - [out] The index into SearchTerms of the matched term, or INDEX_NONE if none.
	 * @returns True if any term was found.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool FindFirstOfAny(const FString& Source, const TArray<FString>& SearchTerms, bool bCaseSensitive, int32& OutFoundIndex, int32& OutTermIndex);

	/**
	 * Encodes a string to Base64.
	 * @param Source - The string to encode.
	 * @returns The Base64-encoded string.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString Base64Encode(const FString& Source);

	/**
	 * Decodes a Base64 string.
	 * @param Source - The Base64 string to decode.
	 * @param OutDecoded - [out] The decoded string, or empty on failure.
	 * @returns True if the input was valid Base64 and was decoded.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool Base64Decode(const FString& Source, FString& OutDecoded);

	/**
	 * Encodes a string as lowercase hex. The string is converted to UTF-8 bytes first.
	 * @param String - The string to encode.
	 * @returns The lowercase hex encoding of the string's UTF-8 bytes.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString HexEncode(const FString& String);

	/**
	 * Decodes a hex string (either case accepted) into the string its bytes spell in UTF-8.
	 * @param Hex - The hex string to decode.
	 * @param OutString - [out] The decoded string, or empty on failure.
	 * @returns True if the input was valid even-length hex and was decoded.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool HexDecode(const FString& Hex, FString& OutString);

	/**
	 * Encodes a byte array as lowercase hex.
	 * @param Bytes - The bytes to encode.
	 * @returns The lowercase hex encoding of the bytes.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString HexEncodeBytes(const TArray<uint8>& Bytes);

	/**
	 * Decodes a hex string (either case accepted) into a byte array.
	 * @param Hex - The hex string to decode.
	 * @param OutBytes - [out] The decoded bytes, or empty on failure.
	 * @returns True if the input was valid even-length hex and was decoded.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool HexDecodeBytes(const FString& Hex, TArray<uint8>& OutBytes);

	/**
	 * Returns the MD5 digest of the string as 32 lowercase hex characters. The string is
	 * converted to UTF-8 bytes first.
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param String - The string to hash.
	 * @returns The MD5 digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString Md5HashString(const FString& String);

	/**
	 * Returns the MD5 digest of a byte array as 32 lowercase hex characters.
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param Bytes - The bytes to hash.
	 * @returns The MD5 digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString Md5HashBytes(const TArray<uint8>& Bytes);

	/**
	 * Returns the SHA-1 digest of the string as 40 lowercase hex characters. The string is
	 * converted to UTF-8 bytes first.
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param String - The string to hash.
	 * @returns The SHA-1 digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString Sha1HashString(const FString& String);

	/**
	 * Returns the SHA-1 digest of a byte array as 40 lowercase hex characters.
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param Bytes - The bytes to hash.
	 * @returns The SHA-1 digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString Sha1HashBytes(const TArray<uint8>& Bytes);

	/**
	 * Returns the CRC32 checksum of the string (seed 0). The string is converted to
	 * UTF-8 bytes first.
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param String - The string to checksum.
	 * @returns The CRC32 checksum.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static int32 Crc32String(const FString& String);

	/**
	 * Returns the CRC32 checksum of a byte array (seed 0).
	 * @note For integrity checks and cache keys, not for security or password storage.
	 * @param Bytes - The bytes to checksum.
	 * @returns The CRC32 checksum.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static int32 Crc32Bytes(const TArray<uint8>& Bytes);

	/**
	 * Checks whether the string is safe to use as a bare file name: not empty, no path
	 * separators or relative segments, and no characters invalid in file names.
	 * @param String - The string to check.
	 * @returns True if the string is a valid bare file name.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static bool IsValidFileName(const FString& String);

	/**
	 * Returns the string with path separators and characters invalid in file names removed
	 * (or replaced when a replacement character is provided). May return an empty string.
	 * @param String - The string to sanitize.
	 * @param Replacement - Optional single-character replacement for stripped characters.
	 * @returns The sanitized file name.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static FString SanitizeFileName(const FString& String, const FString& Replacement = TEXT(""));

	/**
	 * Finds the entry in Candidates most similar to Input (by GetStringSimilarity).
	 * Ties resolve to the earliest index. Cost grows with array size and string
	 * lengths (Levenshtein per candidate). Avoid very large arrays per frame.
	 * @note bCaseSensitive defaults to false (matching user input), unlike the pairwise
	 * comparison functions where it defaults to true.
	 * @param Input - The string to match.
	 * @param Candidates - The candidate strings.
	 * @param OutSimilarity - [out] The winning similarity in [0, 1], 0 when empty.
	 * @param bCaseSensitive - Whether comparison is case-sensitive. Defaults to false.
	 * @returns The index of the best match, or -1 if Candidates is empty.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|String")
	static int32 FindBestStringMatch(const FString& Input, const TArray<FString>& Candidates, float& OutSimilarity, bool bCaseSensitive = false);
};
