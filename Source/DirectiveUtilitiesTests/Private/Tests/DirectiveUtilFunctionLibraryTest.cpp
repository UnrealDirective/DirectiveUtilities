#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilFunctionLibraryTest, "DirectiveUtilities.FunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// Preserve the user/CI clipboard so this test doesn't destroy it, and restore it before returning.
	const FString OriginalClipboard = UDirectiveUtilFunctionLibrary::GetStringFromClipboard();

	const FText TestText = FText::FromString(TEXT("Hello, Clipboard!"));
	UDirectiveUtilFunctionLibrary::CopyTextToClipboard(TestText);
	const FText ClipboardText = UDirectiveUtilFunctionLibrary::GetTextFromClipboard();
	TestEqual("GetTextFromClipboard should return the copied text", ClipboardText.ToString(), TestText.ToString());

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	const FString TestString = TEXT("Hello, Clipboard!");
	UDirectiveUtilFunctionLibrary::CopyStringToClipboard(TestString);
	const FString ClipboardString = UDirectiveUtilFunctionLibrary::GetStringFromClipboard();
	TestEqual("GetStringFromClipboard should return the copied string", ClipboardString, TestString);

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	UDirectiveUtilFunctionLibrary::ClearClipboard();
	const FString ClearedClipboardString = UDirectiveUtilFunctionLibrary::GetStringFromClipboard();
	TestEqual("GetStringFromClipboard should return an empty string after clearing the clipboard", ClearedClipboardString, TEXT(""));

	const FString ProjectVersion = UDirectiveUtilFunctionLibrary::GetProjectVersion();
	TestNotEqual("GetProjectVersion should return a non-empty string", ProjectVersion, FString(""));

	TestTrue("IsRunningInEditor should return true in editor context",
		UDirectiveUtilFunctionLibrary::IsRunningInEditor());

	// Pure pass-through to the engine's GetDerivedClasses (which appends to the output array).

	// Happy path (recursive): the running module guarantees a stable hierarchy to query.
	TArray<UClass*> RecursiveDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UBlueprintFunctionLibrary::StaticClass(), true, RecursiveDerived);
	TestTrue("GetChildClasses should return a non-empty list for a base class with subclasses",
		RecursiveDerived.Num() > 0);
	TestTrue("GetChildClasses should include a known derived class (UDirectiveUtilFunctionLibrary)",
		RecursiveDerived.Contains(UDirectiveUtilFunctionLibrary::StaticClass()));
	TestFalse("GetChildClasses should not include the base class itself",
		RecursiveDerived.Contains(UBlueprintFunctionLibrary::StaticClass()));

	// Non-recursive results must not exceed the recursive results for the same base.
	TArray<UClass*> NonRecursiveDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UBlueprintFunctionLibrary::StaticClass(), false, NonRecursiveDerived);
	TestTrue("GetChildClasses non-recursive count should not exceed recursive count",
		NonRecursiveDerived.Num() <= RecursiveDerived.Num());

	// Leaf class with no subclasses: an empty input array should remain empty.
	TArray<UClass*> LeafDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UDirectiveUtilFunctionLibrary::StaticClass(), true, LeafDerived);
	TestEqual("GetChildClasses should return an empty list for a class with no subclasses",
		LeafDerived.Num(), 0);

	// Null base class: should not crash and should add nothing.
	TArray<UClass*> NullBaseDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(nullptr, true, NullBaseDerived);
	TestEqual("GetChildClasses should return an empty list for a null base class",
		NullBaseDerived.Num(), 0);

	{
		const TCHAR* CommandLine = TEXT("-Fast -Mode=Quality -Name=\"Big Save\"");

		TestTrue("HasCommandLineSwitch should find a present switch",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(CommandLine, TEXT("Fast")));
		TestTrue("HasCommandLineSwitch should match case-insensitively",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(CommandLine, TEXT("fast")));
		TestFalse("HasCommandLineSwitch should not find an absent switch",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(CommandLine, TEXT("Slow")));
		TestFalse("HasCommandLineSwitch should return false for an empty switch",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(CommandLine, TEXT("")));

		FString Value;
		TestTrue("GetCommandLineOption should find a present key",
			UDirectiveUtilFunctionLibrary::GetCommandLineOption(CommandLine, TEXT("Mode"), Value));
		TestEqual("GetCommandLineOption should return the key's value", Value, FString(TEXT("Quality")));
		TestTrue("GetCommandLineOption should read a quoted value",
			UDirectiveUtilFunctionLibrary::GetCommandLineOption(CommandLine, TEXT("Name"), Value));
		TestEqual("GetCommandLineOption should return the quoted value without quotes", Value, FString(TEXT("Big Save")));
		TestFalse("GetCommandLineOption should not find an absent key",
			UDirectiveUtilFunctionLibrary::GetCommandLineOption(CommandLine, TEXT("Missing"), Value));
		TestFalse("GetCommandLineOption should return false for an empty key",
			UDirectiveUtilFunctionLibrary::GetCommandLineOption(CommandLine, TEXT(""), Value));

		// Smoke test through the process-command-line path.
		TestFalse("HasCommandLineSwitch should not find a switch that was never passed",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(TEXT("DirectiveUtilitiesDefinitelyNotPassed")));
	}

	UDirectiveUtilFunctionLibrary::CopyStringToClipboard(OriginalClipboard);
	return true;
}
