// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilEditorTaskLibrary.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/AppStyle.h"
#include "Tasks/DirectiveUtilEditorSlowTask.h"
#include "Widgets/Notifications/SNotificationList.h"

UDirectiveUtilEditorSlowTask* UDirectiveUtilEditorTaskLibrary::StartEditorSlowTask(
	const float TotalWork,
	const FText& Description,
	const bool bCanCancel)
{
	if (TotalWork <= 0.0f)
	{
		return nullptr;
	}

	UDirectiveUtilEditorSlowTask* Task = NewObject<UDirectiveUtilEditorSlowTask>();
	Task->Initialize(TotalWork, Description, bCanCancel, true);
	return Task;
}

bool UDirectiveUtilEditorTaskLibrary::ShowEditorNotification(
	const FText& Message,
	const EDirectiveUtilEditorNotificationState State,
	const float ExpireDuration)
{
	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}

	FNotificationInfo NotificationInfo(Message);
	NotificationInfo.bFireAndForget = true;
	NotificationInfo.ExpireDuration = FMath::Max(0.0f, ExpireDuration);
	if (State == EDirectiveUtilEditorNotificationState::Warning)
	{
		NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
	}

	const TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	if (!Notification.IsValid())
	{
		return false;
	}

	switch (State)
	{
	case EDirectiveUtilEditorNotificationState::Success:
		Notification->SetCompletionState(SNotificationItem::CS_Success);
		break;
	case EDirectiveUtilEditorNotificationState::Failure:
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
		break;
	default:
		Notification->SetCompletionState(SNotificationItem::CS_None);
		break;
	}
	return true;
}
