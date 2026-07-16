#include "Libraries/DirectiveUtilStringFunctionLibrary.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilStringCardinalityTest,
	"DirectiveUtilities.StringScenarios.Cardinality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilStringCardinalityTest::RunTest(const FString& Parameters)
{
	for (const int32 CharacterCount : {0, 1, 2, 31, 32, 255, 256, 1024})
	{
		FString Source;
		Source.Reserve(CharacterCount);
		for (int32 Index = 0; Index < CharacterCount; ++Index)
		{
			Source.AppendChar(TEXT('a'));
		}

		const FString Label = FString::Printf(TEXT("characters=%d"), CharacterCount);
		TestEqual(
			Label + TEXT(" identical"),
			UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(Source, Source),
			0);
		TestEqual(
			Label + TEXT(" empty comparison"),
			UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(Source, FString()),
			CharacterCount);

		if (!Source.IsEmpty())
		{
			for (const int32 ChangedIndex : {0, CharacterCount / 2, CharacterCount - 1})
			{
				FString Changed = Source;
				Changed[ChangedIndex] = TEXT('b');
				TestEqual(
					FString::Printf(TEXT("characters=%d changed=%d"), CharacterCount, ChangedIndex),
					UDirectiveUtilStringFunctionLibrary::GetLevenshteinDistance(Source, Changed),
					1);
			}
		}
	}

	for (const int32 CandidateCount : {0, 1, 2, 31, 32, 256, 1000})
	{
		TArray<FString> Candidates;
		Candidates.Reserve(CandidateCount);
		for (int32 Index = 0; Index < CandidateCount; ++Index)
		{
			Candidates.Add(FString::Printf(TEXT("Candidate%04d"), Index));
		}

		float Similarity = -1.0f;
		if (Candidates.IsEmpty())
		{
			TestEqual(
				"empty candidate set index",
				UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(TEXT("Candidate0000"), Candidates, Similarity),
				INDEX_NONE);
			TestEqual("empty candidate set similarity", Similarity, 0.0f);
			continue;
		}

		const TArray<int32> ExpectedIndices = {0, CandidateCount / 2, CandidateCount - 1};
		for (const int32 ExpectedIndex : ExpectedIndices)
		{
			Similarity = -1.0f;
			const int32 MatchIndex = UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(
				Candidates[ExpectedIndex],
				Candidates,
				Similarity);
			const FString Label = FString::Printf(TEXT("candidates=%d expected=%d"), CandidateCount, ExpectedIndex);
			TestEqual(Label + TEXT(" index"), MatchIndex, ExpectedIndex);
			TestEqual(Label + TEXT(" similarity"), Similarity, 1.0f);
		}
	}

	return true;
}
