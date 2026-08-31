// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilDateTimeFunctionLibrary.h"
#include "DirectiveUtilRuntimeHelpers.h"

namespace
{
	enum class EDirectiveDateTimeToken : uint8
	{
		Literal,
		Year,
		Month,
		Day,
		Hour,
		Minute,
		Second,
		Millisecond
	};

	struct FDirectivePatternPiece
	{
		EDirectiveDateTimeToken Token = EDirectiveDateTimeToken::Literal;
		FString Literal;
		int32 Width = 0;
	};

	bool IsTokenLetter(TCHAR Character)
	{
		return Character == TEXT('y') || Character == TEXT('M') || Character == TEXT('d')
			|| Character == TEXT('H') || Character == TEXT('m') || Character == TEXT('s')
			|| Character == TEXT('f');
	}

	bool TokenForLetter(TCHAR Letter, int32 RunLength, EDirectiveDateTimeToken& OutToken, int32& OutWidth)
	{
		switch (Letter)
		{
			case TEXT('y'):
				if (RunLength == 4) { OutToken = EDirectiveDateTimeToken::Year; OutWidth = 4; return true; }
				return false;
			case TEXT('M'):
			case TEXT('d'):
			case TEXT('H'):
			case TEXT('m'):
			case TEXT('s'):
				if (RunLength == 2)
				{
					switch (Letter)
					{
						case TEXT('M'): OutToken = EDirectiveDateTimeToken::Month; break;
						case TEXT('d'): OutToken = EDirectiveDateTimeToken::Day; break;
						case TEXT('H'): OutToken = EDirectiveDateTimeToken::Hour; break;
						case TEXT('m'): OutToken = EDirectiveDateTimeToken::Minute; break;
						default: OutToken = EDirectiveDateTimeToken::Second; break;
					}
					OutWidth = 2;
					return true;
				}
				return false;
			case TEXT('f'):
				if (RunLength == 3) { OutToken = EDirectiveDateTimeToken::Millisecond; OutWidth = 3; return true; }
				return false;
			default:
				return false;
		}
	}

	TCHAR LetterForToken(const EDirectiveDateTimeToken Token)
	{
		switch (Token)
		{
			case EDirectiveDateTimeToken::Year: return TEXT('y');
			case EDirectiveDateTimeToken::Month: return TEXT('M');
			case EDirectiveDateTimeToken::Day: return TEXT('d');
			case EDirectiveDateTimeToken::Hour: return TEXT('H');
			case EDirectiveDateTimeToken::Minute: return TEXT('m');
			case EDirectiveDateTimeToken::Second: return TEXT('s');
			case EDirectiveDateTimeToken::Millisecond: return TEXT('f');
			case EDirectiveDateTimeToken::Literal:
			default: return TEXT('\0');
		}
	}

	bool CompilePattern(const FString& Pattern, TArray<FDirectivePatternPiece>& OutPieces)
	{
		OutPieces.Reset();
		const int32 Length = Pattern.Len();
		int32 Index = 0;

		while (Index < Length)
		{
			const TCHAR Character = Pattern[Index];

			const bool bIsTokenLetter = IsTokenLetter(Character);

			FString Literal;
			if (!bIsTokenLetter)
			{
				if (Character == TEXT('\''))
				{
					if (Index + 1 < Length && Pattern[Index + 1] == TEXT('\''))
					{
						Literal = TEXT("'");
						Index += 2;
					}
					else
					{
						++Index;
						bool bClosed = false;
						while (Index < Length)
						{
							if (Pattern[Index] == TEXT('\''))
							{
								if (Index + 1 < Length && Pattern[Index + 1] == TEXT('\''))
								{
									Literal += TEXT('\'');
									Index += 2;
									continue;
								}
								++Index;
								bClosed = true;
								break;
							}
							Literal += Pattern[Index];
							++Index;
						}
						if (!bClosed)
						{
							return false;
						}
					}
				}
				else
				{
					int32 RunEnd = Index + 1;
					while (RunEnd < Length && !IsTokenLetter(Pattern[RunEnd]) && Pattern[RunEnd] != TEXT('\''))
					{
						++RunEnd;
					}
					Literal = Pattern.Mid(Index, RunEnd - Index);
					Index = RunEnd;
				}

				FDirectivePatternPiece Piece;
				Piece.Token = EDirectiveDateTimeToken::Literal;
				Piece.Literal = MoveTemp(Literal);
				OutPieces.Add(MoveTemp(Piece));
				continue;
			}

			int32 RunEnd = Index + 1;
			while (RunEnd < Length && Pattern[RunEnd] == Character)
			{
				++RunEnd;
			}
			const int32 RunLength = RunEnd - Index;

			EDirectiveDateTimeToken Token = EDirectiveDateTimeToken::Literal;
			int32 Width = 0;
			if (!TokenForLetter(Character, RunLength, Token, Width))
			{
				return false;
			}

			FDirectivePatternPiece Piece;
			Piece.Token = Token;
			Piece.Width = Width;
			OutPieces.Add(Piece);
			Index = RunEnd;
		}
		return true;
	}

