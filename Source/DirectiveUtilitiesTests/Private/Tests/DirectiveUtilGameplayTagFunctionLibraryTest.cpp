#include "Libraries/DirectiveUtilGameplayTagFunctionLibrary.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilGameplayTagFunctionLibraryTest, "DirectiveUtilities.GameplayTagFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilGameplayTagFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// Native tag registration is unavailable to a DeveloperTool module (module-type validation)
	// and the legacy FName path ensures on 5.6/5.7 after startup. Register through a runtime
	// ini tag source instead, which supports late addition from any module.
	const FName TestTagName(TEXT("DirectiveUtilities.Test.Alpha.Beta"));
	const FName SiblingTagName(TEXT("DirectiveUtilities.Test.Alpha.Gamma"));
	const FName GrandchildTagName(TEXT("DirectiveUtilities.Test.Alpha.Beta.Delta"));
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TestTagName, false);
	FGameplayTag SiblingTag = FGameplayTag::RequestGameplayTag(SiblingTagName, false);
	FGameplayTag GrandchildTag = FGameplayTag::RequestGameplayTag(GrandchildTagName, false);
	if (!Tag.IsValid() || !SiblingTag.IsValid() || !GrandchildTag.IsValid())
	{
		// AddTagIniSearchPath will not rescan an already-added directory, so growing the
		// tag set requires a fresh directory name.
		const FString TagIniDirectory = FPaths::ProjectSavedDir() / TEXT("DirectiveUtilitiesTests") / TEXT("Tags");
		const FString TagIniPath = TagIniDirectory / TEXT("DirectiveUtilitiesTestTags.ini");
		const FString TagIniContents = TEXT("[/Script/GameplayTags.GameplayTagsList]\n")
			TEXT("GameplayTagList=(Tag=\"DirectiveUtilities.Test.Alpha.Beta\",DevComment=\"DirectiveUtilities automation test tag\")\n")
			TEXT("GameplayTagList=(Tag=\"DirectiveUtilities.Test.Alpha.Gamma\",DevComment=\"DirectiveUtilities automation test tag\")\n")
			TEXT("GameplayTagList=(Tag=\"DirectiveUtilities.Test.Alpha.Beta.Delta\",DevComment=\"DirectiveUtilities automation test tag\")\n");
		if (!FFileHelper::SaveStringToFile(TagIniContents, *TagIniPath))
		{
			AddError(TEXT("Failed to write the test tag ini file."));
			return false;
		}
		UGameplayTagsManager::Get().AddTagIniSearchPath(TagIniDirectory);
		Tag = FGameplayTag::RequestGameplayTag(TestTagName, false);
		SiblingTag = FGameplayTag::RequestGameplayTag(SiblingTagName, false);
		GrandchildTag = FGameplayTag::RequestGameplayTag(GrandchildTagName, false);
	}
	TestTrue("The registered test tag should be valid", Tag.IsValid());
	TestTrue("The registered sibling test tag should be valid", SiblingTag.IsValid());
	TestTrue("The registered grandchild test tag should be valid", GrandchildTag.IsValid());

	TestEqual("GetTagSegments should return each segment in order",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagSegments(Tag),
		TArray<FString>({TEXT("DirectiveUtilities"), TEXT("Test"), TEXT("Alpha"), TEXT("Beta")}));
	TestEqual("GetTagDepth should return the segment count", UDirectiveUtilGameplayTagFunctionLibrary::GetTagDepth(Tag), 4);
	TestEqual("GetTagLeafName should return the leaf segment", UDirectiveUtilGameplayTagFunctionLibrary::GetTagLeafName(Tag), FString(TEXT("Beta")));

	const FGameplayTag DirectParent = UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectParent(Tag);
	TestEqual("GetTagDirectParent should return the immediate parent", DirectParent.GetTagName(), FName(TEXT("DirectiveUtilities.Test.Alpha")));

	const FGameplayTagContainer Parents = UDirectiveUtilGameplayTagFunctionLibrary::GetTagParents(Tag);
	TestTrue("GetTagParents should contain the direct parent", Parents.HasTagExact(DirectParent));
	TestFalse("GetTagParents should exclude the tag itself", Parents.HasTagExact(Tag));

	const FGameplayTag InvalidTag;
	TestFalse("GetTagDirectParent of an invalid tag should be invalid", UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectParent(InvalidTag).IsValid());
	TestEqual("GetTagDepth of an invalid tag should be 0", UDirectiveUtilGameplayTagFunctionLibrary::GetTagDepth(InvalidTag), 0);
	TestTrue("GetTagLeafName of an invalid tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagLeafName(InvalidTag).IsEmpty());
	TestEqual("GetTagSegments of an invalid tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagSegments(InvalidTag).Num(), 0);

	const FGameplayTag AlphaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("DirectiveUtilities.Test.Alpha")), false);
	TestTrue("The auto-registered parent tag should be valid", AlphaTag.IsValid());

	const FGameplayTagContainer Children = UDirectiveUtilGameplayTagFunctionLibrary::GetTagChildren(AlphaTag);
	TestTrue("GetTagChildren should contain the first direct child", Children.HasTagExact(Tag));
	TestTrue("GetTagChildren should contain the second direct child", Children.HasTagExact(SiblingTag));
	TestTrue("GetTagChildren should contain a grandchild", Children.HasTagExact(GrandchildTag));

	const FGameplayTagContainer DirectChildren = UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectChildren(AlphaTag);
	TestTrue("GetTagDirectChildren should contain the first direct child", DirectChildren.HasTagExact(Tag));
	TestTrue("GetTagDirectChildren should contain the second direct child", DirectChildren.HasTagExact(SiblingTag));
	TestFalse("GetTagDirectChildren should exclude grandchildren", DirectChildren.HasTagExact(GrandchildTag));

	TestTrue("GetTagChildren of a leaf tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagChildren(GrandchildTag).IsEmpty());

	TestEqual("GetTagCommonAncestor should return the deepest shared ancestor",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagCommonAncestor(GrandchildTag, SiblingTag).GetTagName(), FName(TEXT("DirectiveUtilities.Test.Alpha")));
	TestEqual("GetTagCommonAncestor may return one of the inputs",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagCommonAncestor(Tag, GrandchildTag).GetTagName(), Tag.GetTagName());

	TestTrue("GetTagChildren of an invalid tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagChildren(InvalidTag).IsEmpty());
	TestTrue("GetTagDirectChildren of an invalid tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagDirectChildren(InvalidTag).IsEmpty());
	TestFalse("GetTagCommonAncestor of an invalid tag should be invalid", UDirectiveUtilGameplayTagFunctionLibrary::GetTagCommonAncestor(InvalidTag, Tag).IsValid());

	TestEqual("GetTagAtDepth should truncate to the requested depth",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(GrandchildTag, 3).GetTagName(), FName(TEXT("DirectiveUtilities.Test.Alpha")));
	TestEqual("GetTagAtDepth at the tag's own depth should return the tag",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(GrandchildTag, 5).GetTagName(), GrandchildTag.GetTagName());
	TestEqual("GetTagAtDepth beyond the tag's depth should return the tag",
		UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(GrandchildTag, 99).GetTagName(), GrandchildTag.GetTagName());
	TestFalse("GetTagAtDepth of zero should be invalid", UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(GrandchildTag, 0).IsValid());
	TestFalse("GetTagAtDepth of an invalid tag should be invalid", UDirectiveUtilGameplayTagFunctionLibrary::GetTagAtDepth(InvalidTag, 2).IsValid());

	const FGameplayTagContainer Siblings = UDirectiveUtilGameplayTagFunctionLibrary::GetTagSiblings(Tag);
	TestTrue("GetTagSiblings should contain the sibling", Siblings.HasTagExact(SiblingTag));
	TestFalse("GetTagSiblings should exclude the tag itself", Siblings.HasTagExact(Tag));
	TestFalse("GetTagSiblings should exclude the tag's children", Siblings.HasTagExact(GrandchildTag));
	TestTrue("GetTagSiblings of an invalid tag should be empty", UDirectiveUtilGameplayTagFunctionLibrary::GetTagSiblings(InvalidTag).IsEmpty());

	TestTrue("IsLeafTag should be true for a childless tag", UDirectiveUtilGameplayTagFunctionLibrary::IsLeafTag(GrandchildTag));
	TestFalse("IsLeafTag should be false for a tag with children", UDirectiveUtilGameplayTagFunctionLibrary::IsLeafTag(Tag));
	TestFalse("IsLeafTag of an invalid tag should be false", UDirectiveUtilGameplayTagFunctionLibrary::IsLeafTag(InvalidTag));

	const TArray<FGameplayTag> FoundTags = UDirectiveUtilGameplayTagFunctionLibrary::FindRegisteredTags(TEXT("test.alpha"));
	TestTrue("FindRegisteredTags should match case-insensitively", FoundTags.Contains(Tag));
	TestTrue("FindRegisteredTags should find every matching tag", FoundTags.Contains(SiblingTag) && FoundTags.Contains(GrandchildTag));
	TestEqual("FindRegisteredTags with no match should be empty", UDirectiveUtilGameplayTagFunctionLibrary::FindRegisteredTags(TEXT("DirectiveUtilitiesNoSuchTagXYZ")).Num(), 0);
	TestEqual("FindRegisteredTags with an empty substring should be empty", UDirectiveUtilGameplayTagFunctionLibrary::FindRegisteredTags(TEXT("")).Num(), 0);

	return true;
}
