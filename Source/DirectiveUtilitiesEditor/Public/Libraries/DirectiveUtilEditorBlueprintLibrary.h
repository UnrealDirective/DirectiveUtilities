// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilEditorBlueprintTypes.h"
#include "DirectiveUtilEditorBlueprintLibrary.generated.h"

class UBlueprint;

UCLASS()
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static EDirectiveUtilBlueprintCompileStatus GetBlueprintCompileStatus(const UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static TArray<FAssetData> FindBlueprintsByCompileStatus(
		EDirectiveUtilBlueprintCompileStatus CompileStatus,
		const FDirectiveUtilBlueprintSearchOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static TArray<FAssetData> FindBlueprintsByParentClass(
		UClass* ParentClass,
		const FDirectiveUtilBlueprintSearchOptions& Options,
		bool bIncludeDescendants = true);

	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static TArray<FAssetData> FindBlueprintsImplementingInterface(
		UClass* InterfaceClass,
		const FDirectiveUtilBlueprintSearchOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static TArray<FAssetData> FindBlueprintsContainingComponentClass(
		UClass* ComponentClass,
		const FDirectiveUtilBlueprintSearchOptions& Options,
		bool bIncludeDerivedComponents = true);

	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Editor|Blueprint Inspection")
	static TArray<FName> GetUnusedBlueprintVariables(UBlueprint* Blueprint);
};