	void AppendNumber(FString& OutText, const int64 Value, const int32 Width)
	{
		FString Digits = LexToString(Value);
		while (Digits.Len() < Width)
		{
			Digits = FString(TEXT("0")) + Digits;
		}
		OutText += Digits;
	}

	bool ReadDigits(const FString& Text, int32& CursorPosition, const int32 Width, int64& OutValue)
	{
		int64 Value = 0;
		for (int32 DigitIndex = 0; DigitIndex < Width; ++DigitIndex)
		{
			if (!Text.IsValidIndex(CursorPosition))
			{
				return false;
			}
			const TCHAR Character = Text[CursorPosition];
			if (Character < TEXT('0') || Character > TEXT('9'))
			{
				return false;
			}
			Value = Value * 10 + (Character - TEXT('0'));
			++CursorPosition;
		}
		OutValue = Value;
		return true;
	}
}

bool UDirectiveUtilDateTimeFunctionLibrary::FormatDateTime(const FDateTime& DateTime, const FString& Pattern, FString& OutText)
{
	TArray<FDirectivePatternPiece> Pieces;
	if (!CompilePattern(Pattern, Pieces))
	{
		OutText.Reset();
		return false;
	}

	FString FormattedText;
	for (const FDirectivePatternPiece& Piece : Pieces)
	{
		if (Piece.Token == EDirectiveDateTimeToken::Literal)
		{
			FormattedText += Piece.Literal;
			continue;
		}

		int64 Value = 0;
		switch (Piece.Token)
		{
			case EDirectiveDateTimeToken::Year: Value = DateTime.GetYear(); break;
			case EDirectiveDateTimeToken::Month: Value = DateTime.GetMonth(); break;
			case EDirectiveDateTimeToken::Day: Value = DateTime.GetDay(); break;
			case EDirectiveDateTimeToken::Hour: Value = DateTime.GetHour(); break;
			case EDirectiveDateTimeToken::Minute: Value = DateTime.GetMinute(); break;
			case EDirectiveDateTimeToken::Second: Value = DateTime.GetSecond(); break;
			case EDirectiveDateTimeToken::Millisecond: Value = DateTime.GetMillisecond(); break;
			default: checkNoEntry();
		}
		AppendNumber(FormattedText, Value, Piece.Width);
	}
	OutText = MoveTemp(FormattedText);
	return true;
}

