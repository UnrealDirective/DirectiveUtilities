// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilConfigFunctionLibrary.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "DirectiveUtilRuntimeHelpers.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

#include <limits>

namespace
{
	bool ContainsEmbeddedNull(const FString& Value)
	{
		return Value.Len() != FCString::Strlen(*Value);
	}

	bool ContainsBraceOrQuote(const FString& Name)
	{
		return Name.Contains(TEXT("{")) || Name.Contains(TEXT("}")) || Name.Contains(TEXT("\""));
	}

	bool IsValidSectionName(const FString& SectionName)
	{
		return !SectionName.IsEmpty()
			&& SectionName.Len() < NAME_SIZE
			&& !ContainsEmbeddedNull(SectionName)
			&& SectionName == SectionName.TrimStartAndEnd()
			&& !SectionName.Contains(TEXT("["))
			&& !SectionName.Contains(TEXT("]"))
			&& !ContainsBraceOrQuote(SectionName)
			&& !SectionName.Contains(TEXT("//"))
			&& !SectionName.Contains(TEXT("\r"))
			&& !SectionName.Contains(TEXT("\n"));
	}

	// Engine config layers read these leading characters as array and removal commands rather than as part of the key.
	bool IsConfigCommandPrefix(const TCHAR Character)
	{
		return Character == TEXT('+') || Character == TEXT('-') || Character == TEXT('.') || Character == TEXT('!')
			|| Character == TEXT('@') || Character == TEXT('*') || Character == TEXT('^');
	}

	bool IsValidKeyName(const FString& KeyName)
	{
		return !KeyName.IsEmpty()
			&& KeyName.Len() < NAME_SIZE
			&& !ContainsEmbeddedNull(KeyName)
			&& KeyName == KeyName.TrimStartAndEnd()
			&& KeyName[0] != TEXT(';')
			&& KeyName[0] != TEXT('~')
			&& KeyName[0] != TEXT('[')
			&& !IsConfigCommandPrefix(KeyName[0])
			&& !ContainsBraceOrQuote(KeyName)
			&& !KeyName.Contains(TEXT("="))
			&& !KeyName.Contains(TEXT("//"))
			&& !KeyName.Contains(TEXT("\r"))
			&& !KeyName.Contains(TEXT("\n"));
	}

	// Mirrors FParse::LineExtended, which joins a line ending in a backslash or holding an unclosed unquoted brace
	// with the next line, and drops unquoted braces from the text it returns.
	bool IsChangedByLineParser(const FString& Line)
	{
		if (Line.EndsWith(TEXT("\\")))
		{
			return true;
		}
		const bool bIsCommentLine = Line.StartsWith(TEXT(";"));
		bool bIsQuoted = false;
		bool bIsComment = false;
		int32 BracketDepth = 0;
		for (int32 Index = 0; Index < Line.Len(); ++Index)
		{
			const TCHAR Character = Line[Index];
			const TCHAR NextCharacter = Index + 1 < Line.Len() ? Line[Index + 1] : TEXT('\0');
			if (!bIsQuoted && Character == TEXT('/') && NextCharacter == TEXT('/'))
			{
				bIsComment = true;
			}
			if (!bIsQuoted && !bIsComment && !bIsCommentLine && (Character == TEXT('{') || Character == TEXT('}')))
			{
				return true;
			}
			if (!bIsQuoted && Character == TEXT('{'))
			{
				++BracketDepth;
			}
			else if (!bIsQuoted && Character == TEXT('}') && BracketDepth > 0)
			{
				--BracketDepth;
			}
			else if (bIsQuoted && !bIsComment && Character == TEXT('\\')
				&& (NextCharacter == TEXT('"') || NextCharacter == TEXT('\\')))
			{
				++Index;
			}
			else if (Character == TEXT('"'))
			{
				bIsQuoted = !bIsQuoted;
			}
		}
		return BracketDepth > 0;
	}

