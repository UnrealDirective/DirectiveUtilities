// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilEditorSlowTask.h"

#include "Misc/ScopedSlowTask.h"

void UDirectiveUtilEditorSlowTask::Initialize(
	const float TotalWork,
	const FText& Description,
	const bool bCanCancel,
	const bool bShowDialog)
{
	Finish();
	SlowTask = new FScopedSlowTask(TotalWork, Description);
	if (bShowDialog)
	{
		SlowTask->MakeDialog(bCanCancel, true);
	}
}

bool UDirectiveUtilEditorSlowTask::Advance(const float Work, const FText& Message)
{
	if (!SlowTask || Work < 0.0f)
	{
		return false;
	}

	SlowTask->EnterProgressFrame(Work, Message);
	return true;
}

bool UDirectiveUtilEditorSlowTask::IsCancelRequested() const
{
	return SlowTask && SlowTask->ShouldCancel();
}

bool UDirectiveUtilEditorSlowTask::IsActive() const
{
	return SlowTask != nullptr;
}

void UDirectiveUtilEditorSlowTask::Finish()
{
	delete SlowTask;
	SlowTask = nullptr;
}

void UDirectiveUtilEditorSlowTask::BeginDestroy()
{
	Finish();
	Super::BeginDestroy();
}
