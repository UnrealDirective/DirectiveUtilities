// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilEditorTaskLibrary.h"
#include "Misc/AutomationTest.h"
#include "Tasks/DirectiveUtilEditorSlowTask.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorTaskLibraryTest,
	"DirectiveUtilities.EditorTaskLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorTaskLibraryTest::RunTest(const FString& Parameters)
{
	TestNull("StartEditorSlowTask should reject zero work", UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(0.0f, FText::GetEmpty()));
	UDirectiveUtilEditorSlowTask* PublicTask = UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
		1.0f,
		FText::FromString(TEXT("Public task")));
	TestNotNull("StartEditorSlowTask should create a task for positive work", PublicTask);
	if (PublicTask)
	{
		PublicTask->Finish();
	}

	UDirectiveUtilEditorSlowTask* Task = NewObject<UDirectiveUtilEditorSlowTask>();
	Task->Initialize(2.0f, FText::FromString(TEXT("Test task")), false, false);
	TestTrue("Slow task should be active after initialization", Task->IsActive());
	TestTrue("Advance should accept non-negative work", Task->Advance(1.0f, FText::FromString(TEXT("Step"))));
	TestFalse("Advance should reject negative work", Task->Advance(-1.0f, FText::GetEmpty()));
	TestFalse("A non-cancellable task should not report cancellation", Task->IsCancelRequested());
	Task->Finish();
	TestFalse("Slow task should be inactive after Finish", Task->IsActive());
	TestFalse("Advance should fail after Finish", Task->Advance(1.0f, FText::GetEmpty()));

	TestTrue(
		"ShowEditorNotification should create a notification while Slate is active",
		UDirectiveUtilEditorTaskLibrary::ShowEditorNotification(
			FText::FromString(TEXT("Directive Utilities test")),
			EDirectiveUtilEditorNotificationState::Success,
			0.01f));

	return true;
}
