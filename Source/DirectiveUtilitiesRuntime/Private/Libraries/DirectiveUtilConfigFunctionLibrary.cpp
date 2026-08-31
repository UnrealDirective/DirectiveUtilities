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

	bool IsValidSectionName(const FString& SectionName)
	{
		return !SectionName.IsEmpty()
			&& SectionName.Len() < NAME_SIZE
			&& !ContainsEmbeddedNull(SectionName)
			&& SectionName == SectionName.TrimStartAndEnd()
			&& !SectionName.Contains(TEXT("["))
			&& !SectionName.Contains(TEXT("]"))
			&& !SectionName.Contains(TEXT("//"))
			&& !SectionName.Contains(TEXT("\r"))
			&& !SectionName.Contains(TEXT("\n"));
	}

	bool IsValidKeyName(const FString& KeyName)
	{
		return !KeyName.IsEmpty()
			&& KeyName.Len() < NAME_SIZE
			&& !ContainsEmbeddedNull(KeyName)
			&& KeyName == KeyName.TrimStartAndEnd()
			&& KeyName[0] != TEXT(';')
			&& KeyName[0] != TEXT('~')
			&& !KeyName.Contains(TEXT("="))
			&& !KeyName.Contains(TEXT("//"))
			&& !KeyName.Contains(TEXT("\r"))
			&& !KeyName.Contains(TEXT("\n"));
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
			const FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.IsEmpty() || Trimmed.StartsWith(TEXT(";")) || Trimmed.StartsWith(TEXT("//")))
			{
				continue;
			}
			if (Trimmed.StartsWith(TEXT("[")) && Trimmed.EndsWith(TEXT("]")))
			{
				bHasSection = IsValidSectionName(Trimmed.Mid(1, Trimmed.Len() - 2));
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

	bool SaveConfigFile(FConfigFile& File, const FString& ResolvedPath)
	{
		FString Contents;
		File.WriteToString(Contents, ResolvedPath);
		return UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(
			ResolvedPath, Contents, true, true);
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
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return DefaultValue;
	}

	FConfigFile File;
	FString StoredSectionName;
	FString Value = DefaultValue;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| !File.GetString(*StoredSectionName, *KeyName, Value))
	{
		Value = DefaultValue;
	}
	return Value;
}

int32 UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 DefaultValue)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return DefaultValue;
	}

	FConfigFile File;
	FString StoredSectionName;
	int32 Value = DefaultValue;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| !File.GetInt(*StoredSectionName, *KeyName, Value))
	{
		Value = DefaultValue;
	}
	return Value;
}

int64 UDirectiveUtilConfigFunctionLibrary::ReadConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 DefaultValue)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return DefaultValue;
	}

	FConfigFile File;
	FString StoredSectionName;
	int64 Value = DefaultValue;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| !File.GetInt64(*StoredSectionName, *KeyName, Value))
	{
		Value = DefaultValue;
	}
	return Value;
}

float UDirectiveUtilConfigFunctionLibrary::ReadConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float DefaultValue)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return DefaultValue;
	}

	FConfigFile File;
	FString StoredSectionName;
	float Value = DefaultValue;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| !File.GetFloat(*StoredSectionName, *KeyName, Value))
	{
		Value = DefaultValue;
	}
	return Value;
}

bool UDirectiveUtilConfigFunctionLibrary::ReadConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool DefaultValue)
{
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return DefaultValue;
	}

	FConfigFile File;
	FString StoredSectionName;
	bool Value = DefaultValue;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| !File.GetBool(*StoredSectionName, *KeyName, Value))
	{
		Value = DefaultValue;
	}
	return Value;
}

bool UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, TArray<FString>& OutValues)
{
	OutValues.Reset();
	FString ResolvedPath;
	if (!ValidateConfigArguments(FilePath, SectionName, KeyName, ResolvedPath))
	{
		return false;
	}

	FConfigFile File;
	FString StoredSectionName;
	TArray<FString> Values;
	if (!LoadConfigSection(ResolvedPath, SectionName, File, StoredSectionName)
		|| File.GetArray(*StoredSectionName, *KeyName, Values) <= 0)
	{
		return false;
	}
	OutValues = MoveTemp(Values);
	return true;
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
	return !Stored.IsEmpty() && Value.InitFromString(Stored) ? Value : DefaultValue;
}

FVector UDirectiveUtilConfigFunctionLibrary::ReadConfigVector(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FVector& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FVector Value;
	return !Stored.IsEmpty() && Value.InitFromString(Stored) ? Value : DefaultValue;
}

FRotator UDirectiveUtilConfigFunctionLibrary::ReadConfigRotator(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FRotator& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FRotator Value;
	return !Stored.IsEmpty() && Value.InitFromString(Stored) ? Value : DefaultValue;
}

FColor UDirectiveUtilConfigFunctionLibrary::ReadConfigColor(const FString& FilePath,
	const FString& SectionName, const FString& KeyName, const FColor& DefaultValue)
{
	const FString Stored = ReadConfigString(FilePath, SectionName, KeyName, FString());
	FColor Value;
	return !Stored.IsEmpty() && Value.InitFromString(Stored) ? Value : DefaultValue;
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
