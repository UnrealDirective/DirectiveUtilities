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
	AddToRoot();
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

	const float CompletedWork = SlowTask->CompletedWork + SlowTask->CurrentFrameScope;
	const float RemainingWork = FMath::Max(0.0f, SlowTask->TotalAmountOfWork - CompletedWork);
	SlowTask->EnterProgressFrame(FMath::Min(Work, RemainingWork), Message);
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
	if (IsRooted())
	{
		RemoveFromRoot();
	}
}

void UDirectiveUtilEditorSlowTask::FinishActiveTask()
{
	if (ActiveSlowTask)
	{
		ActiveSlowTask->Finish();
	}
}