	bool IsValidConfigText(const FString& Contents)
	{
		if (ContainsEmbeddedNull(Contents))
		{
			return false;
		}
		TArray<FString> Lines;
		Contents.ParseIntoArrayLines(Lines, false);
		bool bHasSection = false;
		for (const FString& Line : Lines)
		{
			if (IsChangedByLineParser(Line))
			{
				return false;
			}
			const FString Trimmed = Line.TrimStartAndEnd();
			// The engine treats ';' as a comment only in the first column; an indented ';' line with '=' is a key.
			const bool bIsComment = Line.StartsWith(TEXT(";")) || Trimmed.StartsWith(TEXT("//"))
				|| (Trimmed.StartsWith(TEXT(";")) && !Trimmed.Contains(TEXT("=")));
			if (Trimmed.IsEmpty() || bIsComment)
			{
				continue;
			}
			if (Trimmed.StartsWith(TEXT("[")) && Trimmed.EndsWith(TEXT("]")))
			{
				bHasSection = Line.StartsWith(TEXT("[")) && IsValidSectionName(Trimmed.Mid(1, Trimmed.Len() - 2));
				if (!bHasSection)
				{
					return false;
				}
				continue;
			}
			int32 EqualsIndex = INDEX_NONE;
			if (!bHasSection || !Line.FindChar(TEXT('='), EqualsIndex)
				|| !IsValidKeyName(Line.Left(EqualsIndex).TrimStartAndEnd()))
			{
				return false;
			}

			const FString ValueText = Line.Mid(EqualsIndex + 1).TrimStartAndEnd();
			if (ValueText.StartsWith(TEXT("\"")))
			{
				FString ParsedValue;
				int32 CharactersRead = 0;
				if (!FParse::QuotedString(*ValueText, ParsedValue, &CharactersRead))
				{
					return false;
				}
				const FString Remainder = ValueText.Mid(CharactersRead).TrimStartAndEnd();
				if (!Remainder.IsEmpty() && !Remainder.StartsWith(TEXT("//")))
				{
					return false;
				}
			}
		}
		return true;
	}

	bool ValidateConfigArguments(const FString& FilePath, const FString& SectionName, const FString& KeyName, FString& OutResolvedPath)
	{
		if (FilePath.IsEmpty() || ContainsEmbeddedNull(FilePath)
			|| !IsValidSectionName(SectionName) || !IsValidKeyName(KeyName))
		{
			return false;
		}
		OutResolvedPath = DirectiveUtil::ResolveRuntimePath(FilePath);
		return true;
	}

	bool ValidateConfigPathAndSection(const FString& FilePath, const FString& SectionName, FString& OutResolvedPath)
	{
		if (FilePath.IsEmpty() || ContainsEmbeddedNull(FilePath) || !IsValidSectionName(SectionName))
		{
			return false;
		}
		OutResolvedPath = DirectiveUtil::ResolveRuntimePath(FilePath);
		return true;
	}

	bool LoadConfigFile(const FString& ResolvedPath, FConfigFile& OutFile)
	{
		if (!IFileManager::Get().FileExists(*ResolvedPath))
		{
			return false;
		}
		FString Contents;
		if (IFileManager::Get().FileSize(*ResolvedPath) >= MAX_int32
			|| !FFileHelper::LoadFileToString(Contents, *ResolvedPath)
			|| !IsValidConfigText(Contents))
		{
			return false;
		}
		OutFile.ProcessInputFileContents(Contents, TEXT("DirectiveUtilities.Config"));
		return true;
	}

	bool FindSectionNameCaseInsensitive(FConfigFile& File, const FString& SectionName, FString& OutSectionName)
	{
		for (const TPair<FString, FConfigSection>& SectionPair : AsConst(File))
		{
			if (SectionPair.Key.Equals(SectionName, ESearchCase::IgnoreCase))
			{
				OutSectionName = SectionPair.Key;
				return true;
			}
		}
		return false;
	}

