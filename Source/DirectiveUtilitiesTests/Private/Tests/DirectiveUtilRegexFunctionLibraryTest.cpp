// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilRegexFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilRegexFunctionLibraryTest, "DirectiveUtilities.RegexFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilRegexFunctionLibraryTest::RunTest(const FString& Parameters)
{
	TestTrue("RegexMatches should match digits in 'Hello123'", UDirectiveUtilRegexFunctionLibrary::RegexMatches(TEXT("Hello123"), TEXT("[0-9]+")));
	TestFalse("RegexMatches should not match digits in 'Hello'", UDirectiveUtilRegexFunctionLibrary::RegexMatches(TEXT("Hello"), TEXT("[0-9]+")));
	TestFalse("RegexMatches should be case-sensitive by default", UDirectiveUtilRegexFunctionLibrary::RegexMatches(TEXT("HELLO"), TEXT("hello")));
	TestTrue("RegexMatches should honor the case-insensitive flag", UDirectiveUtilRegexFunctionLibrary::RegexMatches(TEXT("HELLO"), TEXT("hello"), false));
	TestFalse("RegexMatches should return false for an empty pattern", UDirectiveUtilRegexFunctionLibrary::RegexMatches(TEXT("Hello"), TEXT("")));

	{
		FString Match;
		int32 Start = 0;
		int32 End = 0;
		const bool bFound = UDirectiveUtilRegexFunctionLibrary::RegexFindFirst(TEXT("abc123def456"), TEXT("[0-9]+"), Match, Start, End);
		TestTrue("RegexFindFirst should find a match", bFound);
		TestEqual("RegexFindFirst should return the first match", Match, FString(TEXT("123")));
		TestEqual("RegexFindFirst should return the match start", Start, 3);
		TestEqual("RegexFindFirst should return the match end (exclusive)", End, 6);
	}
	{
		FString Match = TEXT("sentinel");
		int32 Start = 5;
		int32 End = 5;
		const bool bFound = UDirectiveUtilRegexFunctionLibrary::RegexFindFirst(TEXT("abc"), TEXT("[0-9]+"), Match, Start, End);
		TestFalse("RegexFindFirst should report no match", bFound);
		TestTrue("RegexFindFirst should clear the match on failure", Match.IsEmpty());
		TestEqual("RegexFindFirst should set start to INDEX_NONE on failure", Start, static_cast<int32>(INDEX_NONE));
		TestEqual("RegexFindFirst should set end to INDEX_NONE on failure", End, static_cast<int32>(INDEX_NONE));
	}

	TestEqual("RegexFindAll should return every match",
		UDirectiveUtilRegexFunctionLibrary::RegexFindAll(TEXT("a1b2c3"), TEXT("[0-9]")),
		TArray<FString>({TEXT("1"), TEXT("2"), TEXT("3")}));
	TestEqual("RegexFindAll should return nothing when there are no matches",
		UDirectiveUtilRegexFunctionLibrary::RegexFindAll(TEXT("abc"), TEXT("[0-9]")).Num(), 0);
	TestEqual("RegexFindAll should ignore zero-width matches",
		UDirectiveUtilRegexFunctionLibrary::RegexFindAll(TEXT("abc"), TEXT("x*")).Num(), 0);
	TestEqual("RegexFindAll should return only the non-empty matches of a star pattern",
		UDirectiveUtilRegexFunctionLibrary::RegexFindAll(TEXT("bab"), TEXT("a*")),
		TArray<FString>({TEXT("a")}));

	TestEqual("RegexReplaceAll should replace every match",
		UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(TEXT("a1b2c3"), TEXT("[0-9]"), TEXT("#")), FString(TEXT("a#b#c#")));
	TestEqual("RegexReplaceAll should return the input unchanged when nothing matches",
		UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(TEXT("abc"), TEXT("[0-9]"), TEXT("#")), FString(TEXT("abc")));
	TestEqual("RegexReplaceAll should return the input unchanged for an empty pattern",
		UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(TEXT("abc"), TEXT(""), TEXT("#")), FString(TEXT("abc")));
	TestEqual("RegexReplaceAll should only replace the non-empty matches of a star pattern",
		UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(TEXT("bab"), TEXT("a*"), TEXT("X")), FString(TEXT("bXb")));
	TestEqual("RegexReplaceAll should still replace every non-empty match",
		UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(TEXT("aaa"), TEXT("a"), TEXT("b")), FString(TEXT("bbb")));

	{
		FString Group;
		TestTrue("RegexGetCaptureGroup should return the whole match for group 0",
			UDirectiveUtilRegexFunctionLibrary::RegexGetCaptureGroup(TEXT("2024-06-24"), TEXT("([0-9]+)-([0-9]+)-([0-9]+)"), 0, Group));
		TestEqual("RegexGetCaptureGroup group 0 should be the whole match", Group, FString(TEXT("2024-06-24")));

		TestTrue("RegexGetCaptureGroup should return group 1",
			UDirectiveUtilRegexFunctionLibrary::RegexGetCaptureGroup(TEXT("2024-06-24"), TEXT("([0-9]+)-([0-9]+)-([0-9]+)"), 1, Group));
		TestEqual("RegexGetCaptureGroup group 1 should be the year", Group, FString(TEXT("2024")));

		TestTrue("RegexGetCaptureGroup should return group 2",
			UDirectiveUtilRegexFunctionLibrary::RegexGetCaptureGroup(TEXT("2024-06-24"), TEXT("([0-9]+)-([0-9]+)-([0-9]+)"), 2, Group));
		TestEqual("RegexGetCaptureGroup group 2 should be the month", Group, FString(TEXT("06")));

		Group = TEXT("sentinel");
		TestFalse("RegexGetCaptureGroup should fail for a non-existent group",
			UDirectiveUtilRegexFunctionLibrary::RegexGetCaptureGroup(TEXT("2024-06-24"), TEXT("([0-9]+)-([0-9]+)-([0-9]+)"), 9, Group));
		TestTrue("RegexGetCaptureGroup should clear the output for a non-existent group", Group.IsEmpty());
	}

	return true;
}
