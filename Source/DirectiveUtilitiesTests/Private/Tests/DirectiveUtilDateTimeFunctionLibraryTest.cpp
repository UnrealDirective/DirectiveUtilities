// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilDateTimeFunctionLibrary.h"
#include "Misc/AutomationTest.h"

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

	FDateTime Parsed;
	TestTrue(TEXT("Parsing matches the pattern"),
		UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(TEXT("2026-08-22 09:05:03.412"), TEXT("yyyy-MM-dd HH:mm:ss.fff"), Parsed));
	TestEqual(TEXT("The parsed value equals the original"), Parsed, Sample);

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
	const FDateTime Local = UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(RoundTripInput);
	const FDateTime Restored = UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(Local);
	// A signed comparison here would pass for any negative offset, however wrong.
	TestEqual(TEXT("Local to UTC round-trips exactly"), Restored, RoundTripInput);
	TestEqual(TEXT("Sub-second precision survives the round trip"), Restored.GetMillisecond(), 412);
	TestEqual(TEXT("Sub-millisecond ticks survive the round trip"), Restored.GetTicks(), RoundTripInput.GetTicks());
	const FDateTime HistoricalInput(1965, 6, 15, 12, 0, 0, 321);
	const FDateTime HistoricalLocal = UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(HistoricalInput);
	TestEqual(TEXT("A pre-1970 timestamp round-trips through local time"),
		UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(HistoricalLocal), HistoricalInput);

	// The component constructor is fatal on out-of-range values, so applying an offset at
	// either end of the range must fall back instead of constructing. Reaching the assertion
	// at all is the result being checked.
	for (const FDateTime& Extreme : { FDateTime::MinValue(), FDateTime::MaxValue(), FDateTime() })
	{
		const FDateTime Converted = UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(Extreme);
		TestTrue(TEXT("Converting a range endpoint stays in range"),
			Converted >= FDateTime::MinValue() && Converted <= FDateTime::MaxValue());
		const FDateTime Back = UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(Extreme);
		TestTrue(TEXT("Converting a range endpoint back stays in range"),
			Back >= FDateTime::MinValue() && Back <= FDateTime::MaxValue());
	}

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

	return true;
}
