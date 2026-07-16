#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "Misc/AutomationTest.h"

namespace DirectiveUtilFunctionLibraryTest
{
	enum class EClipboardStage : uint8
	{
		CopyText,
		CheckText,
		CheckString,
		CheckClear,
		Complete
	};

	class FClipboardRoundTripCommand : public IAutomationLatentCommand
	{
	public:
		FClipboardRoundTripCommand(FAutomationTestBase* InTest, FString InOriginalClipboard)
			: Test(InTest)
			, OriginalClipboard(MoveTemp(InOriginalClipboard))
		{
		}

		virtual bool Update() override
		{
			switch (Stage)
			{
			case EClipboardStage::CopyText:
				UDirectiveUtilFunctionLibrary::CopyTextToClipboard(FText::FromString(TextPayload));
				BeginRetryWindow(EClipboardStage::CheckText);
				return false;

			case EClipboardStage::CheckText:
				if (!HasExpectedValue(UDirectiveUtilFunctionLibrary::GetTextFromClipboard().ToString(), TextPayload, TEXT("GetTextFromClipboard should return the copied text")))
				{
					return false;
				}
				UDirectiveUtilFunctionLibrary::CopyStringToClipboard(StringPayload);
				BeginRetryWindow(EClipboardStage::CheckString);
				return false;

			case EClipboardStage::CheckString:
				if (!HasExpectedValue(UDirectiveUtilFunctionLibrary::GetStringFromClipboard(), StringPayload, TEXT("GetStringFromClipboard should return the copied string")))
				{
					return false;
				}
				UDirectiveUtilFunctionLibrary::ClearClipboard();
				BeginRetryWindow(EClipboardStage::CheckClear);
				return false;

			case EClipboardStage::CheckClear:
				if (!HasExpectedValue(UDirectiveUtilFunctionLibrary::GetStringFromClipboard(), FString(), TEXT("GetStringFromClipboard should return an empty string after clearing the clipboard")))
				{
					return false;
				}
				UDirectiveUtilFunctionLibrary::CopyStringToClipboard(OriginalClipboard);
				Stage = EClipboardStage::Complete;
				return true;

			case EClipboardStage::Complete:
				return true;
			}

			return true;
		}

	private:
		void BeginRetryWindow(EClipboardStage NextStage)
		{
			Stage = NextStage;
			FramesRemaining = 120;
		}

		bool HasExpectedValue(const FString& Actual, const FString& Expected, const TCHAR* FailureMessage)
		{
			if (Actual == Expected)
			{
				return true;
			}

			if (--FramesRemaining > 0)
			{
				return false;
			}

			Test->TestEqual(FailureMessage, Actual, Expected);
			return true;
		}

		FAutomationTestBase* Test;
		FString OriginalClipboard;
		EClipboardStage Stage = EClipboardStage::CopyText;
		int32 FramesRemaining = 0;
		const FString TextPayload = TEXT("Directive Utilities Text Clipboard");
		const FString StringPayload = TEXT("Directive Utilities String Clipboard");
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilFunctionLibraryTest, "DirectiveUtilities.FunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilFunctionLibraryTest::RunTest(const FString& Parameters)
{
	const FString OriginalClipboard = UDirectiveUtilFunctionLibrary::GetStringFromClipboard();
	ADD_LATENT_AUTOMATION_COMMAND(DirectiveUtilFunctionLibraryTest::FClipboardRoundTripCommand(this, OriginalClipboard));

	const FString ProjectVersion = UDirectiveUtilFunctionLibrary::GetProjectVersion();
	TestNotEqual("GetProjectVersion should return a non-empty string", ProjectVersion, FString(""));

#if WITH_EDITOR
	constexpr bool bExpectedEditorContext = true;
#else
	constexpr bool bExpectedEditorContext = false;
#endif
	TestEqual(
		"IsRunningInEditor should match the target context",
		UDirectiveUtilFunctionLibrary::IsRunningInEditor(),
		bExpectedEditorContext);

	TArray<UClass*> RecursiveDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UBlueprintFunctionLibrary::StaticClass(), true, RecursiveDerived);
	TestTrue("GetChildClasses should return a non-empty list for a base class with subclasses",
		RecursiveDerived.Num() > 0);
	TestTrue("GetChildClasses should include a known derived class (UDirectiveUtilFunctionLibrary)",
		RecursiveDerived.Contains(UDirectiveUtilFunctionLibrary::StaticClass()));
	TestFalse("GetChildClasses should not include the base class itself",
		RecursiveDerived.Contains(UBlueprintFunctionLibrary::StaticClass()));

	TArray<UClass*> NonRecursiveDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UBlueprintFunctionLibrary::StaticClass(), false, NonRecursiveDerived);
	TestTrue("GetChildClasses non-recursive count should not exceed recursive count",
		NonRecursiveDerived.Num() <= RecursiveDerived.Num());

	TArray<UClass*> LeafDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UDirectiveUtilFunctionLibrary::StaticClass(), true, LeafDerived);
	TestEqual("GetChildClasses should return an empty list for a class with no subclasses",
		LeafDerived.Num(), 0);

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

		TestFalse("HasCommandLineSwitch should not find a switch that was never passed",
			UDirectiveUtilFunctionLibrary::HasCommandLineSwitch(TEXT("DirectiveUtilitiesDefinitelyNotPassed")));
	}

	return true;
}
