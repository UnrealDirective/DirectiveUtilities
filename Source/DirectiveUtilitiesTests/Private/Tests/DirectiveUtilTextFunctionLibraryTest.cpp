#include "Libraries/DirectiveUtilTextFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilTextFunctionLibraryTest, "DirectiveUtilities.TextFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilTextFunctionLibraryTest::RunTest(const FString& Parameters)
{
	const FText NonEmptyText = FText::FromString(TEXT("Hello"));
	TestTrue("IsNotEmpty should return true for non-empty text", UDirectiveUtilTextFunctionLibrary::IsNotEmpty(NonEmptyText));

	const FText EmptyText = FText::GetEmpty();
	TestFalse("IsNotEmpty should return false for empty text", UDirectiveUtilTextFunctionLibrary::IsNotEmpty(EmptyText));

	return true;
}
