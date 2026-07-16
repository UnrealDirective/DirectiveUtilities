#include "Libraries/DirectiveUtilInputFunctionLibrary.h"
#include "Types/DirectiveUtilTypes.h"
#include "InputMappingContext.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilInputFunctionLibraryTest, "DirectiveUtilities.InputFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilInputFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// The null-controller and invalid-context paths intentionally log warnings. Register them as
	// expected (plain match, negative count = consume if present, never required) so the run is clean.
	AddExpectedMessagePlain(TEXT("PlayerController is null. Cannot set input mapping contexts."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);
	AddExpectedMessagePlain(TEXT("Both the previous and new input mapping contexts must be valid."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// AddInputMappingContexts should return Failure with null controller
	TArray<FDirectiveUtilEnhancedInputContextData> Contexts;
	FDirectiveUtilEnhancedInputContextData Context;
	Contexts.Add(Context);
	TestEqual("AddInputMappingContexts should fail with null controller",
		UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(nullptr, Contexts, false),
		EDirectiveUtilSuccessStatus::Failure);

	// RemoveInputMappingContexts should return Failure with null controller
	TArray<TSoftObjectPtr<UInputMappingContext>> RemoveContexts;
	RemoveContexts.Add(nullptr);
	TestEqual("RemoveInputMappingContexts should fail with null controller",
		UDirectiveUtilInputFunctionLibrary::RemoveInputMappingContexts(nullptr, RemoveContexts),
		EDirectiveUtilSuccessStatus::Failure);

	// AddInputMappingContexts should return Failure with empty contexts
	TArray<FDirectiveUtilEnhancedInputContextData> EmptyContexts;
	TestEqual("AddInputMappingContexts should fail with empty contexts",
		UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(nullptr, EmptyContexts, false),
		EDirectiveUtilSuccessStatus::Failure);

	// RemoveInputMappingContexts should fail with empty contexts (early-return before touching the controller)
	TArray<TSoftObjectPtr<UInputMappingContext>> EmptyRemoveContexts;
	TestEqual("RemoveInputMappingContexts should fail with empty contexts",
		UDirectiveUtilInputFunctionLibrary::RemoveInputMappingContexts(nullptr, EmptyRemoveContexts),
		EDirectiveUtilSuccessStatus::Failure);

	// SwapInputMappingContexts loads both contexts before using the controller, so invalid
	// (unset) soft pointers must return Failure regardless of the (null) controller.
	const TSoftObjectPtr<UInputMappingContext> NullContext;
	TestEqual("SwapInputMappingContexts should fail when both contexts are invalid",
		UDirectiveUtilInputFunctionLibrary::SwapInputMappingContexts(nullptr, NullContext, NullContext, 0, false),
		EDirectiveUtilSuccessStatus::Failure);
	TestEqual("SwapInputMappingContexts should fail with invalid contexts even when using previous priority",
		UDirectiveUtilInputFunctionLibrary::SwapInputMappingContexts(nullptr, NullContext, NullContext, 5, true),
		EDirectiveUtilSuccessStatus::Failure);

	// New subsystem-getter helpers: null-controller safety (the active paths require a live player controller).
	TestNull("GetEnhancedInputSubsystem should return null for a null controller",
		UDirectiveUtilInputFunctionLibrary::GetEnhancedInputSubsystem(nullptr));
	TestFalse("IsInputMappingContextActive should return false for a null controller",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(nullptr, NullContext));
	TestEqual("ClearAllInputMappingContexts should fail for a null controller",
		UDirectiveUtilInputFunctionLibrary::ClearAllInputMappingContexts(nullptr),
		EDirectiveUtilSuccessStatus::Failure);

	return true;
}
