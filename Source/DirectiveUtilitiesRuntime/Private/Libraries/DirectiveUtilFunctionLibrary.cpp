// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

void UDirectiveUtilFunctionLibrary::GetChildClasses(const UClass* BaseClass, const bool bRecursive, TArray<UClass*>& DerivedClasses)
{
	GetDerivedClasses(BaseClass, DerivedClasses, bRecursive);
}

void UDirectiveUtilFunctionLibrary::CopyTextToClipboard(const FText& Text)
{
	const FString ClipboardText = Text.ToString();
	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

void UDirectiveUtilFunctionLibrary::CopyStringToClipboard(const FString& String)
{
	FPlatformApplicationMisc::ClipboardCopy(*String);
}

FText UDirectiveUtilFunctionLibrary::GetTextFromClipboard()
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);
	return FText::FromString(ClipboardText);
}

FString UDirectiveUtilFunctionLibrary::GetStringFromClipboard()
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);
	return ClipboardText;
}

void UDirectiveUtilFunctionLibrary::ClearClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(TEXT(""));
}

FString UDirectiveUtilFunctionLibrary::GetProjectVersion()
{
	FString ProjectVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		ProjectVersion,
		GGameIni);
	return ProjectVersion;
}

bool UDirectiveUtilFunctionLibrary::IsRunningInEditor()
{
	return GIsEditor;
}

bool UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(const FString& Switch)
{
	return HasCommandLineSwitch(FCommandLine::Get(), Switch);
}

bool UDirectiveUtilFunctionLibrary::GetCommandLineOption(const FString& Key, FString& OutValue)
{
	return GetCommandLineOption(FCommandLine::Get(), Key, OutValue);
}

bool UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(const TCHAR* CommandLine, const FString& Switch)
{
	if (Switch.IsEmpty())
	{
		return false;
	}
	return FParse::Param(CommandLine, *Switch);
}

bool UDirectiveUtilFunctionLibrary::GetCommandLineOption(const TCHAR* CommandLine, const FString& Key, FString& OutValue)
{
	OutValue.Reset();
	if (Key.IsEmpty())
	{
		return false;
	}
	return FParse::Value(CommandLine, *(Key + TEXT("=")), OutValue);
}
