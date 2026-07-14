// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Misc/Base64.h"
#include "Misc/Crc.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

namespace
{
	TArray<uint8> StringToUtf8Bytes(const FString& String)
	{
		const FTCHARToUTF8 Converter(*String, String.Len());
		return TArray<uint8>(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
	}
}

bool UDirectiveUtilStringFunctionLibrary::ContainsLetters(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsAlpha(Char))
		{
			return true;
		}
	}
	return false;
}

bool UDirectiveUtilStringFunctionLibrary::ContainsNumbers(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsDigit(Char))
		{
			return true;
		}
	}
	return false;
}

bool UDirectiveUtilStringFunctionLibrary::ContainsSpaces(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsWhitespace(Char))
		{
			return true;
		}
	}
	return false;
}

bool UDirectiveUtilStringFunctionLibrary::ContainsSpecialCharacters(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsPunct(Char))
		{
			return true;
		}
	}
	return false;
}

FString UDirectiveUtilStringFunctionLibrary::FilterCharacters(
	const FString& String,
	const bool bLetters,
	const bool bNumbers,
	const bool bSpecialCharacters,
	const bool bSpaces)
{
	FString NewString;
	NewString.Reserve(String.Len());

	for (const TCHAR& Char : String)
	{
		if (bLetters && FChar::IsAlpha(Char)) continue;
		if (bNumbers && FChar::IsDigit(Char)) continue;
		if (bSpecialCharacters && FChar::IsPunct(Char)) continue;
		if (bSpaces && FChar::IsWhitespace(Char)) continue;
		NewString.AppendChar(Char);
	}
	return NewString;
}

FString UDirectiveUtilStringFunctionLibrary::TruncateString(const FString& String, const int32 MaxLength, const FString& Suffix)
{
	if (String.Len() <= MaxLength)
	{
		return String;
	}
	return String.Left(FMath::Max(0, MaxLength - Suffix.Len())) + Suffix;
}

FString UDirectiveUtilStringFunctionLibrary::ToTitleCase(const FString& String)
{
	FString Result;
	Result.Reserve(String.Len());
	bool bCapitalizeNext = true;

	for (const TCHAR& Char : String)
	{
		if (FChar::IsWhitespace(Char))
		{
			bCapitalizeNext = true;
			Result.AppendChar(Char);
		}
		else if (bCapitalizeNext && FChar::IsAlpha(Char))
		{
			Result.AppendChar(FChar::ToUpper(Char));
			bCapitalizeNext = false;
		}
		else
		{
			Result.AppendChar(FChar::ToLower(Char));
			bCapitalizeNext = false;
		}
	}

	return Result;
}

TArray<FString> UDirectiveUtilStringFunctionLibrary::SplitIntoWords(const FString& String)
{
	TArray<FString> Words;
	FString CurrentWord;

	for (int32 Index = 0; Index < String.Len(); ++Index)
	{
		const TCHAR Char = String[Index];
		if (!FChar::IsAlnum(Char))
		{
			if (!CurrentWord.IsEmpty())
			{
				Words.Add(MoveTemp(CurrentWord));
				CurrentWord.Reset();
			}
			continue;
		}

		if (!CurrentWord.IsEmpty())
		{
			const TCHAR Previous = CurrentWord[CurrentWord.Len() - 1];
			const bool bNextIsLower = Index + 1 < String.Len() && FChar::IsLower(String[Index + 1]);
			const bool bBoundary =
				(FChar::IsLower(Previous) && FChar::IsUpper(Char)) ||
				(FChar::IsUpper(Previous) && FChar::IsUpper(Char) && bNextIsLower) ||
				(FChar::IsAlpha(Previous) && FChar::IsDigit(Char)) ||
				(FChar::IsDigit(Previous) && FChar::IsAlpha(Char));
			if (bBoundary)
			{
				Words.Add(MoveTemp(CurrentWord));
				CurrentWord.Reset();
			}
		}

		CurrentWord.AppendChar(Char);
	}

	if (!CurrentWord.IsEmpty())
	{
		Words.Add(MoveTemp(CurrentWord));
	}
	return Words;
}

FString UDirectiveUtilStringFunctionLibrary::ToCamelCase(const FString& String)
{
	const TArray<FString> Words = SplitIntoWords(String);
	FString Result;
	for (int32 Index = 0; Index < Words.Num(); ++Index)
	{
		FString Word = Words[Index].ToLower();
		if (Index > 0)
		{
			Word[0] = FChar::ToUpper(Word[0]);
		}
		Result += Word;
	}
	return Result;
}

