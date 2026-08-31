// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/Package.h"

namespace DirectiveUtilFunctionLibraryTest
{
	bool CanVerifyClipboardRoundTrip()
	{
#if PLATFORM_LINUX
		return !FParse::Param(FCommandLine::Get(), TEXT("NullRHI"));
#else
		return true;
#endif
	}

	EDirectiveUtilBuildConfiguration GetExpectedBuildConfiguration(const EBuildConfiguration BuildConfiguration)
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

	EDirectiveUtilBuildTargetType GetExpectedBuildTargetType(const EBuildTargetType BuildTargetType)
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
	if (DirectiveUtilFunctionLibraryTest::CanVerifyClipboardRoundTrip())
	{
		const FString OriginalClipboard = UDirectiveUtilFunctionLibrary::GetStringFromClipboard();
		ADD_LATENT_AUTOMATION_COMMAND(DirectiveUtilFunctionLibraryTest::FClipboardRoundTripCommand(this, OriginalClipboard));
	}
	else
	{
		UDirectiveUtilFunctionLibrary::CopyTextToClipboard(FText::FromString(TEXT("Directive Utilities Text Clipboard")));
		UDirectiveUtilFunctionLibrary::GetTextFromClipboard();
		UDirectiveUtilFunctionLibrary::CopyStringToClipboard(TEXT("Directive Utilities String Clipboard"));
		UDirectiveUtilFunctionLibrary::GetStringFromClipboard();
		UDirectiveUtilFunctionLibrary::ClearClipboard();
	}

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

	struct FWorldTypeCase
	{
		EWorldType::Type WorldType;
		EDirectiveUtilWorldType Expected;
	};

	const FWorldTypeCase WorldTypeCases[] = {
		{ EWorldType::None, EDirectiveUtilWorldType::None },
		{ EWorldType::Game, EDirectiveUtilWorldType::Game },
		{ EWorldType::Editor, EDirectiveUtilWorldType::Editor },
		{ EWorldType::PIE, EDirectiveUtilWorldType::PlayInEditor },
		{ EWorldType::EditorPreview, EDirectiveUtilWorldType::EditorPreview },
		{ EWorldType::GamePreview, EDirectiveUtilWorldType::GamePreview },
		{ EWorldType::GameRPC, EDirectiveUtilWorldType::GameRPC },
		{ EWorldType::Inactive, EDirectiveUtilWorldType::Inactive }
	};

	UWorld* TestWorld = NewObject<UWorld>();
	TestNotNull("A transient world should be created for world type tests", TestWorld);
	if (TestWorld)
	{
		for (const FWorldTypeCase& TestCase : WorldTypeCases)
		{
			TestWorld->WorldType = TestCase.WorldType;
			TestEqual(
				FString::Printf(TEXT("GetWorldType should map %s"), LexToString(TestCase.WorldType)),
				UDirectiveUtilFunctionLibrary::GetWorldType(TestWorld),
				TestCase.Expected);
		}

		TestWorld->WorldType = static_cast<EWorldType::Type>(MAX_uint8);
		TestEqual(
			"GetWorldType should reject unsupported world types",
			UDirectiveUtilFunctionLibrary::GetWorldType(TestWorld),
			EDirectiveUtilWorldType::Unknown);
	}

	TestEqual(
		"GetWorldType should return Unknown for a null context",
		UDirectiveUtilFunctionLibrary::GetWorldType(nullptr),
		EDirectiveUtilWorldType::Unknown);
	TestEqual(
		"GetWorldType should return Unknown when the context has no world",
		UDirectiveUtilFunctionLibrary::GetWorldType(GetTransientPackage()),
		EDirectiveUtilWorldType::Unknown);

	TestEqual(
		"GetBuildConfigurationType should match the application build configuration",
		UDirectiveUtilFunctionLibrary::GetBuildConfigurationType(),
		DirectiveUtilFunctionLibraryTest::GetExpectedBuildConfiguration(FApp::GetBuildConfiguration()));
	TestEqual(
		"GetBuildTargetType should match the application build target",
		UDirectiveUtilFunctionLibrary::GetBuildTargetType(),
		DirectiveUtilFunctionLibraryTest::GetExpectedBuildTargetType(FApp::GetBuildTargetType()));

