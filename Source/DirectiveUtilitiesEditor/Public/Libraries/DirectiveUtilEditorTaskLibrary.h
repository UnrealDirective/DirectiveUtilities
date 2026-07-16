// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilEditorTaskTypes.h"
#include "DirectiveUtilEditorTaskLibrary.generated.h"

class UDirectiveUtilEditorSlowTask;

UCLASS()
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorTaskLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Starts a modal editor progress task. TotalWork must be greater than zero. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Task")
	static UDirectiveUtilEditorSlowTask* StartEditorSlowTask(float TotalWork, const FText& Description, bool bCanCancel = false);

	/** Displays a fire-and-forget editor notification. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Notification")
	static bool ShowEditorNotification(const FText& Message, EDirectiveUtilEditorNotificationState State = EDirectiveUtilEditorNotificationState::Neutral, float ExpireDuration = 3.0f);
};
