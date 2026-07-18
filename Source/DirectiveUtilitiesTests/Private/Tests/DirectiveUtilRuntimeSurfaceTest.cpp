// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Libraries/DirectiveUtilFunctionLibrary.h"
#include "Libraries/DirectiveUtilGameplayTagFunctionLibrary.h"
#include "Libraries/DirectiveUtilInputFunctionLibrary.h"
#include "Libraries/DirectiveUtilMapFunctionLibrary.h"
#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Libraries/DirectiveUtilRegexFunctionLibrary.h"
#include "Libraries/DirectiveUtilSaveGameFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Libraries/DirectiveUtilTextFunctionLibrary.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "Tasks/DirectiveUtilTask_AsyncLoadAsset.h"
#include "Tasks/DirectiveUtilTask_AsyncTrace.h"
#include "Tasks/DirectiveUtilTask_Delay.h"
#include "Tasks/DirectiveUtilTask_Flow.h"
#include "Tasks/DirectiveUtilTask_MoveToLocation.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilRuntimeSurfaceTest,
	"DirectiveUtilities.Runtime.Surface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilRuntimeSurfaceTest::RunTest(const FString& Parameters)
{
	struct FClassExpectation
	{
		UClass* Class;
		int32 FunctionCount;
	};

	const FClassExpectation Expectations[] = {
		{ UDirectiveUtilArrayFunctionLibrary::StaticClass(), 23 },
		{ UDirectiveUtilFunctionLibrary::StaticClass(), 13 },
		{ UDirectiveUtilGameplayTagFunctionLibrary::StaticClass(), 12 },
		{ UDirectiveUtilInputFunctionLibrary::StaticClass(), 6 },
		{ UDirectiveUtilMapFunctionLibrary::StaticClass(), 6 },
		{ UDirectiveUtilMathFunctionLibrary::StaticClass(), 23 },
		{ UDirectiveUtilRegexFunctionLibrary::StaticClass(), 5 },
		{ UDirectiveUtilSaveGameFunctionLibrary::StaticClass(), 7 },
		{ UDirectiveUtilStringFunctionLibrary::StaticClass(), 33 },
		{ UDirectiveUtilTextFunctionLibrary::StaticClass(), 1 },
		{ UDirectiveUtilTask_AsyncLoadAsset::StaticClass(), 2 },
		{ UDirectiveUtilTask_AsyncLoadClass::StaticClass(), 2 },
		{ UDirectiveUtilTask_AsyncLoadAssets::StaticClass(), 2 },
		{ UDirectiveUtilTask_AsyncTrace::StaticClass(), 4 },
		{ UDirectiveUtilTask_Delay::StaticClass(), 2 },
		{ UDirectiveUtilTask_UpdateForDuration::StaticClass(), 1 },
		{ UDirectiveUtilTask_RepeatWithInterval::StaticClass(), 1 },
		{ UDirectiveUtilTask_MoveToLocation::StaticClass(), 2 },
		{ UDirectiveUtilTask_MoveToActor::StaticClass(), 2 }
	};

	TestTrue(TEXT("The runtime module is loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesRuntime")));

#if !WITH_EDITOR
	TestFalse(TEXT("The editor module is absent from a game target"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesEditor")));
	TestFalse(TEXT("The uncooked node module is absent from a game target"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesBlueprintNodes")));
#endif

	int32 TotalFunctionCount = 0;
	for (const FClassExpectation& Expectation : Expectations)
	{
		TestNotNull(TEXT("Runtime class is reflected"), Expectation.Class);
		if (!Expectation.Class)
		{
			continue;
		}

		TestEqual(
			FString::Printf(TEXT("%s belongs to the runtime script package"), *Expectation.Class->GetName()),
			Expectation.Class->GetOutermost()->GetName(),
			FString(TEXT("/Script/DirectiveUtilitiesRuntime")));

		TestFalse(
			FString::Printf(TEXT("%s is not in an editor-only package"), *Expectation.Class->GetName()),
			Expectation.Class->GetOutermost()->HasAnyPackageFlags(PKG_EditorOnly | PKG_UncookedOnly | PKG_Developer));

		int32 ClassFunctionCount = 0;
		for (TFieldIterator<UFunction> FunctionIterator(Expectation.Class, EFieldIteratorFlags::ExcludeSuper); FunctionIterator; ++FunctionIterator)
		{
			const UFunction* Function = *FunctionIterator;
			if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintPure))
			{
				continue;
			}

			++ClassFunctionCount;
			TestFalse(
				FString::Printf(TEXT("%s.%s is available outside the editor"), *Expectation.Class->GetName(), *Function->GetName()),
				Function->HasAnyFunctionFlags(FUNC_EditorOnly));

#if WITH_EDITOR
			const FString Category = Function->GetMetaData(TEXT("Category"));
			TestTrue(
				FString::Printf(TEXT("%s.%s has a Directive Utilities category"), *Expectation.Class->GetName(), *Function->GetName()),
				Category == TEXT("Directive Utilities") || Category.StartsWith(TEXT("Directive Utilities|")));
#endif
		}

		TestEqual(
			FString::Printf(TEXT("%s exposes the expected runtime function count"), *Expectation.Class->GetName()),
			ClassFunctionCount,
			Expectation.FunctionCount);
		TotalFunctionCount += ClassFunctionCount;
	}

	TestEqual(TEXT("All 147 runtime functions are reflected"), TotalFunctionCount, 147);
	return !HasAnyErrors();
}
