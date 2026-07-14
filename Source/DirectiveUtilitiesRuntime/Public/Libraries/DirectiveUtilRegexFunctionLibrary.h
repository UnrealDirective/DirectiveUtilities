// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilRegexFunctionLibrary.generated.h"

/**
 * UDirectiveUtilRegexFunctionLibrary
 * Exposes the engine's regular-expression matching (FRegexPattern/FRegexMatcher) to Blueprints and Python.
 * @warning Complex patterns with nested quantifiers can be extremely slow on long inputs
 * (catastrophic backtracking), and matching runs synchronously on the calling thread.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilRegexFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns true if the pattern matches anywhere within the input string.
	 * @param Input - The string to search.
	 * @param Pattern - The regular expression pattern.
	 * @param bCaseSensitive - Whether matching is case-sensitive. Defaults to true.
	 * @returns True if at least one match was found.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Regex")
	static bool RegexMatches(const FString& Input, const FString& Pattern, bool bCaseSensitive = true);

	/**
	 * Finds the first match of the pattern within the input string.
	 * @param Input - The string to search.
	 * @param Pattern - The regular expression pattern.
	 * @param OutMatch - [out] The matched substring, or empty if no match.
	 * @param OutMatchStart - [out] The start index of the match, or INDEX_NONE if no match.
	 * @param OutMatchEnd - [out] The index just past the end of the match, or INDEX_NONE if no match.
	 * @param bCaseSensitive - Whether matching is case-sensitive. Defaults to true.
	 * @returns True if a match was found.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Regex")
	static bool RegexFindFirst(const FString& Input, const FString& Pattern, FString& OutMatch, int32& OutMatchStart, int32& OutMatchEnd, bool bCaseSensitive = true);

	/**
	 * Returns every (non-overlapping) match of the pattern within the input string.
	 * @note Zero-width (empty) matches are ignored.
	 * @param Input - The string to search.
	 * @param Pattern - The regular expression pattern.
	 * @param bCaseSensitive - Whether matching is case-sensitive. Defaults to true.
	 * @returns The matched substrings, in order.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Regex")
	static TArray<FString> RegexFindAll(const FString& Input, const FString& Pattern, bool bCaseSensitive = true);

	/**
	 * Replaces every match of the pattern in the input string with a literal replacement.
	 * @note The replacement is literal text; capture-group back-references (e.g. $1) are not expanded.
	 * @note Zero-width (empty) matches are ignored.
	 * @param Input - The string to operate on.
	 * @param Pattern - The regular expression pattern.
	 * @param Replacement - The literal text to substitute for each match.
	 * @param bCaseSensitive - Whether matching is case-sensitive. Defaults to true.
	 * @returns The string with all matches replaced.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Regex")
	static FString RegexReplaceAll(const FString& Input, const FString& Pattern, const FString& Replacement, bool bCaseSensitive = true);

	/**
	 * Returns a specific capture group from the first match of the pattern.
	 * @param Input - The string to search.
	 * @param Pattern - The regular expression pattern.
	 * @param GroupIndex - The capture group to retrieve. 0 is the entire match; 1+ are the parenthesized groups.
	 * @param OutGroup - [out] The captured substring, or empty if the group did not participate in the match.
	 * @param bCaseSensitive - Whether matching is case-sensitive. Defaults to true.
	 * @returns True if a match was found and the requested group participated in it.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Regex")
	static bool RegexGetCaptureGroup(const FString& Input, const FString& Pattern, int32 GroupIndex, FString& OutGroup, bool bCaseSensitive = true);
};
