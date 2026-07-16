// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilGameplayTagFunctionLibrary.h"
#include "GameplayTagsManager.h"

FGameplayTag UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectParent(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return FGameplayTag();
	}
	return Tag.RequestDirectParent();
}

FGameplayTagContainer UDirectiveUtilGameplayTagFunctionLibrary::GetTagParents(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return FGameplayTagContainer();
	}

	FGameplayTagContainer Parents = Tag.GetGameplayTagParents();
	Parents.RemoveTag(Tag);
	return Parents;
}

int32 UDirectiveUtilGameplayTagFunctionLibrary::GetTagDepth(const FGameplayTag& Tag)
{
	int32 Depth = 0;
	for (FGameplayTag Current = Tag; Current.IsValid(); Current = Current.RequestDirectParent())
	{
		++Depth;
	}
	return Depth;
}

FString UDirectiveUtilGameplayTagFunctionLibrary::GetTagLeafName(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return FString();
	}

	const FString TagString = Tag.GetTagName().ToString();
	int32 SeparatorIndex = INDEX_NONE;
	return TagString.FindLastChar(TEXT('.'), SeparatorIndex)
		? TagString.Mid(SeparatorIndex + 1)
		: TagString;
}

TArray<FString> UDirectiveUtilGameplayTagFunctionLibrary::GetTagSegments(const FGameplayTag& Tag)
{
	TArray<FString> Segments;
	if (!Tag.IsValid())
	{
		return Segments;
	}
	Tag.GetTagName().ToString().ParseIntoArray(Segments, TEXT("."), true);
	return Segments;
}

FGameplayTagContainer UDirectiveUtilGameplayTagFunctionLibrary::GetTagChildren(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return FGameplayTagContainer();
	}
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Tag);
}

FGameplayTagContainer UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectChildren(const FGameplayTag& Tag)
{
	FGameplayTagContainer DirectChildren;
	if (!Tag.IsValid())
	{
		return DirectChildren;
	}

	for (const FGameplayTag& Child : GetTagChildren(Tag))
	{
		if (Child.RequestDirectParent() == Tag)
		{
			DirectChildren.AddTag(Child);
		}
	}
	return DirectChildren;
}

FGameplayTag UDirectiveUtilGameplayTagFunctionLibrary::GetTagCommonAncestor(const FGameplayTag& TagA, const FGameplayTag& TagB)
{
	if (!TagA.IsValid() || !TagB.IsValid())
	{
		return FGameplayTag();
	}

	FGameplayTag AncestorA = TagA;
	FGameplayTag AncestorB = TagB;
	int32 DepthA = GetTagDepth(AncestorA);
	int32 DepthB = GetTagDepth(AncestorB);
	while (DepthA > DepthB)
	{
		AncestorA = AncestorA.RequestDirectParent();
		--DepthA;
	}
	while (DepthB > DepthA)
	{
		AncestorB = AncestorB.RequestDirectParent();
		--DepthB;
	}
	while (AncestorA.IsValid() && AncestorB.IsValid() && AncestorA != AncestorB)
	{
		AncestorA = AncestorA.RequestDirectParent();
		AncestorB = AncestorB.RequestDirectParent();
	}
	return AncestorA == AncestorB ? AncestorA : FGameplayTag();
}

FGameplayTag UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(const FGameplayTag& Tag, const int32 Depth)
{
	if (!Tag.IsValid() || Depth < 1)
	{
		return FGameplayTag();
	}

	const int32 TagDepth = GetTagDepth(Tag);
	if (Depth >= TagDepth)
	{
		return Tag;
	}

	FGameplayTag Ancestor = Tag;
	for (int32 CurrentDepth = TagDepth; CurrentDepth > Depth; --CurrentDepth)
	{
		Ancestor = Ancestor.RequestDirectParent();
	}
	return Ancestor;
}

FGameplayTagContainer UDirectiveUtilGameplayTagFunctionLibrary::GetTagSiblings(const FGameplayTag& Tag)
{
	const FGameplayTag DirectParent = GetTagDirectParent(Tag);
	if (!DirectParent.IsValid())
	{
		return FGameplayTagContainer();
	}

	FGameplayTagContainer Siblings = GetTagDirectChildren(DirectParent);
	Siblings.RemoveTag(Tag);
	return Siblings;
}

bool UDirectiveUtilGameplayTagFunctionLibrary::IsLeafTag(const FGameplayTag& Tag)
{
	return Tag.IsValid() && GetTagChildren(Tag).IsEmpty();
}

TArray<FGameplayTag> UDirectiveUtilGameplayTagFunctionLibrary::FindRegisteredTags(const FString& Substring)
{
	TArray<FGameplayTag> MatchingTags;
	if (Substring.IsEmpty())
	{
		return MatchingTags;
	}

	FGameplayTagContainer AllTags;
	UGameplayTagsManager::Get().RequestAllGameplayTags(AllTags, /*OnlyIncludeDictionaryTags*/ false);
	for (const FGameplayTag& RegisteredTag : AllTags)
	{
		if (RegisteredTag.GetTagName().ToString().Contains(Substring))
		{
			MatchingTags.Add(RegisteredTag);
		}
	}
	return MatchingTags;
}
