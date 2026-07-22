// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesEditor.h"
#include "Tasks/DirectiveUtilEditorSlowTask.h"

void FDirectiveUtilitiesEditorModule::StartupModule()
{
}

void FDirectiveUtilitiesEditorModule::ShutdownModule()
{
	UDirectiveUtilEditorSlowTask::FinishActiveTask();
}

IMPLEMENT_MODULE(FDirectiveUtilitiesEditorModule, DirectiveUtilitiesEditor)