bool UDirectiveUtilDateTimeFunctionLibrary::TryParseDateTime(const FString& Text, const FString& Pattern, FDateTime& OutDateTime)
{
	OutDateTime = FDateTime();

	TArray<FDirectivePatternPiece> Pieces;
	if (!CompilePattern(Pattern, Pieces))
	{
		return false;
	}

	int64 Year = 1;
	int64 Month = 1;
	int64 Day = 1;
	int64 Hour = 0;
	int64 Minute = 0;
	int64 Second = 0;
	int64 Millisecond = 0;
	bool bTokenSeen[8] = {};
	int64 TokenValues[8] = {};

	int32 Cursor = 0;
	for (const FDirectivePatternPiece& Piece : Pieces)
	{
		if (Piece.Token == EDirectiveDateTimeToken::Literal)
		{
			if (Text.Mid(Cursor, Piece.Literal.Len()) != Piece.Literal)
			{
				return false;
			}
			Cursor += Piece.Literal.Len();
			continue;
		}

		int64 Value = 0;
		if (!ReadDigits(Text, Cursor, Piece.Width, Value))
		{
			return false;
		}
		const uint8 TokenIndex = static_cast<uint8>(Piece.Token);
		if (bTokenSeen[TokenIndex] && TokenValues[TokenIndex] != Value)
		{
			return false;
		}
		bTokenSeen[TokenIndex] = true;
		TokenValues[TokenIndex] = Value;
		switch (Piece.Token)
		{
			case EDirectiveDateTimeToken::Year: Year = Value; break;
			case EDirectiveDateTimeToken::Month: Month = Value; break;
			case EDirectiveDateTimeToken::Day: Day = Value; break;
			case EDirectiveDateTimeToken::Hour: Hour = Value; break;
			case EDirectiveDateTimeToken::Minute: Minute = Value; break;
			case EDirectiveDateTimeToken::Second: Second = Value; break;
			case EDirectiveDateTimeToken::Millisecond: Millisecond = Value; break;
			default: checkNoEntry();
		}
	}

	if (Cursor != Text.Len())
	{
		return false;
	}
	if (Year < 1 || Year > 9999 || Month < 1 || Month > 12 || Day < 1 || Hour < 0 || Hour > 23
		|| Minute < 0 || Minute > 59 || Second < 0 || Second > 59 || Millisecond < 0 || Millisecond > 999)
	{
		return false;
	}

	const int32 DaysInMonth = FDateTime::DaysInMonth(static_cast<int32>(Year), static_cast<int32>(Month));
	if (Day > DaysInMonth)
	{
		return false;
	}
	OutDateTime = FDateTime(
		static_cast<int32>(Year),
		static_cast<int32>(Month),
		static_cast<int32>(Day),
		static_cast<int32>(Hour),
		static_cast<int32>(Minute),
		static_cast<int32>(Second),
		static_cast<int32>(Millisecond));
	return true;
}

FDateTime UDirectiveUtilDateTimeFunctionLibrary::ToLocalTime(const FDateTime& UtcDateTime)
{
	return DirectiveUtil::UtcToLocal(UtcDateTime);
}

FDateTime UDirectiveUtilDateTimeFunctionLibrary::ToUtcTime(const FDateTime& LocalDateTime)
{
	return DirectiveUtil::LocalToUtc(LocalDateTime);
}

FString UDirectiveUtilDateTimeFunctionLibrary::FormatTimeSpan(const FTimespan& TimeSpan, const bool bIncludeMilliseconds)
{
	FString Text;
	const int64 TotalTicks = TimeSpan.GetTicks();
	if (TotalTicks < 0)
	{
		Text += TEXT('-');
	}

	const uint64 MagnitudeTicks = TotalTicks < 0
		? static_cast<uint64>(-(TotalTicks + 1)) + 1
		: static_cast<uint64>(TotalTicks);
	const uint64 Days = MagnitudeTicks / ETimespan::TicksPerDay;
	const uint64 Hours = (MagnitudeTicks / ETimespan::TicksPerHour) % 24;
	const uint64 Minutes = (MagnitudeTicks / ETimespan::TicksPerMinute) % 60;
	const uint64 Seconds = (MagnitudeTicks / ETimespan::TicksPerSecond) % 60;

	if (Days > 0)
	{
		Text += LexToString(Days);
		Text += TEXT('.');
	}
	AppendNumber(Text, static_cast<int64>(Hours), 2);
	Text += TEXT(':');
	AppendNumber(Text, static_cast<int64>(Minutes), 2);
	Text += TEXT(':');
	AppendNumber(Text, static_cast<int64>(Seconds), 2);

	if (bIncludeMilliseconds)
	{
		Text += TEXT('.');
		const uint64 Milliseconds = (MagnitudeTicks / ETimespan::TicksPerMillisecond) % 1000;
		AppendNumber(Text, static_cast<int64>(Milliseconds), 3);
	}
	return Text;
}
