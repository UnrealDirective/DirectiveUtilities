// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilBlueprintCategoryTest, "DirectiveUtilities.BlueprintCategoryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilBlueprintCategoryTest::RunTest(const FString& Parameters)
{
	const FString ExpectedCategoryRoot = TEXT("Directive Utilities");
	const FString RuntimeScriptPackage = TEXT("/Script/DirectiveUtilitiesRuntime");
	const TSet<FString> PluginScriptPackages = {
		RuntimeScriptPackage,
		TEXT("/Script/DirectiveUtilitiesEditor")
	};

	int32 TestedFunctionCount = 0;

	for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator)
	{
		UClass* Class = *ClassIterator;
		if (!PluginScriptPackages.Contains(Class->GetOutermost()->GetName()))
		{
			continue;
		}

		for (TFieldIterator<UFunction> FunctionIterator(Class, EFieldIteratorFlags::ExcludeSuper); FunctionIterator; ++FunctionIterator)
		{
			const UFunction* Function = *FunctionIterator;
			if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintPure))
			{
				continue;
			}

			++TestedFunctionCount;
			if (Class->GetOutermost()->GetName() == RuntimeScriptPackage)
			{
				TestFalse(
					FString::Printf(TEXT("%s.%s is available at runtime"), *Class->GetName(), *Function->GetName()),
					Class->GetOutermost()->HasAnyPackageFlags(PKG_EditorOnly | PKG_UncookedOnly | PKG_Developer)
						|| Function->HasAnyFunctionFlags(FUNC_EditorOnly)
				);
			}

			const FString Category = Function->GetMetaData(TEXT("Category"));
			FString CategoryRoot = Category;
			int32 CategoryDelimiterIndex = INDEX_NONE;
			if (CategoryRoot.FindChar(TEXT('|'), CategoryDelimiterIndex))
			{
				CategoryRoot.LeftInline(CategoryDelimiterIndex);
			}
			CategoryRoot.TrimStartAndEndInline();
			TestTrue(
				FString::Printf(TEXT("%s.%s uses the Directive Utilities category root"), *Class->GetName(), *Function->GetName()),
				CategoryRoot == ExpectedCategoryRoot
			);
		}
	}

	TestTrue(TEXT("Blueprint-exposed plugin functions were found"), TestedFunctionCount > 0);
	return !HasAnyErrors();
}
