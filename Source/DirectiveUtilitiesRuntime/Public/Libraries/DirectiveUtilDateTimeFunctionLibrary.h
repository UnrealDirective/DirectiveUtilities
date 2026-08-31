// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilDateTimeFunctionLibrary.generated.h"

/**
 * Date and time utilities that the engine's Blueprint surface lacks: custom pattern formatting
 * and parsing, local-time to UTC conversion, and human-readable duration formatting.
 *
 * A pattern is literal text mixed with these tokens:
 *
 * | Token | Meaning | Example |
 * |-------|---------|---------|
 * | yyyy  | Four-digit year | 2026 |
 * | MM    | Two-digit month | 08 |
 * | dd    | Two-digit day of month | 22 |
 * | HH    | Two-digit hour, 00 to 23 | 09 |
 * | mm    | Two-digit minute | 05 |
 * | ss    | Two-digit second | 03 |
 * | fff   | Three-digit millisecond | 412 |
 *
 * Characters that are not token letters and not wrapped in single quotes are emitted verbatim.
 * Wrap token characters in single quotes to emit them literally, such as `yyyy-MM-dd'T'HH:mm`;
 * two consecutive quotes emit one quote character. Any other run of token letters is an error
 * for both formatting and parsing.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilDateTimeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Formats a date and time with a custom pattern.
	 *
	 * @param DateTime The date and time to format.
	 * @param Pattern The pattern describing the output, using the tokens documented on this library.
	 * @param OutText Receives the formatted text, or an empty string on failure.
	 * @return `true` when the pattern was valid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|DateTime")
	static bool FormatDateTime(const FDateTime& DateTime, const FString& Pattern, FString& OutText);

	/**
	 * Parses a date and time written in a custom pattern. The whole text must match the whole
	 * pattern; extra characters and conflicting repeated fields are errors. Components omitted
	 * from the pattern default to year 1, month 1, day 1, and midnight.
	 *
	 * @param Text The text to parse.
	 * @param Pattern The pattern describing the text, using the tokens documented on this library.
	 * @param OutDateTime Receives the parsed value, or a default value on failure.
	 * @return `true` when the text matched the pattern and held a valid calendar date.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|DateTime")
	static bool TryParseDateTime(const FString& Text, const FString& Pattern, FDateTime& OutDateTime);

	/**
	 * Converts a UTC timestamp to local wall-clock time using the timezone rules in effect at
	 * that instant. Returns the input unchanged when the platform's time routines reject the
	 * value, which covers dates outside the platform's supported range and offsets that would
	 * push the result past the FDateTime range.
	 *
	 * @param UtcDateTime The timestamp treated as UTC.
	 * @return The equivalent local time.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|DateTime")
	static FDateTime ToLocalTime(const FDateTime& UtcDateTime);

	/**
	 * Converts local wall-clock time to a UTC timestamp using the timezone rules in effect at
	 * that instant. Ambiguous or skipped local times around daylight-saving transitions resolve
	 * per the platform's standard rules. Returns the input unchanged when the platform's time
	 * routines reject the value.
	 *
	 * @param LocalDateTime The timestamp treated as local time.
	 * @return The equivalent UTC time.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|DateTime")
	static FDateTime ToUtcTime(const FDateTime& LocalDateTime);

	/**
	 * Formats a time span as clock-like text. The output is `hh:mm:ss`, prefixed with days as
	 * `d.` when the span holds one day or more. Negative spans start with `-`.
	 *
	 * @param TimeSpan The span to format.
	 * @param bIncludeMilliseconds When `true`, seconds are followed by `.fff`.
	 * @return The formatted span, such as `02:05:09` or `3.14:00:00`.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|DateTime")
	static FString FormatTimeSpan(const FTimespan& TimeSpan, const bool bIncludeMilliseconds = false);
};
