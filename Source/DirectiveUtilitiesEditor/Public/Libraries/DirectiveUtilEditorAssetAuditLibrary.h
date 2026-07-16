// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilEditorAuditTypes.h"
#include "DirectiveUtilEditorAssetAuditLibrary.generated.h"

UCLASS()
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorAssetAuditLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns assets with no known Asset Registry referencers. Results are candidates and may still be loaded indirectly. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Asset Audit")
	static TArray<FAssetData> FindUnreferencedAssetCandidates(const FDirectiveUtilAssetAuditOptions& Options);

	/** Returns Asset Registry dependencies whose packages cannot be found. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Asset Audit")
	static TArray<FDirectiveUtilMissingAssetReference> FindMissingAssetReferences(const FDirectiveUtilAssetAuditOptions& Options);

	/** Returns dependency cycles between assets in the scanned paths. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Asset Audit")
	static TArray<FDirectiveUtilAssetDependencyCycle> FindAssetDependencyCycles(const FDirectiveUtilAssetAuditOptions& Options);

	/** Builds a read-only report for assets in the scanned paths. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Asset Audit")
	static FDirectiveUtilAssetAuditReport BuildAssetAuditReport(const FDirectiveUtilAssetAuditOptions& Options);

	/** Converts an asset audit report to CSV text. */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Editor|Asset Audit")
	static FString AssetAuditReportToCsv(const FDirectiveUtilAssetAuditReport& Report);
};