	bool LoadConfigSection(const FString& ResolvedPath, const FString& SectionName, FConfigFile& OutFile, FString& OutSectionName)
	{
		return LoadConfigFile(ResolvedPath, OutFile)
			&& FindSectionNameCaseInsensitive(OutFile, SectionName, OutSectionName);
	}

	bool CreateParentDirectories(const FString& ResolvedPath)
	{
		const FString Directory = FPaths::GetPath(ResolvedPath);
		if (Directory.IsEmpty() || IFileManager::Get().DirectoryExists(*Directory))
		{
			return true;
		}
		return IFileManager::Get().MakeDirectory(*Directory, /*Tree*/ true);
	}

	bool ShouldQuoteConfigValue(const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return false;
		}
		if (FChar::IsWhitespace(Value[0]) || FChar::IsWhitespace(Value[Value.Len() - 1]) || Value.EndsWith(TEXT("\\")))
		{
			return true;
		}
		for (const TCHAR Character : Value)
		{
			if (Character == TEXT('"') || Character == TEXT('{') || Character == TEXT('}')
				|| Character == TEXT('\r') || Character == TEXT('\n'))
			{
				return true;
			}
		}
		return Value.Contains(TEXT("//"));
	}

	void AppendConfigLine(FString& Contents, const FString& KeyName, const FString& Value)
	{
		Contents += KeyName;
		Contents += TEXT('=');
		if (ShouldQuoteConfigValue(Value))
		{
			Contents += TEXT('"');
			Contents += Value.ReplaceCharWithEscapedChar();
			Contents += TEXT('"');
		}
		else
		{
			Contents += Value;
		}
		Contents += LINE_TERMINATOR;
	}

	// FConfigFile::WriteToString drops empty sections and leaves edge tabs unquoted, so the file text is built here.
	FString SerializeConfigFile(const FConfigFile& File)
	{
		FString Contents;
		for (const TPair<FString, FConfigSection>& SectionPair : File)
		{
			Contents += TEXT('[');
			Contents += SectionPair.Key;
			Contents += TEXT(']');
			Contents += LINE_TERMINATOR;

			TSet<FName> WrittenKeys;
			for (const TPair<FName, FConfigValue>& Entry : SectionPair.Value)
			{
				bool bAlreadyWritten = false;
				WrittenKeys.Add(Entry.Key, &bAlreadyWritten);
				if (bAlreadyWritten)
				{
					continue;
				}
				const FString KeyName = Entry.Key.ToString();
				TArray<const FConfigValue*> Values;
				SectionPair.Value.MultiFindPointer(Entry.Key, Values, true);
				for (const FConfigValue* Value : Values)
				{
					AppendConfigLine(Contents, KeyName, Value->GetSavedValueForWriting());
				}
			}
			Contents += LINE_TERMINATOR;
		}
		return Contents;
	}

	bool SaveConfigFile(const FConfigFile& File, const FString& ResolvedPath)
	{
		return UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(
			ResolvedPath, SerializeConfigFile(File), true, true);
	}

	const FConfigSection* FindStoredSection(const FString& FilePath, const FString& SectionName, const FString& KeyName, FConfigFile& OutFile)
	{
		FString ResolvedPath;
		FString StoredSectionName;
		if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath)
			|| !LoadConfigSection(ResolvedPath, SectionName, OutFile, StoredSectionName))
		{
			return nullptr;
		}
		return OutFile.FindSection(StoredSectionName);
	}

	// FConfigValue::GetValue expands %TOKEN% macros into a fixed seven-entry buffer that four macros overflow, so reads use the saved text.
	bool ReadStoredValue(const FString& FilePath, const FString& SectionName, const FString& KeyName, FString& OutValue)
	{
		FConfigFile File;
		const FConfigSection* Section = FindStoredSection(FilePath, SectionName, KeyName, File);
		const FConfigValue* Value = Section != nullptr ? Section->Find(FName(*KeyName)) : nullptr;
		if (Value == nullptr)
		{
			return false;
		}
		OutValue = Value->GetSavedValue();
		return true;
	}

	bool ReadNumericComponent(const FString& Text, const TCHAR* Component, double& OutValue, const bool bOptional = false)
	{
		FString Token;
		if (!FParse::Value(*Text, Component, Token))
		{
			return bOptional;
		}
		const TCHAR* Cursor = *Token;
		if (*Cursor == TEXT('-') || *Cursor == TEXT('+'))
		{
			++Cursor;
		}
		auto ConsumeDigits = [&Cursor]()
		{
			int32 Count = 0;
			while (*Cursor >= TEXT('0') && *Cursor <= TEXT('9'))
			{
				++Cursor;
				++Count;
			}
			return Count;
		};
		int32 Digits = ConsumeDigits();
		if (*Cursor == TEXT('.'))
		{
			++Cursor;
			Digits += ConsumeDigits();
		}
		if (Digits == 0)
		{
			return false;
		}
		if (*Cursor == TEXT('e') || *Cursor == TEXT('E'))
		{
			++Cursor;
			if (*Cursor == TEXT('-') || *Cursor == TEXT('+'))
			{
				++Cursor;
			}
			if (ConsumeDigits() == 0)
			{
				return false;
			}
		}
		if (*Cursor != TEXT('\0'))
		{
			return false;
		}
		const double Value = FCString::Atod(*Token);
		if (!FMath::IsFinite(Value))
		{
			return false;
		}
		OutValue = Value;
		return true;
	}

	bool ReadColorComponent(const FString& Text, const TCHAR* Component, uint8& OutValue, const bool bOptional = false)
	{
		double Value = OutValue;
		if (!ReadNumericComponent(Text, Component, Value, bOptional)
			|| Value < 0.0 || Value > MAX_uint8 || Value != FMath::FloorToDouble(Value))
		{
			return false;
		}
		OutValue = static_cast<uint8>(Value);
		return true;
	}

	void SetScalarConfigValue(FConfigFile& File, const FString& SectionName, const FString& KeyName, const FString& Value)
	{
		File.RemoveKeyFromSection(*SectionName, FName(*KeyName));
		File.SetString(*SectionName, *KeyName, *Value);
	}

	bool WriteConfigValues(const FString& FilePath, const FString& SectionName, const FString& KeyName,
		const TFunctionRef<void(FConfigFile&, const FString&)>& ApplyValue)
	{
		FString ResolvedPath;
		if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
		{
			return false;
		}
		if (!CreateParentDirectories(ResolvedPath))
		{
			return false;
		}

		FConfigFile File;
		if (IFileManager::Get().FileExists(*ResolvedPath) && !LoadConfigFile(ResolvedPath, File))
		{
			return false;
		}
		FString StoredSectionName = SectionName;
		FindSectionNameCaseInsensitive(File, SectionName, StoredSectionName);
		ApplyValue(File, StoredSectionName);
		return SaveConfigFile(File, ResolvedPath);
	}
}

