// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilDateTimeFunctionLibrary.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilDateTimeFunctionLibraryTest,
	"DirectiveUtilities.DateTimeFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilDateTimeFunctionLibraryTest::RunTest(const FString& Parameters)
{
	const FDateTime Sample(2026, 8, 22, 9, 5, 3, 412);

	FString Formatted;
	TestTrue(TEXT("A full pattern formats"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("yyyy-MM-dd HH:mm:ss.fff"), Formatted));
	TestEqual(TEXT("Every token formats"), Formatted, FString(TEXT("2026-08-22 09:05:03.412")));

	FString Quoted;
	TestTrue(TEXT("Quoted literals format verbatim"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("'yyyy' yyyy"), Quoted));
	TestEqual(TEXT("The literal kept its letters"), Quoted, FString(TEXT("yyyy 2026")));

	FString DoubledQuote;
	TestTrue(TEXT("A doubled quote emits one quote"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("''yyyy''"), DoubledQuote));
	TestEqual(TEXT("The doubled quote output"), DoubledQuote, FString(TEXT("'2026'")));
	TestTrue(TEXT("A doubled quote inside a quoted literal formats"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("'it''s' yyyy"), DoubledQuote));
	TestEqual(TEXT("The inner doubled quote output"), DoubledQuote, FString(TEXT("it's 2026")));

	FString AliasedPattern(TEXT("yyyy-MM-dd"));
	TestTrue(TEXT("The pattern may alias the output"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, AliasedPattern, AliasedPattern));
	TestEqual(TEXT("Aliased formatting output"), AliasedPattern, FString(TEXT("2026-08-22")));

	FString BadPattern;
	TestFalse(TEXT("An unknown token run fails formatting"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("mmmm"), BadPattern));
	TestEqual(TEXT("Failed formatting leaves no text"), BadPattern, FString());
	TestFalse(TEXT("An unterminated quote fails formatting"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("'open"), BadPattern));
	TestFalse(TEXT("Uppercase year and day letters fail formatting"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("YYYY-MM-DD"), BadPattern));
	TestEqual(TEXT("A rejected letter leaves no text"), BadPattern, FString());
	TestFalse(TEXT("An unquoted letter between tokens fails formatting"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("yyyy-MM-ddTHH:mm"), BadPattern));
	TestTrue(TEXT("Non-ASCII literal text formats"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(Sample, TEXT("yyyy\u5E74MM\u6708"), Formatted));
	TestEqual(TEXT("Non-ASCII literal text is emitted verbatim"), Formatted, FString(TEXT("2026\u5E7408\u6708")));

	FDateTime Parsed;
	TestTrue(TEXT("Parsing matches the pattern"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-08-22 09:05:03.412"), TEXT("yyyy-MM-dd HH:mm:ss.fff"), Parsed));
	TestEqual(TEXT("The parsed value equals the original"), Parsed, Sample);

	FDateTime LetterPattern;
	TestFalse(TEXT("Uppercase year and day letters fail parsing"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-08-22"), TEXT("YYYY-MM-DD"), LetterPattern));
	TestEqual(TEXT("A rejected parse pattern leaves the default value"), LetterPattern, FDateTime());
	FDateTime CaseSensitiveLiteral;
	TestTrue(TEXT("A quoted letter literal parses with matching case"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026T08"), TEXT("yyyy'T'MM"), CaseSensitiveLiteral));
	TestEqual(TEXT("The matching-case literal parse"), CaseSensitiveLiteral, FDateTime(2026, 8, 1));
	TestFalse(TEXT("A literal with different case fails parsing"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026t08"), TEXT("yyyy'T'MM"), CaseSensitiveLiteral));

	FDateTime TrailingJunk;
	TestFalse(TEXT("Trailing text fails parsing"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-08-22x"), TEXT("yyyy-MM-dd"), TrailingJunk));

	FDateTime InvalidMonth;
	TestFalse(TEXT("Month thirteen is rejected"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-13-01"), TEXT("yyyy-MM-dd"), InvalidMonth));

	FDateTime ImpossibleDay;
	TestFalse(TEXT("February thirtieth is rejected"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-02-30"), TEXT("yyyy-MM-dd"), ImpossibleDay));

	FDateTime ShortDigits;
	TestFalse(TEXT("Short digit runs fail parsing"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("26-08-22"), TEXT("yyyy-MM-dd"), ShortDigits));

	FDateTime TimeOnly;
	TestTrue(TEXT("A time-only pattern uses valid default date components"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("09:05:03"), TEXT("HH:mm:ss"), TimeOnly));
	TestEqual(TEXT("The time-only default"), TimeOnly, FDateTime(1, 1, 1, 9, 5, 3));

	FDateTime RepeatedField;
	TestTrue(TEXT("An equal repeated field is accepted"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026/2026"), TEXT("yyyy/yyyy"), RepeatedField));
	TestFalse(TEXT("A conflicting repeated field is rejected"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026/2027"), TEXT("yyyy/yyyy"), RepeatedField));
	TestTrue(TEXT("A doubled quote inside a literal parses"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("it's 2026"), TEXT("'it''s' yyyy"), RepeatedField));

	// Mid-June sits clear of every zone's DST transition, so the local time is unambiguous.
	const FDateTime RoundTripInput = FDateTime::FromUnixTimestamp(1781524800)
		+ FTimespan(412 * ETimespan::TicksPerMillisecond + 3456);
	FDateTime Local;
	FDateTime Restored;
	TestTrue(TEXT("A mid-June UTC timestamp converts to local time"),
		UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(RoundTripInput, Local));
	TestTrue(TEXT("The local time converts back to UTC"),
		UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(Local, Restored));
	// A signed comparison here would pass for any negative offset, however wrong.
	TestEqual(TEXT("Local to UTC round-trips exactly"), Restored, RoundTripInput);
	TestEqual(TEXT("Sub-second precision survives the round trip"), Restored.GetMillisecond(), 412);
	TestEqual(TEXT("Sub-millisecond ticks survive the round trip"), Restored.GetTicks(), RoundTripInput.GetTicks());
	const FTimespan RoundTripOffset = Local - RoundTripInput;
	TestEqual(TEXT("The local offset is a whole number of minutes"), RoundTripOffset.GetTicks() % ETimespan::TicksPerMinute, static_cast<int64>(0));
	TestTrue(TEXT("The local offset is within the range of real time zones"), FMath::Abs(RoundTripOffset.GetTotalHours()) <= 14.0);

	// FDateTime::Now and UtcNow read the same system zone, so this holds in any zone, including UTC.
	const FDateTime UtcNow = FDateTime::UtcNow();
	const FDateTime LocalNow = FDateTime::Now();
	FDateTime ConvertedNow;
	TestTrue(TEXT("The current UTC time converts to local time"),
		UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(UtcNow, ConvertedNow));
	TestTrue(TEXT("ToLocalTime agrees with the engine's local clock"),
		FMath::Abs((ConvertedNow - LocalNow).GetTotalSeconds()) < 60.0);
	TestTrue(TEXT("The current local time converts to UTC"),
		UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(LocalNow, ConvertedNow));
	TestTrue(TEXT("ToUtcTime agrees with the engine's UTC clock"),
		FMath::Abs((ConvertedNow - UtcNow).GetTotalSeconds()) < 60.0);

	const FDateTime HistoricalInput(1965, 6, 15, 12, 0, 0, 321);
	FDateTime HistoricalLocal;
	FDateTime HistoricalRestored;
	TestTrue(TEXT("A pre-1970 timestamp converts to local time"),
		UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(HistoricalInput, HistoricalLocal));
	TestTrue(TEXT("A pre-1970 local time converts back to UTC"),
		UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(HistoricalLocal, HistoricalRestored));
	TestEqual(TEXT("A pre-1970 timestamp round-trips through local time"), HistoricalRestored, HistoricalInput);

	// The component constructor is fatal on out-of-range values, so applying an offset at
	// either end of the range must fail instead of constructing.
	for (const FDateTime& Extreme : { FDateTime::MinValue(), FDateTime::MaxValue() })
	{
		FDateTime Converted;
		if (!UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(Extreme, Converted))
		{
			TestEqual(TEXT("A failed conversion to local time outputs the input"), Converted, Extreme);
		}
		TestTrue(TEXT("Converting a range endpoint stays in range"),
			Converted >= FDateTime::MinValue() && Converted <= FDateTime::MaxValue());
		FDateTime Back;
		if (!UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(Extreme, Back))
		{
			TestEqual(TEXT("A failed conversion to UTC outputs the input"), Back, Extreme);
		}
		TestTrue(TEXT("Converting a range endpoint back stays in range"),
			Back >= FDateTime::MinValue() && Back <= FDateTime::MaxValue());
	}

#if PLATFORM_WINDOWS
	const FDateTime BeforeSystemTimeRange(1600, 6, 15, 12, 0, 0);
	FDateTime RejectedLocal;
	TestFalse(TEXT("Windows rejects a date before 1601 for local conversion"),
		UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(BeforeSystemTimeRange, RejectedLocal));
	TestEqual(TEXT("A rejected local conversion outputs the input"), RejectedLocal, BeforeSystemTimeRange);
	FDateTime RejectedUtc;
	TestFalse(TEXT("Windows rejects a date before 1601 for UTC conversion"),
		UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(BeforeSystemTimeRange, RejectedUtc));
	TestEqual(TEXT("A rejected UTC conversion outputs the input"), RejectedUtc, BeforeSystemTimeRange);
#endif

	TestEqual(TEXT("Formatting a sub-day span"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(FTimespan(0, 2, 5, 9), false), FString(TEXT("02:05:09")));
	TestEqual(TEXT("Formatting a multi-day span"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(FTimespan(3, 14, 0, 0), false), FString(TEXT("3.14:00:00")));
	const FTimespan OneAndHalfSeconds(ETimespan::TicksPerSecond + 500 * ETimespan::TicksPerMillisecond);
	TestEqual(TEXT("Formatting with milliseconds"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(OneAndHalfSeconds, true), FString(TEXT("00:00:01.500")));
	TestTrue(TEXT("Negative spans start with a dash"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(FTimespan(-FTimespan(0, 1, 0, 0).GetTicks()), false).StartsWith(TEXT("-")));
	TestEqual(TEXT("The minimum span formats exactly"),
		UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(FTimespan::MinValue(), true),
		FString(TEXT("-10675199.02:48:05.477")));

	for (const TCHAR* FunctionName : { TEXT("FormatDateTime"), TEXT("TryParseDateTime"), TEXT("ToLocalTime"), TEXT("ToUtcTime"), TEXT("FormatTimeSpan") })
	{
		const UFunction* Function = UDirectiveUtilDateTimeFunctionLibrary::StaticClass()->FindFunctionByName(FName(FunctionName));
		TestTrue(*FString::Printf(TEXT("%s is a pure node"), FunctionName),
			Function != nullptr && Function->HasAnyFunctionFlags(FUNC_BlueprintPure));
	}

	return true;
}
