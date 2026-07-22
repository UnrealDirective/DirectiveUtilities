// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

#include "Algo/Reverse.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProperties.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace DirectiveUtilRuntimePerformance
{
	struct FResult
	{
		FString Name;
		int32 ElementCount = 0;
		int32 Parameter = 0;
		double MedianMilliseconds = 0.0;
		double MinimumMilliseconds = 0.0;
		double MaximumMilliseconds = 0.0;
		int32 SampleCount = 0;

		FString GetKey() const
		{
			return FString::Printf(TEXT("%s|%d|%d"), *Name, ElementCount, Parameter);
		}
	};

	template <typename PrepareType, typename OperationType>
	FResult Measure(
		const FString& Name,
		const int32 ElementCount,
		const int32 Parameter,
		const int32 SampleCount,
		PrepareType&& Prepare,
		OperationType&& Operation)
	{
		Prepare();
		Operation();

		TArray<double> Samples;
		Samples.Reserve(SampleCount);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			Prepare();
			const uint64 StartCycles = FPlatformTime::Cycles64();
			Operation();
			const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
			Samples.Add(FPlatformTime::ToMilliseconds64(ElapsedCycles));
		}

		Samples.Sort();
		FResult Result;
		Result.Name = Name;
		Result.ElementCount = ElementCount;
		Result.Parameter = Parameter;
		Result.MedianMilliseconds = Samples[Samples.Num() / 2];
		Result.MinimumMilliseconds = Samples[0];
		Result.MaximumMilliseconds = Samples.Last();
		Result.SampleCount = SampleCount;
		return Result;
	}

	TArray<int32> MakeSequentialIntegers(const int32 Count)
	{
		TArray<int32> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = Index;
		}
		return Values;
	}

	TArray<int32> MakeShuffledIndices(const int32 Count)
	{
		TArray<int32> Indices = MakeSequentialIntegers(Count);
		FRandomStream RandomStream(1729);
		for (int32 Index = Count - 1; Index > 0; --Index)
		{
			Indices.Swap(Index, RandomStream.RandRange(0, Index));
		}
		return Indices;
	}

	TArray<int32> MakeRepeatingIntegers(const int32 Count, const int32 DistinctCount)
	{
		TArray<int32> Values;
		Values.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values.Add(Index % DistinctCount);
		}
		return Values;
	}

	TArray<FString> MakeNaturalSortStrings(const int32 Count)
	{
		const TArray<int32> Indices = MakeShuffledIndices(Count);
		TArray<FString> Values;
		Values.Reserve(Count);
		for (const int32 Index : Indices)
		{
			Values.Add(FString::Printf(TEXT("Item%d"), Index));
		}
		return Values;
	}

	TArray<FName> MakeNaturalSortNames(const int32 Count)
	{
		const TArray<int32> Indices = MakeShuffledIndices(Count);
		TArray<FName> Values;
		Values.Reserve(Count);
		for (const int32 Index : Indices)
		{
			Values.Add(FName(*FString::Printf(TEXT("Actor%d"), Index)));
		}
		return Values;
	}

	TArray<FString> MakeStringMatchCandidates(const int32 Count)
	{
		TArray<FString> Candidates;
		Candidates.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Candidates.Add(FString::Printf(TEXT("DirectiveUtilityCandidate%05d"), Index));
		}
		return Candidates;
	}

	FString GetOutputPath()
	{
		FString OutputPath;
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfOutput="), OutputPath))
		{
			OutputPath = FPaths::ProjectSavedDir() / TEXT("Automation/DirectiveUtilities/RuntimePerformance.csv");
		}
		return FPaths::ConvertRelativePathToFull(OutputPath);
	}

	bool LoadBaseline(TMap<FString, double>& OutMedians, FString& OutPath)
	{
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfBaseline="), OutPath))
		{
			return true;
		}

		OutPath = FPaths::ConvertRelativePathToFull(OutPath);
		FString Contents;
		if (!FFileHelper::LoadFileToString(Contents, *OutPath))
		{
			return false;
		}

		TArray<FString> Lines;
		Contents.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			if (Line.IsEmpty() || Line.StartsWith(TEXT("#")) || Line.StartsWith(TEXT("benchmark,")))
			{
				continue;
			}

			TArray<FString> Fields;
			Line.ParseIntoArray(Fields, TEXT(","), false);
			if (Fields.Num() < 4)
			{
				continue;
			}

			const FString Key = FString::Printf(
				TEXT("%s|%d|%d"),
				*Fields[0],
				FCString::Atoi(*Fields[1]),
				FCString::Atoi(*Fields[2]));
			OutMedians.Add(Key, FCString::Atod(*Fields[3]));
		}
		return true;
	}

	FString BuildCsv(
		const TArray<FResult>& Results,
		const TMap<FString, double>& BaselineMedians,
		const FString& BaselinePath)
	{
		FString Csv;
		Csv += FString::Printf(TEXT("#engine,%s\n"), *FEngineVersion::Current().ToString());
		Csv += FString::Printf(TEXT("#platform,%hs\n"), FPlatformProperties::PlatformName());
		Csv += FString::Printf(TEXT("#timestamp_utc,%s\n"), *FDateTime::UtcNow().ToIso8601());
		if (!BaselinePath.IsEmpty())
		{
			Csv += FString::Printf(TEXT("#baseline,%s\n"), *BaselinePath);
		}
		Csv += TEXT("benchmark,element_count,parameter,median_ms,min_ms,max_ms,samples,baseline_median_ms,speedup,change_percent\n");

		for (const FResult& Result : Results)
		{
			FString BaselineMedian;
			FString Speedup;
			FString ChangePercent;
			if (const double* Baseline = BaselineMedians.Find(Result.GetKey()))
			{
				BaselineMedian = FString::Printf(TEXT("%.9f"), *Baseline);
				if (*Baseline > 0.0 && Result.MedianMilliseconds > 0.0)
				{
					Speedup = FString::Printf(TEXT("%.4f"), *Baseline / Result.MedianMilliseconds);
					ChangePercent = FString::Printf(
						TEXT("%.2f"),
						((*Baseline - Result.MedianMilliseconds) / *Baseline) * 100.0);
				}
			}

			Csv += FString::Printf(
				TEXT("%s,%d,%d,%.9f,%.9f,%.9f,%d,%s,%s,%s\n"),
				*Result.Name,
				Result.ElementCount,
				Result.Parameter,
				Result.MedianMilliseconds,
				Result.MinimumMilliseconds,
				Result.MaximumMilliseconds,
				Result.SampleCount,
				*BaselineMedian,
				*Speedup,
				*ChangePercent);
		}
		return Csv;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilRuntimePerformanceTest,
	"Performance.DirectiveUtilities.Runtime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::PerfFilter)

bool FDirectiveUtilRuntimePerformanceTest::RunTest(const FString& Parameters)
{
	using namespace DirectiveUtilRuntimePerformance;

	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();
	FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));
	if (!TestNotNull(TEXT("Integer array property is available"), ArrayProperty))
	{
		return false;
	}

	constexpr int32 SampleCount = 7;
	TArray<FResult> Results;
	for (const int32 ElementCount : {16, 256, 1024, 4096, 16384})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		const int32 DenseDistinctCount = FMath::Max(1, FMath::Min(64, ElementCount / 4));
		const TArray<int32> DenseSource = MakeRepeatingIntegers(ElementCount, DenseDistinctCount);
		TArray<int32> Output;
		int32 MostCommonItem = INDEX_NONE;
		int32 MostCommonCount = 0;

		Results.Add(Measure(
			TEXT("RemoveDuplicatesUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetDistinctUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetMostCommonUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; MostCommonItem = INDEX_NONE; MostCommonCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &MostCommonItem, &MostCommonCount); }));

		Results.Add(Measure(
			TEXT("RemoveDuplicatesDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetDistinctDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; Output.Reset(); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetMostCommonDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; MostCommonItem = INDEX_NONE; MostCommonCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &MostCommonItem, &MostCommonCount); }));
	}

	for (const int32 ElementCount : {100, 1000, 10000, 100000})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		TArray<float> Weights;
		Weights.Reserve(ElementCount);
		for (int32 Index = 0; Index < ElementCount; ++Index)
		{
			Weights.Add(Index % 11 == 0 ? 0.0f : static_cast<float>((Index % 17) + 1));
		}
		TArray<int32> Output;
		FRandomStream RandomStream;

		Results.Add(Measure(
			TEXT("SampleWithoutReplacement"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 16, false, &RandomStream, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWeightedWithoutReplacement"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, 16, false, &RandomStream, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWeightedWithReplacement"), ElementCount, 256, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, 256, true, &RandomStream, &Output, ArrayProperty); }));
	}

	{
		constexpr int32 ElementCount = 100000;
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		TArray<float> Weights;
		Weights.Init(1.0f, ElementCount);
		TArray<int32> Output;
		FRandomStream RandomStream;
		int32 PageCount = 0;
		for (const int32 RequestedCount : {1, 16, 24999, 25000, 25001, 50000, 75000, 100000})
		{
			const TCHAR* BenchmarkName = RequestedCount == 75000
				? TEXT("SampleWithoutReplacementDense")
				: TEXT("SampleWithoutReplacementRatio");
			Results.Add(Measure(
				BenchmarkName, ElementCount, RequestedCount, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, RequestedCount, false, &RandomStream, &Output, ArrayProperty); }));
		}

		Results.Add(Measure(
			TEXT("SampleWithoutReplacementAlias"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 16, false, &RandomStream, &TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWithReplacement"), ElementCount, 256, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 256, true, &RandomStream, &Output, ArrayProperty); }));

		for (const int32 RequestedCount : {1, 16, 1000, 50000})
		{
			Results.Add(Measure(
				TEXT("SampleWeightedRatio"), ElementCount, RequestedCount, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, RequestedCount, false, &RandomStream, &Output, ArrayProperty); }));
		}

		for (const int32 PageSize : {1, 128, 4096})
		{
			const int32 PageIndex = PageSize == 128 ? 400 : (ElementCount / PageSize) / 2;
			Results.Add(Measure(
				TEXT("GetPage"), ElementCount, PageSize, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); PageCount = 0; },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, PageIndex, PageSize, &Output, ArrayProperty, &PageCount); }));
		}

		Results.Add(Measure(
			TEXT("GetPageAlias"), ElementCount, 128, SampleCount,
			[&]() { TestObject->TestArray = Source; PageCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, 400, 128, &TestObject->TestArray, ArrayProperty, &PageCount); }));

		Results.Add(Measure(
			TEXT("SliceAliasCorrectness"), ElementCount, 128, SampleCount,
			[&]() { TestObject->TestArray = Source; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 51200, 128, &TestObject->TestArray, ArrayProperty); }));

		int32 ItemToCount = ElementCount - 1;
		int32 OccurrenceCount = 0;
		Results.Add(Measure(
			TEXT("CountOccurrences"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; OccurrenceCount = 0; },
			[&]() { OccurrenceCount = UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&TestObject->TestArray, ArrayProperty, &ItemToCount); }));

	}

	for (const int32 ElementCount : {1000, 100000, 1000000})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		for (const int32 Shift : {1, ElementCount / 3, ElementCount - 1})
		{
			Results.Add(Measure(
				TEXT("Rotate"), ElementCount, Shift, SampleCount,
				[&]() { TestObject->TestArray = Source; },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, Shift); }));
		}
	}

	for (const int32 ElementCount : {100, 1000, 10000})
	{
		const TArray<FString> SourceStrings = MakeNaturalSortStrings(ElementCount);
		const TArray<FName> SourceNames = MakeNaturalSortNames(ElementCount);
		TArray<FString> WorkingStrings;
		TArray<FName> WorkingNames;

		Results.Add(Measure(
			TEXT("NaturalSortString"), ElementCount, 0, SampleCount,
			[&]() { WorkingStrings = SourceStrings; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(WorkingStrings); }));

		Results.Add(Measure(
			TEXT("NaturalSortName"), ElementCount, 0, SampleCount,
			[&]() { WorkingNames = SourceNames; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(WorkingNames); }));
	}

	for (const int32 ElementCount : {101, 1001, 100001})
	{
		const TArray<int32> ShuffledIndices = MakeShuffledIndices(ElementCount);
		TArray<float> ShuffledFloats;
		ShuffledFloats.Reserve(ElementCount);
		for (const int32 Value : ShuffledIndices)
		{
			ShuffledFloats.Add(static_cast<float>(Value) + 0.25f);
		}
		float MedianResult = 0.0f;

		Results.Add(Measure(
			TEXT("IntMedian"), ElementCount, 0, SampleCount,
			[]() {},
			[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(ShuffledIndices); }));

		Results.Add(Measure(
			TEXT("FloatMedian"), ElementCount, 0, SampleCount,
			[]() {},
			[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(ShuffledFloats); }));

		if (ElementCount == 100001)
		{
			const TArray<int32> SortedValues = MakeSequentialIntegers(ElementCount);
			TArray<int32> ReverseValues = SortedValues;
			Algo::Reverse(ReverseValues);
			Results.Add(Measure(
				TEXT("IntMedianSorted"), ElementCount, 0, SampleCount,
				[]() {},
				[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(SortedValues); }));
			Results.Add(Measure(
				TEXT("IntMedianReverse"), ElementCount, 0, SampleCount,
				[]() {},
				[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(ReverseValues); }));
		}
	}

	for (const int32 CandidateCount : {10, 1000, 10000})
	{
		const TArray<FString> Candidates = MakeStringMatchCandidates(CandidateCount);
		const FString Input = FString::Printf(TEXT("DirectiveUtilityCandidate%05dX"), CandidateCount - 1);
		float Similarity = 0.0f;
		int32 MatchIndex = INDEX_NONE;

		Results.Add(Measure(
			TEXT("FindBestStringMatch"), CandidateCount, Input.Len(), SampleCount,
			[&]() { Similarity = 0.0f; MatchIndex = INDEX_NONE; },
			[&]() { MatchIndex = UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(Input, Candidates, Similarity); }));
	}

	TMap<FString, double> BaselineMedians;
	FString BaselinePath;
	if (!LoadBaseline(BaselineMedians, BaselinePath))
	{
		AddError(FString::Printf(TEXT("Unable to read performance baseline: %s"), *BaselinePath));
		return false;
	}

	for (const FResult& Result : Results)
	{
		FString Comparison;
		if (const double* Baseline = BaselineMedians.Find(Result.GetKey()))
		{
			const double Speedup = Result.MedianMilliseconds > 0.0 ? *Baseline / Result.MedianMilliseconds : 0.0;
			const double ChangePercent = *Baseline > 0.0
				? ((*Baseline - Result.MedianMilliseconds) / *Baseline) * 100.0
				: 0.0;
			Comparison = FString::Printf(TEXT(" baseline=%.6fms speedup=%.3fx change=%.2f%%"), *Baseline, Speedup, ChangePercent);
		}
		AddInfo(FString::Printf(
			TEXT("RUNTIME_PERF %s elements=%d parameter=%d median=%.6fms min=%.6fms max=%.6fms samples=%d%s"),
			*Result.Name,
			Result.ElementCount,
			Result.Parameter,
			Result.MedianMilliseconds,
			Result.MinimumMilliseconds,
			Result.MaximumMilliseconds,
			Result.SampleCount,
			*Comparison));
	}

	const FString OutputPath = GetOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
	const FString Csv = BuildCsv(Results, BaselineMedians, BaselinePath);
	TestTrue(
		FString::Printf(TEXT("Performance results saved to %s"), *OutputPath),
		FFileHelper::SaveStringToFile(Csv, *OutputPath));
	return !HasAnyErrors();
}