FString UDirectiveUtilConfigFunctionLibrary::ReadConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& DefaultValue)
{
	FString Value;
	return ReadStoredValue(FilePath, SectionName, KeyName, Value) ? Value : DefaultValue;
}

int32 UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 DefaultValue)
{
	FString Value;
	return ReadStoredValue(FilePath, SectionName, KeyName, Value) ? FCString::Atoi(*Value) : DefaultValue;
}

int64 UDirectiveUtilConfigFunctionLibrary::ReadConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 DefaultValue)
{
	FString Value;
	return ReadStoredValue(FilePath, SectionName, KeyName, Value) ? FCString::Atoi64(*Value) : DefaultValue;
}

float UDirectiveUtilConfigFunctionLibrary::ReadConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float DefaultValue)
{
	FString Value;
	return ReadStoredValue(FilePath, SectionName, KeyName, Value) ? FCString::Atof(*Value) : DefaultValue;
}

bool UDirectiveUtilConfigFunctionLibrary::ReadConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool DefaultValue)
{
	FString Value;
	return ReadStoredValue(FilePath, SectionName, KeyName, Value) ? FCString::ToBool(*Value) : DefaultValue;
}

bool UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, TArray<FString>& OutValues)
{
	OutValues.Reset();
	FConfigFile File;
	const FConfigSection* Section = FindStoredSection(FilePath, SectionName, KeyName, File);
	if (Section == nullptr)
	{
		return false;
	}

	TArray<const FConfigValue*> StoredValues;
	Section->MultiFindPointer(FName(*KeyName), StoredValues, true);
	for (const FConfigValue* StoredValue : StoredValues)
	{
		OutValues.Add(StoredValue->GetSavedValue());
	}
	return !OutValues.IsEmpty();
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& Value)
{
	if (ContainsEmbeddedNull(Value))
	{
		return false;
	}
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		SetScalarConfigValue(File, StoredSectionName, KeyName, Value);
	});
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 Value)
{
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		SetScalarConfigValue(File, StoredSectionName, KeyName, LexToString(Value));
	});
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 Value)
{
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		SetScalarConfigValue(File, StoredSectionName, KeyName, LexToString(Value));
	});
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float Value)
{
	if (!FMath::IsFinite(Value))
	{
		return false;
	}
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		SetScalarConfigValue(
			File,
			StoredSectionName,
			KeyName,
			FString::Printf(TEXT("%.*g"), std::numeric_limits<float>::max_digits10, Value));
	});
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool Value)
{
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		SetScalarConfigValue(File, StoredSectionName, KeyName, Value ? TEXT("True") : TEXT("False"));
	});
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, const TArray<FString>& Values)
{
	if (Values.ContainsByPredicate([](const FString& Value)
	{
		return ContainsEmbeddedNull(Value);
	}))
	{
		return false;
	}
	return WriteConfigValues(FilePath, SectionName, KeyName, [&](FConfigFile& File, const FString& StoredSectionName)
	{
		File.SetArray(*StoredSectionName, *KeyName, Values);
	});
}

