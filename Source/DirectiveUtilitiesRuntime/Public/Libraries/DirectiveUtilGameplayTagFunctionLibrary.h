// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "DirectiveUtilGameplayTagFunctionLibrary.generated.h"

/**
 * UDirectiveUtilGameplayTagFunctionLibrary
 * Hierarchy navigation helpers for Gameplay Tags that the engine does not expose to Blueprints.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilGameplayTagFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns the direct (immediate) parent of a tag, e.g. "A.B.C" returns "A.B".
	 * @param Tag - The tag to read.
	 * @returns The direct parent tag, or an invalid tag if the tag is a root or invalid.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTag GetTagDirectParent(const FGameplayTag& Tag);

	/**
	 * Returns all ancestor tags of a tag (its parents, grandparents, etc.), excluding the tag itself.
	 * e.g. "A.B.C" returns { "A.B", "A" }.
	 * @param Tag - The tag to read.
	 * @returns A container of the tag's ancestors.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTagContainer GetTagParents(const FGameplayTag& Tag);

	/**
	 * Returns the number of segments (hierarchy depth) of a tag, e.g. "A.B.C" returns 3.
	 * @param Tag - The tag to read.
	 * @returns The depth, or 0 for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags", meta = (BlueprintThreadSafe))
	static int32 GetTagDepth(const FGameplayTag& Tag);

	/**
	 * Returns the last (leaf) segment of a tag's name, e.g. "A.B.C" returns "C".
	 * @param Tag - The tag to read.
	 * @returns The leaf segment, or an empty string for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags", meta = (BlueprintThreadSafe))
	static FString GetTagLeafName(const FGameplayTag& Tag);

	/**
	 * Splits a tag's name into its individual segments, e.g. "A.B.C" returns [ "A", "B", "C" ].
	 * @param Tag - The tag to read.
	 * @returns The segments in order, or an empty array for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags", meta = (BlueprintThreadSafe))
	static TArray<FString> GetTagSegments(const FGameplayTag& Tag);

	/**
	 * Returns all registered descendant tags of a tag (children, grandchildren, etc.), excluding the tag itself.
	 * e.g. "A" returns { "A.B", "A.B.C" } when those tags are registered.
	 * @param Tag - The tag to read.
	 * @returns A container of the tag's descendants, or an empty container for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTagContainer GetTagChildren(const FGameplayTag& Tag);

	/**
	 * Returns only the direct (immediate) registered children of a tag, e.g. "A" returns "A.B" but not "A.B.C".
	 * @param Tag - The tag to read.
	 * @returns A container of the tag's direct children, or an empty container for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTagContainer GetTagDirectChildren(const FGameplayTag& Tag);

	/**
	 * Returns the deepest tag that both tags share as an ancestor, e.g. "A.B.C" and "A.B.D" return "A.B".
	 * The result may be one of the inputs when one tag is an ancestor of the other.
	 * @param TagA - The first tag.
	 * @param TagB - The second tag.
	 * @returns The deepest common ancestor tag, or an invalid tag if the tags share none.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTag GetTagCommonAncestor(const FGameplayTag& TagA, const FGameplayTag& TagB);

	/**
	 * Truncates a tag to its first Depth segments, e.g. "A.B.C" at depth 2 returns "A.B".
	 * An ancestor of a registered tag is always registered itself.
	 * @param Tag - The tag to truncate.
	 * @param Depth - The number of leading segments to keep.
	 * @returns The truncated tag, the tag itself when Depth >= its depth, or an invalid tag when Depth < 1 or the tag is invalid.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTag GetTagAtDepth(const FGameplayTag& Tag, int32 Depth);

	/**
	 * Returns the registered tags that share a tag's direct parent, excluding the tag itself.
	 * Enumerating the siblings of a root tag (which has no parent) is not supported and returns an empty container.
	 * @param Tag - The tag to read.
	 * @returns A container of the tag's siblings, or an empty container for an invalid or root tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static FGameplayTagContainer GetTagSiblings(const FGameplayTag& Tag);

	/**
	 * Checks whether a tag has no registered children.
	 * @param Tag - The tag to test.
	 * @returns True if the tag is valid and has no registered descendants; false for an invalid tag.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static bool IsLeafTag(const FGameplayTag& Tag);

	/**
	 * Finds every registered gameplay tag whose name contains Substring (case-insensitive), in registry order.
	 * Cost scales with the size of the tag registry; intended for tooling and debug use, not per-frame calls.
	 * @param Substring - The text to search for. An empty string returns an empty array.
	 * @returns The matching tags.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|GameplayTags")
	static TArray<FGameplayTag> FindRegisteredTags(const FString& Substring);
};
