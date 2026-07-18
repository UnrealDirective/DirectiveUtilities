// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
	EDirectiveUtilWorldType ToDirectiveWorldType(const EWorldType::Type WorldType)
	{
		switch (WorldType)
		{
		case EWorldType::None:
			return EDirectiveUtilWorldType::None;
		case EWorldType::Game:
			return EDirectiveUtilWorldType::Game;
		case EWorldType::Editor:
			return EDirectiveUtilWorldType::Editor;
		case EWorldType::PIE:
			return EDirectiveUtilWorldType::PlayInEditor;
		case EWorldType::EditorPreview:
			return EDirectiveUtilWorldType::EditorPreview;
		case EWorldType::GamePreview:
			return EDirectiveUtilWorldType::GamePreview;
		case EWorldType::GameRPC:
			return EDirectiveUtilWorldType::GameRPC;
		case EWorldType::Inactive:
			return EDirectiveUtilWorldType::Inactive;
		}

		return EDirectiveUtilWorldType::Unknown;
	}

	EDirectiveUtilBuildConfiguration ToDirectiveBuildConfiguration(const EBuildConfiguration BuildConfiguration)
	{
		switch (BuildConfiguration)
		{
		case EBuildConfiguration::Unknown:
			return EDirectiveUtilBuildConfiguration::Unknown;
		case EBuildConfiguration::Debug:
			return EDirectiveUtilBuildConfiguration::Debug;
		case EBuildConfiguration::DebugGame:
			return EDirectiveUtilBuildConfiguration::DebugGame;
		case EBuildConfiguration::Development:
			return EDirectiveUtilBuildConfiguration::Development;
		case EBuildConfiguration::Shipping:
			return EDirectiveUtilBuildConfiguration::Shipping;
		case EBuildConfiguration::Test:
			return EDirectiveUtilBuildConfiguration::Test;
		}

		return EDirectiveUtilBuildConfiguration::Unknown;
	}

	EDirectiveUtilBuildTargetType ToDirectiveBuildTargetType(const EBuildTargetType BuildTargetType)
	{
		switch (BuildTargetType)
		{
		case EBuildTargetType::Unknown:
			return EDirectiveUtilBuildTargetType::Unknown;
		case EBuildTargetType::Game:
			return EDirectiveUtilBuildTargetType::Game;
		case EBuildTargetType::Server:
			return EDirectiveUtilBuildTargetType::Server;
		case EBuildTargetType::Client:
			return EDirectiveUtilBuildTargetType::Client;
		case EBuildTargetType::Editor:
			return EDirectiveUtilBuildTargetType::Editor;
		case EBuildTargetType::Program:
			return EDirectiveUtilBuildTargetType::Program;
		}

		return EDirectiveUtilBuildTargetType::Unknown;
	}
}

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

EDirectiveUtilWorldType UDirectiveUtilFunctionLibrary::GetWorldType(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? ToDirectiveWorldType(World->WorldType) : EDirectiveUtilWorldType::Unknown;
}

EDirectiveUtilBuildConfiguration UDirectiveUtilFunctionLibrary::GetBuildConfigurationType()
{
	return ToDirectiveBuildConfiguration(FApp::GetBuildConfiguration());
}

EDirectiveUtilBuildTargetType UDirectiveUtilFunctionLibrary::GetBuildTargetType()
{
	return ToDirectiveBuildTargetType(FApp::GetBuildTargetType());
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