bool UDirectiveUtilConfigFunctionLibrary::RemoveConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName))
	{
		return false;
	}
	if (!File.RemoveKeyFromSection(*StoredSectionName, FName(*KeyName)))
	{
		return false;
	}
	return SaveConfigFile(File, ResolvedPath);
}

bool UDirectiveUtilConfigFunctionLibrary::ClearConfigSection(const FString& FilePath, const FString& SectionName)
{
	FString ResolvedPath;
	if (!ValidateConfigPathAndSection(FilePath, SectionName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName))
	{
		return false;
	}
	File.Remove(StoredSectionName);
	return SaveConfigFile(File, ResolvedPath);
}

bool UDirectiveUtilConfigFunctionLibrary::HasConfigSection(const FString& FilePath, const FString& SectionName)
{
	FString ResolvedPath;
	if (!ValidateConfigPathAndSection(FilePath, SectionName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	return LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName);
}

bool UDirectiveUtilConfigFunctionLibrary::HasConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName))
	{
		return false;
	}
	const FConfigSection* Section = File.FindSection(StoredSectionName);
	return Section != nullptr && Section->Find(FName(*KeyName)) != nullptr;
}

FVector2D UDirectiveUtilConfigFunctionLibrary::ReadConfigVector2D(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FVector2D& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FVector2D Value;
	return ReadNumericComponent(Stored, TEXT("X="), Value.X)
		&& ReadNumericComponent(Stored, TEXT("Y="), Value.Y) ? Value : DefaultValue;
}

FVector UDirectiveUtilConfigFunctionLibrary::ReadConfigVector(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FVector& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FVector Value;
	return ReadNumericComponent(Stored, TEXT("X="), Value.X)
		&& ReadNumericComponent(Stored, TEXT("Y="), Value.Y)
		&& ReadNumericComponent(Stored, TEXT("Z="), Value.Z) ? Value : DefaultValue;
}

