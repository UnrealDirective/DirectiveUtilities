#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Misc/AutomationTest.h"

namespace
{
    int32 ReferenceLevenshteinDistance(const FString& Left, const FString& Right)
    {
        TArray<int32> PreviousRow;
        TArray<int32> CurrentRow;
        PreviousRow.SetNumUninitialized(Right.Len() + 1);
        CurrentRow.SetNumUninitialized(Right.Len() + 1);
        for (int32 ColumnIndex = 0; ColumnIndex <= Right.Len(); ++ColumnIndex)
        {
            PreviousRow[ColumnIndex] = ColumnIndex;
        }
        for (int32 RowIndex = 1; RowIndex <= Left.Len(); ++RowIndex)
        {
            CurrentRow[0] = RowIndex;
            for (int32 ColumnIndex = 1; ColumnIndex <= Right.Len(); ++ColumnIndex)
            {
                CurrentRow[ColumnIndex] = FMath::Min3(
                    PreviousRow[ColumnIndex] + 1,
                    CurrentRow[ColumnIndex - 1] + 1,
                    PreviousRow[ColumnIndex - 1] + (Left[RowIndex - 1] == Right[ColumnIndex - 1] ? 0 : 1));
            }
            Swap(PreviousRow, CurrentRow);
        }
        return PreviousRow.Last();
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilStringFunctionLibraryTest, "DirectiveUtilities.StringFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilStringFunctionLibraryTest::RunTest(const FString& Parameters)
{
    TestTrue("ContainsLetters should return true for 'Hello'", UDirectiveUtilStringFunctionLibrary::ContainsLetters(TEXT("Hello")));
    TestFalse("ContainsLetters should return false for '1234'", UDirectiveUtilStringFunctionLibrary::ContainsLetters(TEXT("1234")));

    TestTrue("ContainsNumbers should return true for '1234'", UDirectiveUtilStringFunctionLibrary::ContainsNumbers(TEXT("1234")));
    TestFalse("ContainsNumbers should return false for 'Hello'", UDirectiveUtilStringFunctionLibrary::ContainsNumbers(TEXT("Hello")));

    TestTrue("ContainsSpaces should return true for 'Hello World'", UDirectiveUtilStringFunctionLibrary::ContainsSpaces(TEXT("Hello World")));
    TestFalse("ContainsSpaces should return false for 'HelloWorld'", UDirectiveUtilStringFunctionLibrary::ContainsSpaces(TEXT("HelloWorld")));

    TestTrue("ContainsSpecialCharacters should return true for 'Hello!'", UDirectiveUtilStringFunctionLibrary::ContainsSpecialCharacters(TEXT("Hello!")));
    TestFalse("ContainsSpecialCharacters should return false for 'Hello'", UDirectiveUtilStringFunctionLibrary::ContainsSpecialCharacters(TEXT("Hello")));

    const FString FilteredString = UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), true, true, true, true);
    TestEqual("FilterCharacters should return an empty string", FilteredString, TEXT(""));

    const TArray<FString> UnsortedArray = { TEXT("Banana"), TEXT("Apple"), TEXT("Cherry") };
    const TArray<FString> SortedArray = UDirectiveUtilStringFunctionLibrary::GetSortedStringArray(UnsortedArray);
    TestEqual("GetSortedStringArray should return a sorted array", SortedArray, TArray<FString>({ TEXT("Apple"), TEXT("Banana"), TEXT("Cherry") }));

