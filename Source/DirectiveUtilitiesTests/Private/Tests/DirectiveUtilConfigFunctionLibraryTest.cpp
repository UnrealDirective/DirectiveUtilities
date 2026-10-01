// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilConfigFunctionLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace
{
	FString GetTestFilePath()
	{
		// ProjectSavedDir is itself relative to the binaries directory, so resolve it up front
		// and exercise the nodes with the absolute paths their contract describes.
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("DirectiveUtilTests") / TEXT("Config") / TEXT("Settings.ini");
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilConfigFunctionLibraryTest,
	"DirectiveUtilities.ConfigFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilConfigFunctionLibraryTest::RunTest(const FString& Parameters)
{
	const FString FilePath = GetTestFilePath();
	IFileManager::Get().Delete(*FilePath);

	TestEqual(TEXT("A missing file reads the string default"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(FilePath, TEXT("Audio"), TEXT("Volume"), TEXT("0.5")), FString(TEXT("0.5")));
	TestEqual(TEXT("A missing file reads the int default"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(FilePath, TEXT("Audio"), TEXT("Channels"), 2), 2);
	TArray<FString> MissingArray;
	TestFalse(TEXT("A missing key reads no array"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(FilePath, TEXT("Audio"), TEXT("Missing"), MissingArray));

	TestTrue(TEXT("Writing a string creates the file"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(FilePath, TEXT("Audio"), TEXT("Device"), TEXT("Speakers")));
	TestTrue(TEXT("The written file exists on disk"), IFileManager::Get().FileExists(*FilePath));
	TestEqual(TEXT("The written string reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(FilePath, TEXT("Audio"), TEXT("Device"), TEXT("")), FString(TEXT("Speakers")));
	const FString EscapedString = TEXT(" leading // text\nsecond \\\"quoted\\\" \\\\");
	TestTrue(TEXT("Writing a string that needs config escaping succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			FilePath, TEXT("Text"), TEXT("Escaped"), EscapedString));
	TestEqual(TEXT("Config escaping round trips exactly"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(
			FilePath, TEXT("Text"), TEXT("Escaped"), FString()), EscapedString);
	TestTrue(TEXT("HasConfigKey finds an existing key"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigKey(FilePath, TEXT("audio"), TEXT("device")));
	TestFalse(TEXT("HasConfigKey misses an absent key"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigKey(FilePath, TEXT("Audio"), TEXT("Missing")));

	TestTrue(TEXT("Writing an int succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigInt(FilePath, TEXT("Audio"), TEXT("Channels"), 6));
	TestEqual(TEXT("The int reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(FilePath, TEXT("Audio"), TEXT("Channels"), 0), 6);
	TestTrue(TEXT("A differently cased section updates the existing section"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigInt(FilePath, TEXT("audio"), TEXT("channels"), 7));
	TestEqual(TEXT("Section and key reads ignore case"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(FilePath, TEXT("AUDIO"), TEXT("CHANNELS"), 0), 7);

	TestTrue(TEXT("Writing an int64 succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigInt64(FilePath, TEXT("Progress"), TEXT("Experience"), 9876543210123));
	TestEqual(TEXT("The int64 reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigInt64(FilePath, TEXT("Progress"), TEXT("Experience"), 0), 9876543210123);

	TestTrue(TEXT("Writing a float succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigFloat(FilePath, TEXT("Audio"), TEXT("Music"), 0.75f));
	TestTrue("The float reads back", FMath::IsNearlyEqual(
		UDirectiveUtilConfigFunctionLibrary::ReadConfigFloat(FilePath, TEXT("Audio"), TEXT("Music"), 0.0f), 0.75f));

	TestTrue(TEXT("Writing a bool succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigBool(FilePath, TEXT("Video"), TEXT("bVSync"), true));
	TestEqual(TEXT("The bool reads back as text"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(FilePath, TEXT("Video"), TEXT("bVSync"), TEXT("")), FString(TEXT("True")));
	TestTrue(TEXT("The bool reads back through ReadConfigBool"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigBool(FilePath, TEXT("Video"), TEXT("bVSync"), false));
	TestFalse(TEXT("A missing bool falls back to its default"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigBool(FilePath, TEXT("Video"), TEXT("bMissing"), false));

	const FVector2D UiScale(1.25, 0.75);
	const FVector SpawnLocation(100.0, -25.0, 8.0);
	const FRotator SpawnRotation(10.0, 90.0, 5.0);
	const FColor AccentColor(12, 34, 56, 78);
	TestTrue(TEXT("Writing a Vector2D succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigVector2D(FilePath, TEXT("Spatial"), TEXT("UiScale"), UiScale));
	TestTrue(TEXT("Writing a Vector succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigVector(FilePath, TEXT("Spatial"), TEXT("Location"), SpawnLocation));
	TestTrue(TEXT("Writing a Rotator succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigRotator(FilePath, TEXT("Spatial"), TEXT("Rotation"), SpawnRotation));
	TestTrue(TEXT("Writing a Color succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigColor(FilePath, TEXT("Spatial"), TEXT("Accent"), AccentColor));
	TestEqual(TEXT("The Vector2D reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigVector2D(FilePath, TEXT("Spatial"), TEXT("UiScale"), FVector2D::ZeroVector), UiScale);
	TestEqual(TEXT("The Vector reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigVector(FilePath, TEXT("Spatial"), TEXT("Location"), FVector::ZeroVector), SpawnLocation);
	TestEqual(TEXT("The Rotator reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigRotator(FilePath, TEXT("Spatial"), TEXT("Rotation"), FRotator::ZeroRotator), SpawnRotation);
	TestEqual(TEXT("The Color reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigColor(FilePath, TEXT("Spatial"), TEXT("Accent"), FColor::Black), AccentColor);
	TestEqual(TEXT("A malformed Vector uses its default"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigVector(FilePath, TEXT("Audio"), TEXT("Device"), SpawnLocation), SpawnLocation);

	TestTrue(TEXT("Writing an array succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(FilePath, TEXT("Mods"), TEXT("LoadOrder"), { TEXT("a"), TEXT("b") }));
	TArray<FString> LoadedArray;
	TestTrue(TEXT("The array reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(FilePath, TEXT("Mods"), TEXT("LoadOrder"), LoadedArray));
	TestEqual(TEXT("Array entries survive"), LoadedArray, TArray<FString>({ TEXT("a"), TEXT("b") }));
	TestEqual(TEXT("A scalar read of an array key returns the last entry"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(FilePath, TEXT("Mods"), TEXT("LoadOrder"), TEXT("missing")),
		FString(TEXT("b")));
	TestTrue(TEXT("A scalar replaces every prior array entry"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(FilePath, TEXT("Mods"), TEXT("LoadOrder"), TEXT("single")));
	TestTrue(TEXT("The replacement reads as one value"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(FilePath, TEXT("Mods"), TEXT("LoadOrder"), LoadedArray));
	TestEqual(TEXT("No stale array entry remains"), LoadedArray, TArray<FString>({ TEXT("single") }));
	TestTrue(TEXT("An array can replace the scalar again"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(
			FilePath, TEXT("Mods"), TEXT("LoadOrder"), { TEXT("a"), TEXT("b") }));
	TestTrue(TEXT("An empty array removes every value for the key"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(
			FilePath, TEXT("Mods"), TEXT("LoadOrder"), {}));
	TestFalse(TEXT("A removed array key is absent"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(
			FilePath, TEXT("Mods"), TEXT("LoadOrder"), LoadedArray));
	TestTrue(TEXT("The array can be restored after removal"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(
			FilePath, TEXT("Mods"), TEXT("LoadOrder"), { TEXT("a"), TEXT("b") }));

	TArray<FString> Sections;
	TestTrue(TEXT("Section names list from a real file"),
		UDirectiveUtilConfigFunctionLibrary::GetConfigSectionNames(FilePath, Sections));
	TestEqual(TEXT("All six sections were listed"), Sections.Num(), 6);

	TArray<FString> Keys;
	TestTrue(TEXT("Keys list inside a section"),
		UDirectiveUtilConfigFunctionLibrary::GetConfigKeysInSection(FilePath, TEXT("Audio"), Keys));
	TestEqual(TEXT("The audio section holds three keys"), Keys.Num(), 3);
	TestFalse(TEXT("Listing keys of a missing section fails"),
		UDirectiveUtilConfigFunctionLibrary::GetConfigKeysInSection(FilePath, TEXT("Nope"), Keys));

	TestTrue(TEXT("HasConfigSection finds an existing section"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(FilePath, TEXT("Video")));
	TestFalse(TEXT("HasConfigSection misses an absent section"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(FilePath, TEXT("Absent")));

	TestTrue(TEXT("RemoveConfigKey deletes an existing key"),
		UDirectiveUtilConfigFunctionLibrary::RemoveConfigKey(FilePath, TEXT("Audio"), TEXT("Device")));
	TestEqual(TEXT("The removed key falls back to defaults"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(FilePath, TEXT("Audio"), TEXT("Device"), TEXT("gone")), FString(TEXT("gone")));
	TestFalse(TEXT("Removing a missing key fails"),
		UDirectiveUtilConfigFunctionLibrary::RemoveConfigKey(FilePath, TEXT("Audio"), TEXT("Device")));

	TestTrue(TEXT("ClearConfigSection ignores section-name case"),
		UDirectiveUtilConfigFunctionLibrary::ClearConfigSection(FilePath, TEXT("aUdIo")));
	TestFalse(TEXT("The cleared section is gone"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(FilePath, TEXT("Audio")));

	// FConfigFile::Write deletes a file whose generated text is empty; clearing every
	// section must leave the file in place instead.
	for (const FString& Remaining : { FString(TEXT("Video")), FString(TEXT("Progress")), FString(TEXT("Mods")), FString(TEXT("Spatial")), FString(TEXT("Text")) })
	{
		UDirectiveUtilConfigFunctionLibrary::ClearConfigSection(FilePath, Remaining);
	}
	TestTrue(TEXT("Clearing the last section leaves the file on disk"),
		IFileManager::Get().FileExists(*FilePath));

	TestFalse(TEXT("Empty arguments fail writes"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigInt(FilePath, TEXT(""), TEXT("Key"), 1));
	TestFalse(TEXT("Empty arguments fail section queries"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(FString(), TEXT("Audio")));
	const FString OversizedKey = FString::ChrN(NAME_SIZE, TEXT('K'));
	TestFalse(TEXT("Oversized keys fail without constructing an FName"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			FilePath, TEXT("Runtime"), OversizedKey, TEXT("value")));
	TestEqual(TEXT("Oversized key reads return the default"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(
			FilePath, TEXT("Runtime"), OversizedKey, TEXT("default")),
		FString(TEXT("default")));
	for (const FString& InvalidKey : { FString(TEXT(" Bad")), FString(TEXT("Bad=Key")), FString(TEXT(";Bad")), FString(TEXT("~Bad")), FString(TEXT("Bad//Key")) })
	{
		TestFalse(TEXT("Keys that change meaning when serialized are rejected"),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
				FilePath, TEXT("Runtime"), InvalidKey, TEXT("value")));
	}
	TestFalse(TEXT("Section syntax is rejected"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			FilePath, TEXT("[Runtime]"), TEXT("Mode"), TEXT("value")));
	TestFalse(TEXT("Section names that would be truncated as comments are rejected"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			FilePath, TEXT("Runtime//Ignored"), TEXT("Mode"), TEXT("value")));
	TestFalse(TEXT("Non-finite floats are rejected"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigFloat(
			FilePath, TEXT("Runtime"), TEXT("InvalidFloat"), std::numeric_limits<float>::infinity()));
	TestFalse(TEXT("Non-finite vectors are rejected"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigVector(
			FilePath, TEXT("Runtime"), TEXT("InvalidVector"),
			FVector(std::numeric_limits<double>::infinity(), 0.0, 0.0)));

	const FString MalformedPath = FPaths::GetPath(FilePath) / TEXT("Malformed.ini");
	const FString MalformedContents = TEXT("[Safe]\nGood=1\nthis is not a key\n");
	TestTrue(TEXT("A malformed source file is created"),
		FFileHelper::SaveStringToFile(MalformedContents, *MalformedPath));
	TestFalse(TEXT("A write refuses to replace malformed existing content"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			MalformedPath, TEXT("Safe"), TEXT("Good"), TEXT("2")));
	FString PreservedMalformedContents;
	TestTrue(TEXT("The refused malformed file still reads as bytes"),
		FFileHelper::LoadFileToString(PreservedMalformedContents, *MalformedPath));
	TestEqual(TEXT("The malformed file remains unchanged"), PreservedMalformedContents, MalformedContents);
	IFileManager::Get().Delete(*MalformedPath);

	const FString UnterminatedQuotePath = FPaths::GetPath(FilePath) / TEXT("UnterminatedQuote.ini");
	const FString UnterminatedQuoteContents = TEXT("[Safe]\nGood=\"unterminated\n");
	TestTrue(TEXT("A source file with an unterminated quoted value is created"),
		FFileHelper::SaveStringToFile(UnterminatedQuoteContents, *UnterminatedQuotePath));
	TestFalse(TEXT("A write refuses an unterminated quoted value"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			UnterminatedQuotePath, TEXT("Safe"), TEXT("Good"), TEXT("2")));
	FString PreservedUnterminatedQuote;
	TestTrue(TEXT("The refused quoted file still reads as bytes"),
		FFileHelper::LoadFileToString(PreservedUnterminatedQuote, *UnterminatedQuotePath));
	TestEqual(TEXT("The unterminated quoted file remains unchanged"),
		PreservedUnterminatedQuote, UnterminatedQuoteContents);
	IFileManager::Get().Delete(*UnterminatedQuotePath);

	const FString OversizedFilePath = FPaths::GetPath(FilePath) / TEXT("OversizedKey.ini");
	const FString OversizedFileContents = FString(TEXT("[Safe]\n")) + OversizedKey + TEXT("=1\n");
	TestTrue(TEXT("A source file with an oversized key is created"),
		FFileHelper::SaveStringToFile(OversizedFileContents, *OversizedFilePath));
	TestEqual(TEXT("Reading a file with an oversized key fails safely"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(
			OversizedFilePath, TEXT("Safe"), TEXT("Good"), TEXT("default")),
		FString(TEXT("default")));
	TestFalse(TEXT("Writing refuses a source file with an oversized key"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			OversizedFilePath, TEXT("Safe"), TEXT("Good"), TEXT("2")));
	IFileManager::Get().Delete(*OversizedFilePath);

	const FString SyntaxPath = FPaths::GetPath(FilePath) / TEXT("Syntax.ini");
	IFileManager::Get().Delete(*SyntaxPath);
	const FString MacroValue = TEXT("%GAME%%GAME%%GAME%%GAME%%GAMEDIR%");
	TestTrue(TEXT("A value with many engine macros writes"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(SyntaxPath, TEXT("Macros"), TEXT("Value"), MacroValue));
	TestEqual(TEXT("Engine macros read back verbatim"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(SyntaxPath, TEXT("Macros"), TEXT("Value"), FString()), MacroValue);
	TestTrue(TEXT("Macro array entries write"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(
			SyntaxPath, TEXT("Macros"), TEXT("List"), { MacroValue, TEXT("%GAME%") }));
	TArray<FString> MacroArray;
	TestTrue(TEXT("Macro array entries read"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(SyntaxPath, TEXT("Macros"), TEXT("List"), MacroArray));
	TestEqual(TEXT("Macro array entries read back verbatim"), MacroArray, TArray<FString>({ MacroValue, TEXT("%GAME%") }));
	TestEqual(TEXT("A macro value read as an int uses the stored text"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigInt(SyntaxPath, TEXT("Macros"), TEXT("Value"), 9), 0);

	const TArray<FString> ParserSensitiveValues = {
		TEXT("\tTabbed\t"), TEXT("\nLeading break"), TEXT("Trailing break\r\n"), TEXT("{Braced}"), TEXT("a\"{\"b"),
		TEXT("]"), TEXT("Ends with \\"), TEXT("\"Quoted\""), TEXT("C:\\Saved\\Path"), FString() };
	for (const FString& SensitiveValue : ParserSensitiveValues)
	{
		TestTrue(*FString::Printf(TEXT("A parser-sensitive value writes: %s"), *SensitiveValue.ReplaceCharWithEscapedChar()),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(SyntaxPath, TEXT("Values"), TEXT("Sensitive"), SensitiveValue));
		TestEqual(*FString::Printf(TEXT("A parser-sensitive value reads back: %s"), *SensitiveValue.ReplaceCharWithEscapedChar()),
			UDirectiveUtilConfigFunctionLibrary::ReadConfigString(SyntaxPath, TEXT("Values"), TEXT("Sensitive"), TEXT("default")),
			SensitiveValue);
	}
	TestTrue(TEXT("Parser-sensitive array entries write"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(SyntaxPath, TEXT("Values"), TEXT("List"), ParserSensitiveValues));
	TArray<FString> SensitiveArray;
	TestTrue(TEXT("Parser-sensitive array entries read"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(SyntaxPath, TEXT("Values"), TEXT("List"), SensitiveArray));
	TestEqual(TEXT("Parser-sensitive array entries round trip"), SensitiveArray, ParserSensitiveValues);

	FString ContentsBeforeRejectedWrites;
	TestTrue(TEXT("The syntax file reads as bytes"),
		FFileHelper::LoadFileToString(ContentsBeforeRejectedWrites, *SyntaxPath));
	for (const FString& InvalidSection : { FString(TEXT("Bad{Section")), FString(TEXT("Bad}Section")), FString(TEXT("Bad\"Section")) })
	{
		TestFalse(*FString::Printf(TEXT("Section names with braces or quotes are rejected: %s"), *InvalidSection),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(SyntaxPath, InvalidSection, TEXT("Key"), TEXT("value")));
	}
	for (const FString& InvalidKey : { FString(TEXT("Bad{Key")), FString(TEXT("Bad}Key")), FString(TEXT("Bad\"Key")), FString(TEXT("[Bad")) })
	{
		TestFalse(*FString::Printf(TEXT("Keys with braces, quotes, or a leading bracket are rejected: %s"), *InvalidKey),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(SyntaxPath, TEXT("Values"), InvalidKey, TEXT("]")));
		TestFalse(*FString::Printf(TEXT("Array writes reject the same keys: %s"), *InvalidKey),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(SyntaxPath, TEXT("Values"), InvalidKey, { TEXT("value") }));
	}
	for (const FString& CommandKey : { FString(TEXT("+Bad")), FString(TEXT("-Bad")), FString(TEXT(".Bad")), FString(TEXT("!Bad")),
		FString(TEXT("@Bad")), FString(TEXT("*Bad")), FString(TEXT("^Bad")) })
	{
		TestFalse(*FString::Printf(TEXT("Keys with an engine command prefix are rejected: %s"), *CommandKey),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(SyntaxPath, TEXT("Values"), CommandKey, TEXT("value")));
	}
	FString ContentsAfterRejectedWrites;
	TestTrue(TEXT("The syntax file still reads as bytes"),
		FFileHelper::LoadFileToString(ContentsAfterRejectedWrites, *SyntaxPath));
	TestEqual(TEXT("Rejected writes leave the file unchanged"), ContentsAfterRejectedWrites, ContentsBeforeRejectedWrites);

	TestTrue(TEXT("Removing the last key of a section succeeds"),
		UDirectiveUtilConfigFunctionLibrary::RemoveConfigKey(SyntaxPath, TEXT("Macros"), TEXT("Value"))
		&& UDirectiveUtilConfigFunctionLibrary::RemoveConfigKey(SyntaxPath, TEXT("Macros"), TEXT("List")));
	TestTrue(TEXT("The emptied section remains"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(SyntaxPath, TEXT("Macros")));
	TArray<FString> EmptySectionKeys;
	TestTrue(TEXT("The emptied section lists keys"),
		UDirectiveUtilConfigFunctionLibrary::GetConfigKeysInSection(SyntaxPath, TEXT("Macros"), EmptySectionKeys));
	TestEqual(TEXT("The emptied section holds no keys"), EmptySectionKeys.Num(), 0);
	TestTrue(TEXT("An empty array write on a new section succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigStringArray(SyntaxPath, TEXT("Fresh"), TEXT("List"), {}));
	TestTrue(TEXT("The empty array write creates the section"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(SyntaxPath, TEXT("Fresh")));
	TestFalse(TEXT("The empty array write stores no key"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigStringArray(SyntaxPath, TEXT("Fresh"), TEXT("List"), EmptySectionKeys));
	IFileManager::Get().Delete(*SyntaxPath);

	const FString HandAuthoredPath = FPaths::GetPath(FilePath) / TEXT("HandAuthored.ini");
	TestTrue(TEXT("A hand-authored file with an empty section is created"),
		FFileHelper::SaveStringToFile(FString(TEXT("[Empty]\n[Data]\nRoot=%GAME%%GAME%%GAME%%GAME%\n")), *HandAuthoredPath));
	TestEqual(TEXT("A hand-authored macro value reads verbatim"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(HandAuthoredPath, TEXT("Data"), TEXT("Root"), FString()),
		FString(TEXT("%GAME%%GAME%%GAME%%GAME%")));
	TestTrue(TEXT("An unrelated write succeeds"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigInt(HandAuthoredPath, TEXT("Data"), TEXT("Count"), 2));
	TestTrue(TEXT("A hand-authored empty section survives an unrelated write"),
		UDirectiveUtilConfigFunctionLibrary::HasConfigSection(HandAuthoredPath, TEXT("Empty")));
	IFileManager::Get().Delete(*HandAuthoredPath);

	const FString JoinedLinePath = FPaths::GetPath(FilePath) / TEXT("JoinedLines.ini");
	for (const FString& JoinedContents : {
		FString(TEXT("[Safe]\nGood=1\nBad{=2\n")),
		FString(TEXT("[Safe]\nGood={open\nNext=2\n")),
		FString(TEXT("[Safe]\nPath=C:\\Dir\\\nNext=2\n")),
		FString(TEXT("; note {\n[Safe]\nGood=1\n")),
		FString(TEXT("  [Safe]\nGood=1\n")),
		FString(TEXT("[Safe]\nGood=1\nJson={\"a\":1}\n")),
		FString(TEXT("[Safe]\nGood=1\nTail=a}\n")),
		FString(TEXT("[Safe]\nGood=1\n  ;Hidden=1\n")) })
	{
		TestTrue(TEXT("A file the engine parser reads differently is created"),
			FFileHelper::SaveStringToFile(JoinedContents, *JoinedLinePath));
		TestEqual(*FString::Printf(TEXT("Reading refuses parser-ambiguous text: %s"), *JoinedContents.ReplaceCharWithEscapedChar()),
			UDirectiveUtilConfigFunctionLibrary::ReadConfigString(JoinedLinePath, TEXT("Safe"), TEXT("Good"), TEXT("default")),
			FString(TEXT("default")));
		TestFalse(*FString::Printf(TEXT("Writing refuses parser-ambiguous text: %s"), *JoinedContents.ReplaceCharWithEscapedChar()),
			UDirectiveUtilConfigFunctionLibrary::WriteConfigString(JoinedLinePath, TEXT("Safe"), TEXT("Good"), TEXT("3")));
		FString PreservedJoinedContents;
		TestTrue(TEXT("The refused file still reads as bytes"),
			FFileHelper::LoadFileToString(PreservedJoinedContents, *JoinedLinePath));
		TestEqual(TEXT("The refused file remains unchanged"), PreservedJoinedContents, JoinedContents);
	}
	IFileManager::Get().Delete(*JoinedLinePath);

	const FString RelativePath = FString(TEXT("DirectiveUtilTests")) / TEXT("Config") / TEXT("Relative.ini");
	const FString AbsoluteRelativePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / RelativePath;
	IFileManager::Get().Delete(*AbsoluteRelativePath);
	TestTrue(TEXT("A Saved-relative config path writes"),
		UDirectiveUtilConfigFunctionLibrary::WriteConfigString(
			RelativePath, TEXT("Runtime"), TEXT("Mode"), TEXT("Packaged")));
	TestEqual(TEXT("A Saved-relative config path reads back"),
		UDirectiveUtilConfigFunctionLibrary::ReadConfigString(
			RelativePath, TEXT("Runtime"), TEXT("Mode"), TEXT("Missing")),
		FString(TEXT("Packaged")));
	TestTrue(TEXT("The relative config landed under the project Saved directory"),
		FPaths::FileExists(AbsoluteRelativePath));
	IFileManager::Get().Delete(*AbsoluteRelativePath);

	IFileManager::Get().Delete(*FilePath);
	return true;
}
