// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilEditorSlowTask.generated.h"

struct FScopedSlowTask;

UCLASS(BlueprintType)
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorSlowTask : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Task")
	bool Advance(float Work, const FText& Message);

	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Editor|Task")
	bool IsCancelRequested() const;

	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Editor|Task")
	bool IsActive() const;

	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Task")
	void Finish();

	void Initialize(float TotalWork, const FText& Description, bool bCanCancel, bool bShowDialog);
	static void FinishActiveTask();

private:
	FScopedSlowTask* SlowTask = nullptr;
};
