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
	return GetTagSegments(Tag).Num();
}

FString UDirectiveUtilGameplayTagFunctionLibrary::GetTagLeafName(const FGameplayTag& Tag)
{
	const TArray<FString> Segments = GetTagSegments(Tag);
	return Segments.Num() > 0 ? Segments.Last() : FString();
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

	const int32 DirectChildDepth = GetTagDepth(Tag) + 1;
	for (const FGameplayTag& Child : GetTagChildren(Tag))
	{
		if (GetTagDepth(Child) == DirectChildDepth)
		{
			DirectChildren.AddTag(Child);
		}
	}
	return DirectChildren;
}

FGameplayTag UDirectiveUtilGameplayTagFunctionLibrary::GetTagCommonAncestor(const FGameplayTag& TagA, const FGameplayTag& TagB)
{
	const TArray<FString> SegmentsA = GetTagSegments(TagA);
	const TArray<FString> SegmentsB = GetTagSegments(TagB);

	FString Prefix;
	for (int32 Index = 0; Index < SegmentsA.Num() && Index < SegmentsB.Num(); ++Index)
	{
		if (!SegmentsA[Index].Equals(SegmentsB[Index]))
		{
			break;
		}
		if (!Prefix.IsEmpty())
		{
			Prefix += TEXT(".");
		}
		Prefix += SegmentsA[Index];
	}

	if (Prefix.IsEmpty())
	{
		return FGameplayTag();
	}
	// A common prefix of two registered tags is itself registered (parents auto-register).
	return FGameplayTag::RequestGameplayTag(FName(*Prefix), false);
}

FGameplayTag UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(const FGameplayTag& Tag, const int32 Depth)
{
	if (!Tag.IsValid() || Depth < 1)
	{
		return FGameplayTag();
	}

	const TArray<FString> Segments = GetTagSegments(Tag);
	if (Depth >= Segments.Num())
	{
		return Tag;
	}

	FString Prefix = Segments[0];
	for (int32 Index = 1; Index < Depth; ++Index)
	{
		Prefix += TEXT(".");
		Prefix += Segments[Index];
	}
	// An ancestor of a registered tag is always registered itself (parents auto-register).
	return FGameplayTag::RequestGameplayTag(FName(*Prefix), false);
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