FString UDirectiveUtilStringFunctionLibrary::ToPascalCase(const FString& String)
{
	FString Result;
	for (const FString& Word : SplitIntoWords(String))
	{
		FString Cased = Word.ToLower();
		Cased[0] = FChar::ToUpper(Cased[0]);
		Result += Cased;
	}
	return Result;
}

FString UDirectiveUtilStringFunctionLibrary::ToSnakeCase(const FString& String)
{
	return FString::Join(SplitIntoWords(String), TEXT("_")).ToLower();
}

FString UDirectiveUtilStringFunctionLibrary::ToKebabCase(const FString& String)
{
	return FString::Join(SplitIntoWords(String), TEXT("-")).ToLower();
}

TArray<FString> UDirectiveUtilStringFunctionLibrary::SortStringArray(TArray<FString> StringArray)
{
	Algo::Sort(StringArray);
	return StringArray;
}

TArray<FString> UDirectiveUtilStringFunctionLibrary::GetSortedStringArray(const TArray<FString> StringArray)
{
	TArray<FString> SortedArray = StringArray;
	Algo::Sort(SortedArray);
	return SortedArray;
}

int32 UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(const FString& A, const FString& B, const bool bCaseSensitive)
{
	const FString StringA = bCaseSensitive ? A : A.ToLower();
	const FString StringB = bCaseSensitive ? B : B.ToLower();
	const int32 LenA = StringA.Len();
	const int32 LenB = StringB.Len();

	if (LenA == 0) { return LenB; }
	if (LenB == 0) { return LenA; }

	TArray<int32> PreviousRow;
	TArray<int32> CurrentRow;
	PreviousRow.SetNumUninitialized(LenB + 1);
	CurrentRow.SetNumUninitialized(LenB + 1);

	for (int32 ColumnIndex = 0; ColumnIndex <= LenB; ++ColumnIndex)
	{
		PreviousRow[ColumnIndex] = ColumnIndex;
	}

	for (int32 RowIndex = 1; RowIndex <= LenA; ++RowIndex)
	{
		CurrentRow[0] = RowIndex;
		for (int32 ColumnIndex = 1; ColumnIndex <= LenB; ++ColumnIndex)
		{
			const int32 SubstitutionCost = (StringA[RowIndex - 1] == StringB[ColumnIndex - 1]) ? 0 : 1;
			CurrentRow[ColumnIndex] = FMath::Min3(
				PreviousRow[ColumnIndex] + 1,
				CurrentRow[ColumnIndex - 1] + 1,
				PreviousRow[ColumnIndex - 1] + SubstitutionCost);
		}
		Exchange(PreviousRow, CurrentRow);
	}

	return PreviousRow[LenB];
}

float UDirectiveUtilStringFunctionLibrary::GetStringSimilarity(const FString& A, const FString& B, const bool bCaseSensitive)
{
	const int32 MaxLength = FMath::Max(A.Len(), B.Len());
	if (MaxLength == 0)
	{
		return 1.0f;
	}
	const int32 Distance = GetLevenshteinDistance(A, B, bCaseSensitive);
	return 1.0f - (static_cast<float>(Distance) / static_cast<float>(MaxLength));
}

bool UDirectiveUtilStringFunctionLibrary::ContainsAny(const FString& Source, const TArray<FString>& SearchTerms, const bool bCaseSensitive)
{
	const ESearchCase::Type SearchCase = bCaseSensitive ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	for (const FString& Term : SearchTerms)
	{
		if (!Term.IsEmpty() && Source.Contains(Term, SearchCase))
		{
			return true;
		}
	}
	return false;
}

bool UDirectiveUtilStringFunctionLibrary::FindFirstOfAny(const FString& Source, const TArray<FString>& SearchTerms, const bool bCaseSensitive, int32& OutFoundIndex, int32& OutTermIndex)
{
	OutFoundIndex = INDEX_NONE;
	OutTermIndex = INDEX_NONE;

	const ESearchCase::Type SearchCase = bCaseSensitive ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	for (int32 TermIndex = 0; TermIndex < SearchTerms.Num(); ++TermIndex)
	{
		const FString& Term = SearchTerms[TermIndex];
		if (Term.IsEmpty())
		{
			continue;
		}

		const int32 FoundIndex = Source.Find(Term, SearchCase, ESearchDir::FromStart);
		if (FoundIndex != INDEX_NONE && (OutFoundIndex == INDEX_NONE || FoundIndex < OutFoundIndex))
		{
			OutFoundIndex = FoundIndex;
			OutTermIndex = TermIndex;
		}
	}

	return OutFoundIndex != INDEX_NONE;
}

