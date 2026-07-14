// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilRegexFunctionLibrary.h"
#include "Internationalization/Regex.h"

namespace
{
	FRegexPattern MakePattern(const FString& Pattern, const bool bCaseSensitive)
	{
		return FRegexPattern(Pattern, bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);
	}
}

bool UDirectiveUtilRegexFunctionLibrary::RegexMatches(const FString& Input, const FString& Pattern, const bool bCaseSensitive)
{
	if (Pattern.IsEmpty())
	{
		return false;
	}

	const FRegexPattern RegexPattern = MakePattern(Pattern, bCaseSensitive);
	FRegexMatcher Matcher(RegexPattern, Input);
	return Matcher.FindNext();
}

bool UDirectiveUtilRegexFunctionLibrary::RegexFindFirst(const FString& Input, const FString& Pattern, FString& OutMatch, int32& OutMatchStart, int32& OutMatchEnd, const bool bCaseSensitive)
{
	OutMatch = FString();
	OutMatchStart = INDEX_NONE;
	OutMatchEnd = INDEX_NONE;

	if (Pattern.IsEmpty())
	{
		return false;
	}

	const FRegexPattern RegexPattern = MakePattern(Pattern, bCaseSensitive);
	FRegexMatcher Matcher(RegexPattern, Input);
	if (Matcher.FindNext())
	{
		OutMatchStart = Matcher.GetMatchBeginning();
		OutMatchEnd = Matcher.GetMatchEnding();
		OutMatch = Matcher.GetCaptureGroup(0);
		return true;
	}
	return false;
}

TArray<FString> UDirectiveUtilRegexFunctionLibrary::RegexFindAll(const FString& Input, const FString& Pattern, const bool bCaseSensitive)
{
	TArray<FString> Matches;
	if (Pattern.IsEmpty())
	{
		return Matches;
	}

	const FRegexPattern RegexPattern = MakePattern(Pattern, bCaseSensitive);
	FRegexMatcher Matcher(RegexPattern, Input);
	while (Matcher.FindNext())
	{
		const int32 Begin = Matcher.GetMatchBeginning();
		const int32 End = Matcher.GetMatchEnding();
		if (Begin == End)
		{
			continue;
		}
		Matches.Add(Matcher.GetCaptureGroup(0));
	}
	return Matches;
}

FString UDirectiveUtilRegexFunctionLibrary::RegexReplaceAll(const FString& Input, const FString& Pattern, const FString& Replacement, const bool bCaseSensitive)
{
	if (Pattern.IsEmpty())
	{
		return Input;
	}

	const FRegexPattern RegexPattern = MakePattern(Pattern, bCaseSensitive);
	FRegexMatcher Matcher(RegexPattern, Input);

	FString Result;
	int32 LastEnd = 0;
	while (Matcher.FindNext())
	{
		const int32 Begin = Matcher.GetMatchBeginning();
		const int32 End = Matcher.GetMatchEnding();

		if (Begin == End)
		{
			continue;
		}

		if (Begin < LastEnd)
		{
			continue;
		}

		Result.Append(Input.Mid(LastEnd, Begin - LastEnd));
		Result.Append(Replacement);
		LastEnd = End;
	}
	Result.Append(Input.Mid(LastEnd));
	return Result;
}

bool UDirectiveUtilRegexFunctionLibrary::RegexGetCaptureGroup(const FString& Input, const FString& Pattern, const int32 GroupIndex, FString& OutGroup, const bool bCaseSensitive)
{
	OutGroup = FString();

	if (Pattern.IsEmpty() || GroupIndex < 0)
	{
		return false;
	}

	const FRegexPattern RegexPattern = MakePattern(Pattern, bCaseSensitive);
	FRegexMatcher Matcher(RegexPattern, Input);
	if (!Matcher.FindNext())
	{
		return false;
	}

	if (Matcher.GetCaptureGroupBeginning(GroupIndex) == INDEX_NONE)
	{
		return false;
	}

	OutGroup = Matcher.GetCaptureGroup(GroupIndex);
	return true;
}
