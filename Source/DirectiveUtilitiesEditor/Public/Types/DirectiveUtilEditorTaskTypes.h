// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "DirectiveUtilEditorTaskTypes.generated.h"

UENUM(BlueprintType)
enum class EDirectiveUtilEditorNotificationState : uint8
{
	Neutral,
	Success,
	Warning,
	Failure
};