FString UDirectiveUtilStringFunctionLibrary::Base64Encode(const FString& Source)
{
	return FBase64::Encode(Source);
}

bool UDirectiveUtilStringFunctionLibrary::Base64Decode(const FString& Source, FString& OutDecoded)
{
	OutDecoded.Reset();
	return FBase64::Decode(Source, OutDecoded);
}

FString UDirectiveUtilStringFunctionLibrary::HexEncode(const FString& String)
{
	return HexEncodeBytes(StringToUtf8Bytes(String));
}

bool UDirectiveUtilStringFunctionLibrary::HexDecode(const FString& Hex, FString& OutString)
{
	OutString.Reset();

	TArray<uint8> Bytes;
	if (!HexDecodeBytes(Hex, Bytes))
	{
		return false;
	}

	const FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()), Bytes.Num());
	OutString = FString(Converter.Length(), Converter.Get());
	return true;
}

FString UDirectiveUtilStringFunctionLibrary::HexEncodeBytes(const TArray<uint8>& Bytes)
{
	return BytesToHexLower(Bytes.GetData(), Bytes.Num());
}

bool UDirectiveUtilStringFunctionLibrary::HexDecodeBytes(const FString& Hex, TArray<uint8>& OutBytes)
{
	OutBytes.Reset();
	if (Hex.Len() % 2 != 0)
	{
		return false;
	}

	OutBytes.Reserve(Hex.Len() / 2);
	for (int32 Index = 0; Index < Hex.Len(); Index += 2)
	{
		const TCHAR High = Hex[Index];
		const TCHAR Low = Hex[Index + 1];
		if (!CheckTCharIsHex(High) || !CheckTCharIsHex(Low))
		{
			OutBytes.Reset();
			return false;
		}
		OutBytes.Add(static_cast<uint8>((TCharToNibble(High) << 4) | TCharToNibble(Low)));
	}
	return true;
}

FString UDirectiveUtilStringFunctionLibrary::Md5HashString(const FString& String)
{
	return Md5HashBytes(StringToUtf8Bytes(String));
}

FString UDirectiveUtilStringFunctionLibrary::Md5HashBytes(const TArray<uint8>& Bytes)
{
	return FMD5::HashBytes(Bytes.GetData(), Bytes.Num());
}

FString UDirectiveUtilStringFunctionLibrary::Sha1HashString(const FString& String)
{
	return Sha1HashBytes(StringToUtf8Bytes(String));
}

FString UDirectiveUtilStringFunctionLibrary::Sha1HashBytes(const TArray<uint8>& Bytes)
{
	uint8 Digest[20];
	FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Digest);
	return BytesToHexLower(Digest, UE_ARRAY_COUNT(Digest));
}

int32 UDirectiveUtilStringFunctionLibrary::Crc32String(const FString& String)
{
	return Crc32Bytes(StringToUtf8Bytes(String));
}

int32 UDirectiveUtilStringFunctionLibrary::Crc32Bytes(const TArray<uint8>& Bytes)
{
	return static_cast<int32>(FCrc::MemCrc32(Bytes.GetData(), Bytes.Num()));
}

bool UDirectiveUtilStringFunctionLibrary::IsValidFileName(const FString& String)
{
	return !String.IsEmpty() && FPaths::GetCleanFilename(String) == String && SanitizeFileName(String) == String;
}

FString UDirectiveUtilStringFunctionLibrary::SanitizeFileName(const FString& String, const FString& Replacement)
{
	return FPaths::MakeValidFileName(String, Replacement.IsEmpty() ? TEXT('\0') : Replacement[0]);
}

int32 UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(const FString& Input, const TArray<FString>& Candidates, float& OutSimilarity, const bool bCaseSensitive)
{
	OutSimilarity = 0.0f;
	int32 BestIndex = INDEX_NONE;

	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		const float Similarity = GetStringSimilarity(Input, Candidates[Index], bCaseSensitive);
		if (BestIndex == INDEX_NONE || Similarity > OutSimilarity)
		{
			BestIndex = Index;
			OutSimilarity = Similarity;
		}
	}
	return BestIndex;
}