	TestNotNull("The world type enum should be reflected", StaticEnum<EDirectiveUtilWorldType>());
	TestNotNull("The build configuration enum should be reflected", StaticEnum<EDirectiveUtilBuildConfiguration>());
	TestNotNull("The build target type enum should be reflected", StaticEnum<EDirectiveUtilBuildTargetType>());

	const UFunction* GetWorldTypeFunction = UDirectiveUtilFunctionLibrary::StaticClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilFunctionLibrary, GetWorldType));
	TestNotNull("GetWorldType should be reflected", GetWorldTypeFunction);
	if (GetWorldTypeFunction)
	{
		TestTrue("GetWorldType should be Blueprint pure", GetWorldTypeFunction->HasAnyFunctionFlags(FUNC_BlueprintPure));
		TestFalse("GetWorldType should be available at runtime", GetWorldTypeFunction->HasAnyFunctionFlags(FUNC_EditorOnly));
#if WITH_EDITOR
		TestEqual(
			"GetWorldType should use its input as the Blueprint world context",
			GetWorldTypeFunction->GetMetaData(TEXT("WorldContext")),
			FString(TEXT("WorldContextObject")));
#endif
	}

	TArray<UClass*> RecursiveDerived;
	UDirectiveUtilFunctionLibrary::GetChildClasses(UBlueprintFunctionLibrary::StaticClass(), true, RecursiveDerived);
	TestTrue("GetChildClasses should return a non-empty list for a base class with subclasses",
		RecursiveDerived.Num() > 0);
	TestTrue("GetChildClasses should include a known derived class (UDirectiveUtilFunctionLibrary)",
		RecursiveDerived.Contains(UDirectiveUtilFunctionLibrary::StaticClass()));
	TestFalse("GetChildClasses should not include the base class itself",
		RecursiveDerived.Contains(UBlueprintFunctionLibrary::StaticClass()));

	double ElapsedMilliseconds = 123.0;
	TestFalse(
		"StartStopwatch should reject NAME_None",
		UDirectiveUtilFunctionLibrary::StartStopwatch(NAME_None));
	TestFalse(
		"StopStopwatch should reject NAME_None",
		UDirectiveUtilFunctionLibrary::StopStopwatch(NAME_None, ElapsedMilliseconds));
	TestEqual("StopStopwatch should reset elapsed time for NAME_None", ElapsedMilliseconds, 0.0);

	const FName MissingStopwatchKey(TEXT("DirectiveUtilMissingStopwatch"));
	ElapsedMilliseconds = 123.0;
	TestFalse(
		"StopStopwatch should reject a missing key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(MissingStopwatchKey, ElapsedMilliseconds));
	TestEqual("StopStopwatch should reset elapsed time for a missing key", ElapsedMilliseconds, 0.0);

	const FName BasicStopwatchKey(TEXT("DirectiveUtilBasicStopwatch"));
	TestTrue(
		"StartStopwatch should start an unused key",
		UDirectiveUtilFunctionLibrary::StartStopwatch(BasicStopwatchKey));
	TestFalse(
		"StartStopwatch should preserve an active key by default",
		UDirectiveUtilFunctionLibrary::StartStopwatch(BasicStopwatchKey));
	TestTrue(
		"StopStopwatch should stop an active key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(BasicStopwatchKey, ElapsedMilliseconds));
	TestTrue("StopStopwatch should return a non-negative duration", ElapsedMilliseconds >= 0.0);
	TestFalse(
		"StopStopwatch should consume the active key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(BasicStopwatchKey, ElapsedMilliseconds));

	const FName RestartStopwatchKey(TEXT("DirectiveUtilRestartStopwatch"));
	TestTrue(
		"StartStopwatch should start the restart test key",
		UDirectiveUtilFunctionLibrary::StartStopwatch(RestartStopwatchKey));
	TestTrue(
		"StartStopwatch should replace an active key when requested",
		UDirectiveUtilFunctionLibrary::StartStopwatch(RestartStopwatchKey, true));
	TestTrue(
		"StopStopwatch should stop a restarted key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(RestartStopwatchKey, ElapsedMilliseconds));

	const FName FirstOverlapKey(TEXT("DirectiveUtilFirstOverlapStopwatch"));
	const FName SecondOverlapKey(TEXT("DirectiveUtilSecondOverlapStopwatch"));
	TestTrue(
		"StartStopwatch should start the first overlapping key",
		UDirectiveUtilFunctionLibrary::StartStopwatch(FirstOverlapKey));
	TestTrue(
		"StartStopwatch should start the second overlapping key",
		UDirectiveUtilFunctionLibrary::StartStopwatch(SecondOverlapKey));
	TestTrue(
		"StopStopwatch should stop the first overlapping key out of order",
		UDirectiveUtilFunctionLibrary::StopStopwatch(FirstOverlapKey, ElapsedMilliseconds));
	TestTrue(
		"StopStopwatch should stop the second overlapping key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(SecondOverlapKey, ElapsedMilliseconds));

	const FName TimedStopwatchKey(TEXT("DirectiveUtilTimedStopwatch"));
	TestTrue(
		"StartStopwatch should start the elapsed-time test key",
		UDirectiveUtilFunctionLibrary::StartStopwatch(TimedStopwatchKey));
	FPlatformProcess::SleepNoStats(0.01f);
	TestTrue(
		"StopStopwatch should stop the elapsed-time test key",
		UDirectiveUtilFunctionLibrary::StopStopwatch(TimedStopwatchKey, ElapsedMilliseconds));
	TestTrue("StopStopwatch should measure elapsed real time in milliseconds", ElapsedMilliseconds >= 5.0);

	const FName CrossThreadStopwatchKey(TEXT("DirectiveUtilCrossThreadStopwatch"));
	TestTrue(
		"StartStopwatch should start a key that will stop on another thread",
		UDirectiveUtilFunctionLibrary::StartStopwatch(CrossThreadStopwatchKey));
	double CrossThreadElapsedMilliseconds = 0.0;
	TFuture<bool> CrossThreadStop = Async(EAsyncExecution::ThreadPool, [&CrossThreadElapsedMilliseconds, CrossThreadStopwatchKey]()
	{
		return UDirectiveUtilFunctionLibrary::StopStopwatch(
			CrossThreadStopwatchKey,
			CrossThreadElapsedMilliseconds);
	});
	TestTrue("StopStopwatch should find a key started on another thread", CrossThreadStop.Get());
	TestTrue("Cross-thread stopwatch duration should be non-negative", CrossThreadElapsedMilliseconds >= 0.0);

	const UFunction* StartStopwatchFunction = UDirectiveUtilFunctionLibrary::StaticClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilFunctionLibrary, StartStopwatch));
	const UFunction* StopStopwatchFunction = UDirectiveUtilFunctionLibrary::StaticClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilFunctionLibrary, StopStopwatch));
	TestNotNull("StartStopwatch should be reflected", StartStopwatchFunction);
	TestNotNull("StopStopwatch should be reflected", StopStopwatchFunction);
	for (const UFunction* StopwatchFunction : { StartStopwatchFunction, StopStopwatchFunction })
	{
		if (!StopwatchFunction)
		{
			continue;
		}

		TestTrue("Stopwatch functions should be Blueprint callable", StopwatchFunction->HasAnyFunctionFlags(FUNC_BlueprintCallable));
		TestFalse("Stopwatch functions should not be Blueprint pure", StopwatchFunction->HasAnyFunctionFlags(FUNC_BlueprintPure));
		TestFalse("Stopwatch functions should be available at runtime", StopwatchFunction->HasAnyFunctionFlags(FUNC_EditorOnly));
#if WITH_EDITOR
		TestEqual(
			"Stopwatch functions should use the profiling category",
			StopwatchFunction->GetMetaData(TEXT("Category")),
			FString(TEXT("Directive Utilities|Utility|Profiling")));
#endif
	}

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
