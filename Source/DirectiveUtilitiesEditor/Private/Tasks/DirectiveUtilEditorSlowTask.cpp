// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilEditorSlowTask.h"

#include "Misc/ScopedSlowTask.h"

namespace
{
	UDirectiveUtilEditorSlowTask* ActiveSlowTask = nullptr;
}

void UDirectiveUtilEditorSlowTask::Initialize(
	const float TotalWork,
	const FText& Description,
	const bool bCanCancel,
	const bool bShowDialog)
{
	Finish();
	if (!FMath::IsFinite(TotalWork) || TotalWork <= 0.0f)
	{
		return;
	}
	if (ActiveSlowTask && ActiveSlowTask != this)
	{
		return;
	}

	SlowTask = new FScopedSlowTask(TotalWork, Description);
	ActiveSlowTask = this;
	if (bShowDialog)
	{
		SlowTask->MakeDialog(bCanCancel, true);
	}
}

bool UDirectiveUtilEditorSlowTask::Advance(const float Work, const FText& Message)
{
	if (!SlowTask || !FMath::IsFinite(Work) || Work < 0.0f)
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
	if (!SlowTask)
	{
		return;
	}

	delete SlowTask;
	SlowTask = nullptr;
	if (ActiveSlowTask == this)
	{
		ActiveSlowTask = nullptr;
	}
}

void UDirectiveUtilEditorSlowTask::BeginDestroy()
{
	Finish();
	Super::BeginDestroy();
}