    TestEqual("TruncateString should not truncate short strings",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello"), 10), FString(TEXT("Hello")));
    TestEqual("TruncateString should truncate long strings with suffix",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 8), FString(TEXT("Hello...")));
    TestEqual("TruncateString should not truncate strings at exact max length",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello"), 5), FString(TEXT("Hello")));

    TestEqual("ToTitleCase should capitalize first letter of each word",
        UDirectiveUtilStringFunctionLibrary::ToTitleCase(TEXT("hello world")), FString(TEXT("Hello World")));
    TestEqual("ToTitleCase should handle all caps",
        UDirectiveUtilStringFunctionLibrary::ToTitleCase(TEXT("HELLO WORLD")), FString(TEXT("Hello World")));
    TestEqual("ToTitleCase should handle single word",
        UDirectiveUtilStringFunctionLibrary::ToTitleCase(TEXT("hello")), FString(TEXT("Hello")));
    TestEqual("ToTitleCase should handle empty string",
        UDirectiveUtilStringFunctionLibrary::ToTitleCase(TEXT("")), FString(TEXT("")));

    // When MaxLength < Suffix.Len(), MaxLength - Suffix.Len() is negative; FString::Left clamps
    // negative counts to 0, so the result is just the suffix (no crash).
    TestEqual("TruncateString should return only the suffix when MaxLength is less than the suffix length",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 2), FString(TEXT("...")));
    TestEqual("TruncateString should return only the suffix when MaxLength is zero",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 0), FString(TEXT("...")));
    TestEqual("TruncateString should return only the suffix when MaxLength equals the suffix length",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 3), FString(TEXT("...")));
    TestEqual("TruncateString should not truncate an empty input string",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT(""), 5), FString(TEXT("")));
    TestEqual("TruncateString should honor a custom suffix",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 6, TEXT(">>")), FString(TEXT("Hell>>")));
    TestEqual("TruncateString should return only a custom suffix when MaxLength is less than its length",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 1, TEXT(">>")), FString(TEXT(">>")));
    TestEqual("TruncateString should handle an empty suffix by returning a hard cut",
        UDirectiveUtilStringFunctionLibrary::TruncateString(TEXT("Hello World"), 5, TEXT("")), FString(TEXT("Hello")));

    TestFalse("ContainsLetters should return false for an empty string", UDirectiveUtilStringFunctionLibrary::ContainsLetters(TEXT("")));
    TestFalse("ContainsNumbers should return false for an empty string", UDirectiveUtilStringFunctionLibrary::ContainsNumbers(TEXT("")));
    TestFalse("ContainsSpaces should return false for an empty string", UDirectiveUtilStringFunctionLibrary::ContainsSpaces(TEXT("")));
    TestFalse("ContainsSpecialCharacters should return false for an empty string", UDirectiveUtilStringFunctionLibrary::ContainsSpecialCharacters(TEXT("")));

    TestTrue("ContainsSpaces should treat a tab as whitespace", UDirectiveUtilStringFunctionLibrary::ContainsSpaces(TEXT("Hello\tWorld")));
    TestFalse("ContainsSpecialCharacters should return false for letters and digits only", UDirectiveUtilStringFunctionLibrary::ContainsSpecialCharacters(TEXT("Hello123")));

    TestEqual("FilterCharacters should strip only letters",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), true, false, false, false), FString(TEXT("123! ")));
    TestEqual("FilterCharacters should strip only numbers",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), false, true, false, false), FString(TEXT("Hello! ")));
    TestEqual("FilterCharacters should strip only special characters",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), false, false, true, false), FString(TEXT("Hello123 ")));
    TestEqual("FilterCharacters should strip only spaces",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), false, false, false, true), FString(TEXT("Hello123!")));
    TestEqual("FilterCharacters should return the original string when no flags are set",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), false, false, false, false), FString(TEXT("Hello123! ")));
    TestEqual("FilterCharacters should return an empty string for an empty input",
        UDirectiveUtilStringFunctionLibrary::FilterCharacters(TEXT(""), true, true, true, true), FString(TEXT("")));

    {
        const FString EquivalentInputs[] = {
            TEXT("my var name"), TEXT("my_var_name"), TEXT("my-var-name"), TEXT("MyVarName"), TEXT("myVarName")
        };
        for (const FString& Input : EquivalentInputs)
        {
            TestEqual(FString::Printf(TEXT("ToCamelCase(\"%s\") is \"myVarName\""), *Input),
                UDirectiveUtilStringFunctionLibrary::ToCamelCase(Input), FString(TEXT("myVarName")));
            TestEqual(FString::Printf(TEXT("ToPascalCase(\"%s\") is \"MyVarName\""), *Input),
                UDirectiveUtilStringFunctionLibrary::ToPascalCase(Input), FString(TEXT("MyVarName")));
            TestEqual(FString::Printf(TEXT("ToSnakeCase(\"%s\") is \"my_var_name\""), *Input),
                UDirectiveUtilStringFunctionLibrary::ToSnakeCase(Input), FString(TEXT("my_var_name")));
            TestEqual(FString::Printf(TEXT("ToKebabCase(\"%s\") is \"my-var-name\""), *Input),
                UDirectiveUtilStringFunctionLibrary::ToKebabCase(Input), FString(TEXT("my-var-name")));
        }

        TestEqual("SplitIntoWords keeps acronyms together",
            UDirectiveUtilStringFunctionLibrary::SplitIntoWords(TEXT("XMLParser")),
            TArray<FString>({ TEXT("XML"), TEXT("Parser") }));
        TestEqual("SplitIntoWords splits on letter/digit transitions",
            UDirectiveUtilStringFunctionLibrary::SplitIntoWords(TEXT("version2Beta")),
            TArray<FString>({ TEXT("version"), TEXT("2"), TEXT("Beta") }));
        TestEqual("SplitIntoWords of an empty string is an empty array",
            UDirectiveUtilStringFunctionLibrary::SplitIntoWords(TEXT("")), TArray<FString>());

        TestEqual("ToCamelCase(\"XMLParser\") is \"xmlParser\"",
            UDirectiveUtilStringFunctionLibrary::ToCamelCase(TEXT("XMLParser")), FString(TEXT("xmlParser")));
        TestEqual("ToSnakeCase(\"XMLParser\") is \"xml_parser\"",
            UDirectiveUtilStringFunctionLibrary::ToSnakeCase(TEXT("XMLParser")), FString(TEXT("xml_parser")));
        TestEqual("ToSnakeCase(\"version2Beta\") is \"version_2_beta\"",
            UDirectiveUtilStringFunctionLibrary::ToSnakeCase(TEXT("version2Beta")), FString(TEXT("version_2_beta")));

        TestEqual("ToCamelCase of an empty string is empty",
            UDirectiveUtilStringFunctionLibrary::ToCamelCase(TEXT("")), FString(TEXT("")));
        TestEqual("ToPascalCase of an empty string is empty",
            UDirectiveUtilStringFunctionLibrary::ToPascalCase(TEXT("")), FString(TEXT("")));
        TestEqual("ToSnakeCase of an empty string is empty",
            UDirectiveUtilStringFunctionLibrary::ToSnakeCase(TEXT("")), FString(TEXT("")));
        TestEqual("ToKebabCase of an empty string is empty",
            UDirectiveUtilStringFunctionLibrary::ToKebabCase(TEXT("")), FString(TEXT("")));
    }

    const TArray<FString> UnsortedForLegacy = { TEXT("Banana"), TEXT("Apple"), TEXT("Cherry") };
    TestEqual("SortStringArray should return a sorted array",
        UDirectiveUtilStringFunctionLibrary::SortStringArray(UnsortedForLegacy),
        TArray<FString>({ TEXT("Apple"), TEXT("Banana"), TEXT("Cherry") }));
    TestEqual("SortStringArray should return an empty array for empty input",
        UDirectiveUtilStringFunctionLibrary::SortStringArray(TArray<FString>()), TArray<FString>());

    TestEqual("GetSortedStringArray should return an empty array for empty input",
        UDirectiveUtilStringFunctionLibrary::GetSortedStringArray(TArray<FString>()), TArray<FString>());

    TestEqual("Levenshtein of identical strings is 0",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("abc"), TEXT("abc")), 0);
    TestEqual("Levenshtein kitten->sitting is 3",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("kitten"), TEXT("sitting")), 3);
    TestEqual("Levenshtein from empty equals the other length",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT(""), TEXT("abc")), 3);
    TestEqual("Levenshtein is case-sensitive by default",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("ABC"), TEXT("abc")), 3);
    TestEqual("Levenshtein honors the case-insensitive flag",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("ABC"), TEXT("abc"), /*bCaseSensitive*/ false), 0);

    TestTrue("Similarity of identical strings is 1",
        FMath::IsNearlyEqual(UDirectiveUtilStringFunctionLibrary::GetStringSimilarity(TEXT("abc"), TEXT("abc")), 1.0f, 1.e-4f));
    TestTrue("Similarity of two empty strings is 1",
        FMath::IsNearlyEqual(UDirectiveUtilStringFunctionLibrary::GetStringSimilarity(TEXT(""), TEXT("")), 1.0f, 1.e-4f));
    TestTrue("Similarity of completely different equal-length strings is 0",
        FMath::IsNearlyEqual(UDirectiveUtilStringFunctionLibrary::GetStringSimilarity(TEXT("abc"), TEXT("xyz")), 0.0f, 1.e-4f));
    TestTrue("Similarity kitten/sitting is ~0.571",
        FMath::IsNearlyEqual(UDirectiveUtilStringFunctionLibrary::GetStringSimilarity(TEXT("kitten"), TEXT("sitting")), 1.0f - 3.0f / 7.0f, 1.e-3f));

    TestTrue("ContainsAny finds a present term",
        UDirectiveUtilStringFunctionLibrary::ContainsAny(TEXT("Hello World"), {TEXT("foo"), TEXT("World")}));
    TestFalse("ContainsAny returns false when no term is present",
        UDirectiveUtilStringFunctionLibrary::ContainsAny(TEXT("Hello"), {TEXT("foo"), TEXT("bar")}));
    TestTrue("ContainsAny honors the case-insensitive flag",
        UDirectiveUtilStringFunctionLibrary::ContainsAny(TEXT("HELLO"), {TEXT("hello")}, /*bCaseSensitive*/ false));
    TestFalse("ContainsAny ignores empty terms",
        UDirectiveUtilStringFunctionLibrary::ContainsAny(TEXT("Hello"), {TEXT("")}));

    {
        int32 FoundIndex = 0;
        int32 TermIndex = 0;
        const bool bFound = UDirectiveUtilStringFunctionLibrary::FindFirstOfAny(TEXT("abcXYZ123"), {TEXT("123"), TEXT("XYZ")}, true, FoundIndex, TermIndex);
        TestTrue("FindFirstOfAny should find a term", bFound);
        TestEqual("FindFirstOfAny should return the earliest match index", FoundIndex, 3);
        TestEqual("FindFirstOfAny should return which term matched earliest", TermIndex, 1);
    }
    {
        int32 FoundIndex = 5;
        int32 TermIndex = 5;
        const bool bFound = UDirectiveUtilStringFunctionLibrary::FindFirstOfAny(TEXT("abc"), {TEXT("x"), TEXT("y")}, true, FoundIndex, TermIndex);
        TestFalse("FindFirstOfAny should report no match", bFound);
        TestEqual("FindFirstOfAny should set found index to INDEX_NONE on failure", FoundIndex, static_cast<int32>(INDEX_NONE));
        TestEqual("FindFirstOfAny should set term index to INDEX_NONE on failure", TermIndex, static_cast<int32>(INDEX_NONE));
    }

    {
        const FString Original = TEXT("Hello, DirectiveUtilities! 123");
        const FString Encoded = UDirectiveUtilStringFunctionLibrary::Base64Encode(Original);
        TestFalse("Base64Encode should produce a non-empty string", Encoded.IsEmpty());
        FString Decoded;
        const bool bDecoded = UDirectiveUtilStringFunctionLibrary::Base64Decode(Encoded, Decoded);
        TestTrue("Base64Decode should succeed for valid input", bDecoded);
        TestEqual("Base64 should round-trip the original string", Decoded, Original);
    }

    {
        TestEqual("HexEncode(\"Hi!\") is \"486921\"",
            UDirectiveUtilStringFunctionLibrary::HexEncode(TEXT("Hi!")), FString(TEXT("486921")));
        TestEqual("HexEncode of an empty string is empty",
            UDirectiveUtilStringFunctionLibrary::HexEncode(TEXT("")), FString(TEXT("")));

        FString Decoded;
        TestTrue("HexDecode should decode valid hex", UDirectiveUtilStringFunctionLibrary::HexDecode(TEXT("486921"), Decoded));
        TestEqual("HexDecode should round-trip HexEncode", Decoded, FString(TEXT("Hi!")));
        TestTrue("HexDecode should accept uppercase hex", UDirectiveUtilStringFunctionLibrary::HexDecode(TEXT("4A"), Decoded));
        TestEqual("HexDecode of \"4A\" is \"J\"", Decoded, FString(TEXT("J")));
        TestFalse("HexDecode should reject non-hex characters", UDirectiveUtilStringFunctionLibrary::HexDecode(TEXT("XYZ"), Decoded));
        TestFalse("HexDecode should reject odd-length input", UDirectiveUtilStringFunctionLibrary::HexDecode(TEXT("48692"), Decoded));

        TestEqual("HexEncodeBytes({0x00, 0xFF}) is \"00ff\"",
            UDirectiveUtilStringFunctionLibrary::HexEncodeBytes({0x00, 0xFF}), FString(TEXT("00ff")));
        TArray<uint8> DecodedBytes;
        TestTrue("HexDecodeBytes should decode valid hex", UDirectiveUtilStringFunctionLibrary::HexDecodeBytes(TEXT("00ff"), DecodedBytes));
        TestEqual("HexDecodeBytes should round-trip HexEncodeBytes", DecodedBytes, TArray<uint8>({0x00, 0xFF}));
        TestFalse("HexDecodeBytes should reject non-hex characters", UDirectiveUtilStringFunctionLibrary::HexDecodeBytes(TEXT("zz"), DecodedBytes));
    }

    TestEqual("Md5HashString(\"abc\") matches the known vector",
        UDirectiveUtilStringFunctionLibrary::Md5HashString(TEXT("abc")), FString(TEXT("900150983cd24fb0d6963f7d28e17f72")));
    TestEqual("Md5HashString of an empty string matches the known vector",
        UDirectiveUtilStringFunctionLibrary::Md5HashString(TEXT("")), FString(TEXT("d41d8cd98f00b204e9800998ecf8427e")));
    TestEqual("Sha1HashString(\"abc\") matches the known vector",
        UDirectiveUtilStringFunctionLibrary::Sha1HashString(TEXT("abc")), FString(TEXT("a9993e364706816aba3e25717850c26c9cd0d89d")));
    TestEqual("Sha1HashBytes of {0x61,0x62,0x63} matches the known vector",
        UDirectiveUtilStringFunctionLibrary::Sha1HashBytes({0x61, 0x62, 0x63}), FString(TEXT("a9993e364706816aba3e25717850c26c9cd0d89d")));
    TestEqual("Crc32String(\"abc\") equals Crc32Bytes of its ASCII bytes",
        UDirectiveUtilStringFunctionLibrary::Crc32String(TEXT("abc")),
        UDirectiveUtilStringFunctionLibrary::Crc32Bytes({0x61, 0x62, 0x63}));
    TestEqual("Crc32String of an empty string is 0",
        UDirectiveUtilStringFunctionLibrary::Crc32String(TEXT("")), 0);
    TestEqual("Md5HashString hashes the UTF-8 bytes of non-ASCII input",
        UDirectiveUtilStringFunctionLibrary::Md5HashString(TEXT("\u00E9")), // e-acute; UTF-8 bytes C3 A9
        UDirectiveUtilStringFunctionLibrary::Md5HashBytes({0xC3, 0xA9}));

    TestTrue("IsValidFileName should accept a simple name", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("Save01")));
    TestTrue("IsValidFileName should accept a name with a space and extension", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("my save.sav")));
    TestFalse("IsValidFileName should reject a relative segment", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("../escape")));
    TestFalse("IsValidFileName should reject a forward slash", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("a/b")));
    TestFalse("IsValidFileName should reject a backslash", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("a\\b")));
    TestFalse("IsValidFileName should reject an empty string", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("")));
    TestFalse("IsValidFileName should reject a reserved character", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("a:b")));
    TestFalse("IsValidFileName should reject the current-directory segment", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT(".")));
    TestFalse("IsValidFileName should reject the parent-directory segment", UDirectiveUtilStringFunctionLibrary::IsValidFileName(TEXT("..")));

    {
        const FString Sanitized = UDirectiveUtilStringFunctionLibrary::SanitizeFileName(TEXT("../a/b?.sav"));
        TestTrue("SanitizeFileName should produce a name that IsValidFileName accepts",
            UDirectiveUtilStringFunctionLibrary::IsValidFileName(Sanitized));
        const FString Replaced = UDirectiveUtilStringFunctionLibrary::SanitizeFileName(TEXT("a/b"), TEXT("_"));
        TestTrue("SanitizeFileName should substitute the replacement character for stripped characters",
            Replaced.Contains(TEXT("_")));
        TestTrue("SanitizeFileName with a replacement should still produce a valid file name",
            UDirectiveUtilStringFunctionLibrary::IsValidFileName(Replaced));
    }

    {
        float Similarity = -1.0f;
        TestEqual("FindBestStringMatch should return the exact match",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("color"), {TEXT("colour"), TEXT("color")}, Similarity), 1);
        TestTrue("FindBestStringMatch should report similarity 1 for an exact match",
            FMath::IsNearlyEqual(Similarity, 1.0f, 1.e-4f));

        TestEqual("FindBestStringMatch should return the nearest match",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("colr"), {TEXT("color"), TEXT("colour"), TEXT("colt")}, Similarity), 0);

        TestEqual("FindBestStringMatch should be case-insensitive by default",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("COLOR"), {TEXT("apple"), TEXT("color")}, Similarity), 1);
        TestTrue("FindBestStringMatch should report similarity 1 for a case-insensitive match",
            FMath::IsNearlyEqual(Similarity, 1.0f, 1.e-4f));

        TestEqual("FindBestStringMatch should honor the case-sensitive flag",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("COLOR"), {TEXT("color"), TEXT("COLT")}, Similarity, /*bCaseSensitive*/ true), 1);

        TestEqual("FindBestStringMatch should return -1 for an empty array",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("color"), TArray<FString>(), Similarity), static_cast<int32>(INDEX_NONE));
        TestTrue("FindBestStringMatch should report similarity 0 for an empty array",
            FMath::IsNearlyEqual(Similarity, 0.0f, 1.e-4f));

        TestEqual("FindBestStringMatch should still return an index for an empty input",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT(""), {TEXT("abc")}, Similarity), 0);
        TestTrue("FindBestStringMatch should report similarity 0 for an empty input against a non-empty candidate",
            FMath::IsNearlyEqual(Similarity, 0.0f, 1.e-4f));

        TestEqual("FindBestStringMatch should keep the first equally close candidate",
            UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("cat"), {TEXT("bat"), TEXT("hat")}, Similarity), 0);
    }

    TestEqual("Levenshtein should trim equal prefixes and suffixes without changing the result",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("shared-prefix-A-shared-suffix"), TEXT("shared-prefix-B-shared-suffix")), 1);
    TestEqual("Levenshtein should support Unicode code units",
        UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(TEXT("café-one"), TEXT("café-two")), 3);

    FRandomStream LevenshteinStream(90210);
    for (int32 Iteration = 0; Iteration < 200; ++Iteration)
    {
        FString Left;
        FString Right;
        const int32 LeftLength = LevenshteinStream.RandRange(0, 64);
        const int32 RightLength = LevenshteinStream.RandRange(0, 64);
        Left.Reserve(LeftLength);
        Right.Reserve(RightLength);
        for (int32 Index = 0; Index < LeftLength; ++Index)
        {
            Left.AppendChar(static_cast<TCHAR>(TEXT('a') + LevenshteinStream.RandRange(0, 5)));
        }
        for (int32 Index = 0; Index < RightLength; ++Index)
        {
            Right.AppendChar(static_cast<TCHAR>(TEXT('a') + LevenshteinStream.RandRange(0, 5)));
        }

        const int32 ExpectedDistance = ReferenceLevenshteinDistance(Left, Right);
        TestEqual(
            FString::Printf(TEXT("Levenshtein fuzz case %d"), Iteration),
            UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(Left, Right),
            ExpectedDistance);
        TestEqual(
            FString::Printf(TEXT("Levenshtein symmetry case %d"), Iteration),
            UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(Right, Left),
            ExpectedDistance);
    }

    return true;
}