FRotator UDirectiveUtilConfigFunctionLibrary::ReadConfigRotator(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FRotator& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FRotator Value;
	return ReadNumericComponent(Stored, TEXT("P="), Value.Pitch)
		&& ReadNumericComponent(Stored, TEXT("Y="), Value.Yaw)
		&& ReadNumericComponent(Stored, TEXT("R="), Value.Roll) ? Value : DefaultValue;
}

FColor UDirectiveUtilConfigFunctionLibrary::ReadConfigColor(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FColor& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FColor Value(0, 0, 0, MAX_uint8);
	return ReadColorComponent(Stored, TEXT("R="), Value.R)
		&& ReadColorComponent(Stored, TEXT("G="), Value.G)
		&& ReadColorComponent(Stored, TEXT("B="), Value.B)
		&& ReadColorComponent(Stored, TEXT("A="), Value.A, true) ? Value : DefaultValue;
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigVector2D(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FVector2D& Value)
{
	return !Value.ContainsNaN() && WriteConfigString(FilePath, SectionName, KeyName, Value.ToString());
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigVector(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FVector& Value)
{
	return !Value.ContainsNaN() && WriteConfigString(FilePath, SectionName, KeyName, Value.ToString());
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigRotator(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FRotator& Value)
{
	return !Value.ContainsNaN() && WriteConfigString(FilePath, SectionName, KeyName, Value.ToString());
}

bool UDirectiveUtilConfigFunctionLibrary::WriteConfigColor(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FColor& Value)
{
	return WriteConfigString(FilePath, SectionName, KeyName, Value.ToString());
}

bool UDirectiveUtilConfigFunctionLibrary::GetConfigSectionNames(const FString& FilePath, TArray<FString>& OutSectionNames)
{
	OutSectionNames.Reset();
	if (FilePath.IsEmpty() || ContainsEmbeddedNull(FilePath))
	{
		return false;
	}

	const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(FilePath);
	FConfigFile File;
	if (!LoadConfigFile(ResolvedPath, File))
	{
		return false;
	}

	for (const TPair<FString, FConfigSection>& SectionPair : AsConst(File))
	{
		OutSectionNames.Add(SectionPair.Key);
	}
	OutSectionNames.Sort([](const FString& Left, const FString& Right)
	{
		const int32 CaseInsensitiveOrder = Left.Compare(Right, ESearchCase::IgnoreCase);
		return CaseInsensitiveOrder == 0
			? Left.Compare(Right, ESearchCase::CaseSensitive) < 0
			: CaseInsensitiveOrder < 0;
	});
	return true;
}

bool UDirectiveUtilConfigFunctionLibrary::GetConfigKeysInSection(const FString& FilePath, const FString& SectionName, TArray<FString>& OutKeyNames)
{
	OutKeyNames.Reset();
	FString ResolvedPath;
	if (!ValidateConfigPathAndSection(FilePath, SectionName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName))
	{
		return false;
	}
	const FConfigSection* Section = File.FindSection(StoredSectionName);
	if (Section == nullptr)
	{
		return false;
	}

	TSet<FName> SeenKeys;
	for (const TPair<FName, FConfigValue>& Entry : AsConst(*Section))
	{
		bool bAlreadySeen = false;
		SeenKeys.Add(Entry.Key, &bAlreadySeen);
		if (!bAlreadySeen)
		{
			OutKeyNames.Add(Entry.Key.ToString());
		}
	}
	OutKeyNames.Sort([](const FString& Left, const FString& Right)
	{
		const int32 CaseInsensitiveOrder = Left.Compare(Right, ESearchCase::IgnoreCase);
		return CaseInsensitiveOrder == 0
			? Left.Compare(Right, ESearchCase::CaseSensitive) < 0
			: CaseInsensitiveOrder < 0;
	});
	return true;
}
