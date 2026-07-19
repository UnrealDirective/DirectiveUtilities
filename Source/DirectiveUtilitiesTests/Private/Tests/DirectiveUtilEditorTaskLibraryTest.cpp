// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilEditorTaskLibrary.h"
#include "Misc/AutomationTest.h"
#include "Tasks/DirectiveUtilEditorSlowTask.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorTaskLibraryTest,
	"DirectiveUtilities.EditorTaskLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorTaskLibraryTest::RunTest(const FString& Parameters)
{
	TestNull("StartEditorSlowTask should reject zero work", UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(0.0f, FText::GetEmpty()));
	TestNull(
		"StartEditorSlowTask should reject NaN work",
		UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
			std::numeric_limits<float>::quiet_NaN(),
			FText::GetEmpty()));
	TestNull(
		"StartEditorSlowTask should reject infinite work",
		UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
			std::numeric_limits<float>::infinity(),
			FText::GetEmpty()));
	UDirectiveUtilEditorSlowTask* PublicTask = UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
		1.0f,
		FText::FromString(TEXT("Public task")));
	TestNotNull("StartEditorSlowTask should create a task for positive work", PublicTask);
	if (PublicTask)
	{
		TestNull(
			"StartEditorSlowTask should reject a nested task",
			UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
				1.0f,
				FText::FromString(TEXT("Nested task"))));
		PublicTask->Finish();
	}

	UDirectiveUtilEditorSlowTask* Task = NewObject<UDirectiveUtilEditorSlowTask>();
	Task->Initialize(2.0f, FText::FromString(TEXT("Test task")), false, false);
	TestTrue("Slow task should be active after initialization", Task->IsActive());
	TestTrue("Advance should accept non-negative work", Task->Advance(1.0f, FText::FromString(TEXT("Step"))));
	TestFalse("Advance should reject negative work", Task->Advance(-1.0f, FText::GetEmpty()));
	TestFalse("Advance should reject NaN work", Task->Advance(std::numeric_limits<float>::quiet_NaN(), FText::GetEmpty()));
	TestFalse("Advance should reject infinite work", Task->Advance(std::numeric_limits<float>::infinity(), FText::GetEmpty()));
	TestFalse("A non-cancellable task should not report cancellation", Task->IsCancelRequested());
	Task->Finish();
	TestFalse("Slow task should be inactive after Finish", Task->IsActive());
	TestFalse("Advance should fail after Finish", Task->Advance(1.0f, FText::GetEmpty()));
	Task->Initialize(std::numeric_limits<float>::quiet_NaN(), FText::GetEmpty(), false, false);
	TestFalse("Initialize should leave a task inactive for NaN work", Task->IsActive());
	Task->Initialize(std::numeric_limits<float>::infinity(), FText::GetEmpty(), false, false);
	TestFalse("Initialize should leave a task inactive for infinite work", Task->IsActive());

	UDirectiveUtilEditorSlowTask* FirstTask = NewObject<UDirectiveUtilEditorSlowTask>();
	UDirectiveUtilEditorSlowTask* SecondTask = NewObject<UDirectiveUtilEditorSlowTask>();
	FirstTask->Initialize(1.0f, FText::FromString(TEXT("First task")), false, false);
	SecondTask->Initialize(1.0f, FText::FromString(TEXT("Second task")), false, false);
	TestTrue("The first slow task should remain active", FirstTask->IsActive());
	TestFalse("A concurrent slow task should remain inactive", SecondTask->IsActive());
	FirstTask->Finish();
	SecondTask->Initialize(1.0f, FText::FromString(TEXT("Second task")), false, false);
	TestTrue("A slow task should start after the active task finishes", SecondTask->IsActive());
	SecondTask->Finish();

	TWeakObjectPtr<UDirectiveUtilEditorSlowTask> AbandonedTask = NewObject<UDirectiveUtilEditorSlowTask>();
	AbandonedTask->Initialize(1.0f, FText::FromString(TEXT("Abandoned task")), false, false);
	TestTrue("An unfinished slow task should be active before collection", AbandonedTask->IsActive());
	CollectGarbage(RF_NoFlags);
	TestFalse("An unfinished slow task should be collected safely", AbandonedTask.IsValid());
	UDirectiveUtilEditorSlowTask* TaskAfterCollection = NewObject<UDirectiveUtilEditorSlowTask>();
	TaskAfterCollection->Initialize(1.0f, FText::FromString(TEXT("Task after collection")), false, false);
	TestTrue("A slow task should start after an unfinished task is collected", TaskAfterCollection->IsActive());
	TaskAfterCollection->Finish();

	TestTrue(
		"ShowEditorNotification should create a notification while Slate is active",
		UDirectiveUtilEditorTaskLibrary::ShowEditorNotification(
			FText::FromString(TEXT("Directive Utilities test")),
			EDirectiveUtilEditorNotificationState::Success,
			0.01f));
	TestTrue(
		"ShowEditorNotification should normalize a non-finite duration",
		UDirectiveUtilEditorTaskLibrary::ShowEditorNotification(
			FText::FromString(TEXT("Directive Utilities duration test")),
			EDirectiveUtilEditorNotificationState::Neutral,
			std::numeric_limits<float>::quiet_NaN()));

	return true;
}
