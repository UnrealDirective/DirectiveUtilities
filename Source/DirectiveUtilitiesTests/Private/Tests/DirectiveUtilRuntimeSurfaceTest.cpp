// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Libraries/DirectiveUtilBytesFunctionLibrary.h"
#include "Libraries/DirectiveUtilConfigFunctionLibrary.h"
#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"
#include "Libraries/DirectiveUtilDataTableFunctionLibrary.h"
#include "Libraries/DirectiveUtilDateTimeFunctionLibrary.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
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
#include "Tasks/DirectiveUtilTask_FileSystem.h"
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
	};

	const FClassExpectation Expectations[] = {
		{ UDirectiveUtilArrayFunctionLibrary::StaticClass() },
		{ UDirectiveUtilBytesFunctionLibrary::StaticClass() },
		{ UDirectiveUtilConfigFunctionLibrary::StaticClass() },
		{ UDirectiveUtilCsvFunctionLibrary::StaticClass() },
		{ UDirectiveUtilDataTableFunctionLibrary::StaticClass() },
		{ UDirectiveUtilDateTimeFunctionLibrary::StaticClass() },
		{ UDirectiveUtilFileSystemFunctionLibrary::StaticClass() },
		{ UDirectiveUtilFunctionLibrary::StaticClass() },
		{ UDirectiveUtilGameplayTagFunctionLibrary::StaticClass() },
		{ UDirectiveUtilInputFunctionLibrary::StaticClass() },
		{ UDirectiveUtilMapFunctionLibrary::StaticClass() },
		{ UDirectiveUtilMathFunctionLibrary::StaticClass() },
		{ UDirectiveUtilRegexFunctionLibrary::StaticClass() },
		{ UDirectiveUtilSaveGameFunctionLibrary::StaticClass() },
		{ UDirectiveUtilStringFunctionLibrary::StaticClass() },
		{ UDirectiveUtilTextFunctionLibrary::StaticClass() },
		{ UDirectiveUtilTask_AsyncLoadAsset::StaticClass() },
		{ UDirectiveUtilTask_AsyncLoadClass::StaticClass() },
		{ UDirectiveUtilTask_AsyncLoadAssets::StaticClass() },
		{ UDirectiveUtilTask_ReadTextFile::StaticClass() },
		{ UDirectiveUtilTask_ReadBinaryFile::StaticClass() },
		{ UDirectiveUtilTask_WriteTextFile::StaticClass() },
		{ UDirectiveUtilTask_WriteBinaryFile::StaticClass() },
		{ UDirectiveUtilTask_WatchFile::StaticClass() },
		{ UDirectiveUtilTask_AsyncTrace::StaticClass() },
		{ UDirectiveUtilTask_Delay::StaticClass() },
		{ UDirectiveUtilTask_UpdateForDuration::StaticClass() },
		{ UDirectiveUtilTask_RepeatWithInterval::StaticClass() },
		{ UDirectiveUtilTask_MoveToLocation::StaticClass() },
		{ UDirectiveUtilTask_MoveToActor::StaticClass() }
	};

	TestTrue(TEXT("The runtime module is loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesRuntime")));

#if !WITH_EDITOR
	TestFalse(TEXT("The editor module is absent from a game target"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesEditor")));
	TestFalse(TEXT("The uncooked node module is absent from a game target"), FModuleManager::Get().IsModuleLoaded(TEXT("DirectiveUtilitiesBlueprintNodes")));
#endif

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

		for (TFieldIterator<UFunction> FunctionIterator(Expectation.Class, EFieldIteratorFlags::ExcludeSuper); FunctionIterator; ++FunctionIterator)
		{
			const UFunction* Function = *FunctionIterator;
			if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintPure))
			{
				continue;
			}

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
	}

	return !HasAnyErrors();
}
