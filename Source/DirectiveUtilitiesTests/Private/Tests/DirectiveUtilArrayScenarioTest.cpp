#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"

namespace
{
	FArrayProperty* GetIntegerArrayProperty()
	{
		return FindFProperty<FArrayProperty>(
			UDirectiveUtilTestObject::StaticClass(),
			GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));
	}

	TArray<int32> MakeSequentialValues(const int32 Count)
	{
		TArray<int32> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = Index;
		}
		return Values;
	}

	TArray<int32> MakeRepeatingValues(const int32 Count, const int32 DistinctCount)
	{
		TArray<int32> Values;
		Values.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values.Add(Index % DistinctCount);
		}
		return Values;
	}

	TArray<int32> MakeDistinctReference(const TArray<int32>& Values)
	{
		TArray<int32> Result;
		for (const int32 Value : Values)
		{
			Result.AddUnique(Value);
		}
		return Result;
	}

	bool FindMostCommonReference(const TArray<int32>& Values, int32& OutValue, int32& OutCount)
	{
		OutValue = 0;
		OutCount = 0;
		TMap<int32, int32> Counts;
		for (const int32 Value : Values)
		{
			++Counts.FindOrAdd(Value);
		}
		for (const TPair<int32, int32>& Pair : Counts)
		{
			OutCount = FMath::Max(OutCount, Pair.Value);
		}
		for (const int32 Value : Values)
		{
			if (Counts.FindRef(Value) == OutCount)
			{
				OutValue = Value;
				break;
			}
		}
		return !Values.IsEmpty();
	}

	int32 CountReference(const TArray<int32>& Values, const int32 QueryValue)
	{
		int32 Count = 0;
		for (const int32 Value : Values)
		{
			Count += Value == QueryValue ? 1 : 0;
		}
		return Count;
	}

	TArray<int32> SliceReference(const TArray<int32>& Values, const int32 StartIndex, const int32 Count)
	{
		TArray<int32> Result;
		Result.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Result.Add(Values[StartIndex + Index]);
		}
		return Result;
	}

	TArray<int32> MakeSampleReference(const int32 SourceCount, const int32 RequestedCount, const int32 Seed)
	{
		TArray<int32> AvailableIndices = MakeSequentialValues(SourceCount);
		TArray<int32> Result;
		const int32 SampleCount = FMath::Clamp(RequestedCount, 0, SourceCount);
		Result.Reserve(SampleCount);
		FRandomStream RandomStream(Seed);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			const int32 SelectedIndex = RandomStream.RandRange(0, AvailableIndices.Num() - 1);
			Result.Add(AvailableIndices[SelectedIndex]);
			AvailableIndices.RemoveAtSwap(SelectedIndex, 1, EAllowShrinking::No);
		}
		return Result;
	}

	template <typename ValueType>
	TArray<ValueType> MakeRotationReference(const TArray<ValueType>& Values, const int32 Shift)
	{
		if (Values.IsEmpty())
		{
			return Values;
		}

		int32 NormalizedShift = Shift % Values.Num();
		if (NormalizedShift < 0)
		{
			NormalizedShift += Values.Num();
		}

		TArray<ValueType> Result;
		Result.Reserve(Values.Num());
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			Result.Add(Values[(Index - NormalizedShift + Values.Num()) % Values.Num()]);
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayCardinalityTest,
	"DirectiveUtilities.ArrayScenarios.Cardinality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayCardinalityTest::RunTest(const FString& Parameters)
{
	FArrayProperty* ArrayProperty = GetIntegerArrayProperty();
	if (!TestNotNull("Integer array property should be available", ArrayProperty))
	{
		return false;
	}

	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();
	struct FScenario
	{
		int32 ItemCount;
		int32 DistinctCount;
	};
	const TArray<FScenario> Scenarios = {
		{0, 1},
		{1, 1},
		{2, 1},
		{3, 2},
		{16, 16},
		{31, 7},
		{64, 4},
		{257, 257},
		{1024, 1},
		{4096, 257},
		{16384, 1024}
	};

	for (const FScenario& Scenario : Scenarios)
	{
		const TArray<int32> Source = MakeRepeatingValues(Scenario.ItemCount, Scenario.DistinctCount);
		const TArray<int32> ExpectedDistinct = MakeDistinctReference(Source);
		const FString Label = FString::Printf(
			TEXT("items=%d distinct=%d"),
			Scenario.ItemCount,
			FMath::Min(Scenario.ItemCount, Scenario.DistinctCount));

		TestObject->TestArray = Source;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
		TestEqual(Label + TEXT(" RemoveDuplicates"), TestObject->TestArray, ExpectedDistinct);

		TArray<int32> DistinctResult;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(
			&Source,
			ArrayProperty,
			&DistinctResult,
			ArrayProperty);
		TestEqual(Label + TEXT(" GetDistinct"), DistinctResult, ExpectedDistinct);

		int32 MostCommonValue = INDEX_NONE;
		int32 MostCommonCount = INDEX_NONE;
		const bool bFoundMostCommon = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(
			&Source,
			ArrayProperty,
			&MostCommonValue,
			&MostCommonCount);
		if (Source.IsEmpty())
		{
			TestFalse(Label + TEXT(" GetMostCommon should fail"), bFoundMostCommon);
			TestEqual(Label + TEXT(" GetMostCommon count"), MostCommonCount, 0);
		}
		else
		{
			TestTrue(Label + TEXT(" GetMostCommon should succeed"), bFoundMostCommon);
			TestEqual(Label + TEXT(" GetMostCommon value"), MostCommonValue, 0);
			TestEqual(
				Label + TEXT(" GetMostCommon count"),
				MostCommonCount,
				FMath::DivideAndRoundUp(Scenario.ItemCount, Scenario.DistinctCount));
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArraySamplingScenarioTest,
	"DirectiveUtilities.ArrayScenarios.Sampling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArraySamplingScenarioTest::RunTest(const FString& Parameters)
{
	FArrayProperty* ArrayProperty = GetIntegerArrayProperty();
	if (!TestNotNull("Integer array property should be available", ArrayProperty))
	{
		return false;
	}

	constexpr int32 Seed = 7351;
	for (const int32 SourceCount : {0, 1, 2, 3, 4, 17, 100, 1000})
	{
		const TArray<int32> Source = MakeSequentialValues(SourceCount);
		const TArray<int32> RequestedCounts = {-1, 0, 1, SourceCount / 4, SourceCount, SourceCount + 5};
		for (const int32 RequestedCount : RequestedCounts)
		{
			FRandomStream FirstStream(Seed);
			FRandomStream SecondStream(Seed);
			TArray<int32> FirstSample;
			TArray<int32> SecondSample;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
				&Source,
				ArrayProperty,
				RequestedCount,
				false,
				&FirstStream,
				&FirstSample,
				ArrayProperty);
			UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
				&Source,
				ArrayProperty,
				RequestedCount,
				false,
				&SecondStream,
				&SecondSample,
				ArrayProperty);

			const FString Label = FString::Printf(TEXT("source=%d requested=%d"), SourceCount, RequestedCount);
			TestEqual(Label + TEXT(" count"), FirstSample.Num(), FMath::Clamp(RequestedCount, 0, SourceCount));
			TestEqual(Label + TEXT(" deterministic"), FirstSample, SecondSample);
			TSet<int32> UniqueValues;
			for (const int32 Value : FirstSample)
			{
				TestTrue(Label + TEXT(" source membership"), Source.Contains(Value));
				UniqueValues.Add(Value);
			}
			TestEqual(Label + TEXT(" uniqueness"), UniqueValues.Num(), FirstSample.Num());
		}
	}

	const TArray<int32> BoundarySource = MakeSequentialValues(100);
	for (const int32 RequestedCount : {24, 25, 26, 75, 100})
	{
		FRandomStream RandomStream(Seed);
		TArray<int32> Sample;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
			&BoundarySource,
			ArrayProperty,
			RequestedCount,
			false,
			&RandomStream,
			&Sample,
			ArrayProperty);
		TestEqual(
			FString::Printf(TEXT("threshold requested=%d"), RequestedCount),
			Sample,
			MakeSampleReference(BoundarySource.Num(), RequestedCount, Seed));
	}

	const TArray<int32> ReplacementSource = MakeSequentialValues(7);
	for (const int32 RequestedCount : {0, 1, 7, 14, 100})
	{
		FRandomStream FirstStream(Seed);
		FRandomStream SecondStream(Seed);
		TArray<int32> FirstSample;
		TArray<int32> SecondSample;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
			&ReplacementSource,
			ArrayProperty,
			RequestedCount,
			true,
			&FirstStream,
			&FirstSample,
			ArrayProperty);
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
			&ReplacementSource,
			ArrayProperty,
			RequestedCount,
			true,
			&SecondStream,
			&SecondSample,
			ArrayProperty);
		const FString Label = FString::Printf(TEXT("replacement requested=%d"), RequestedCount);
		TestEqual(Label + TEXT(" count"), FirstSample.Num(), RequestedCount);
		TestEqual(Label + TEXT(" deterministic"), FirstSample, SecondSample);
		for (const int32 Value : FirstSample)
		{
			TestTrue(Label + TEXT(" source membership"), ReplacementSource.Contains(Value));
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayRangeScenarioTest,
	"DirectiveUtilities.ArrayScenarios.RangesAndPages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayRangeScenarioTest::RunTest(const FString& Parameters)
{
	FArrayProperty* ArrayProperty = GetIntegerArrayProperty();
	if (!TestNotNull("Integer array property should be available", ArrayProperty))
	{
		return false;
	}

	struct FPageScenario
	{
		int32 ItemCount;
		int32 PageIndex;
		int32 PageSize;
		bool bExpectedValid;
		int32 ExpectedPageCount;
	};
	const TArray<FPageScenario> PageScenarios = {
		{0, 0, 1, false, 0},
		{1, 0, 1, true, 1},
		{2, 1, 1, true, 2},
		{5, 0, 2, true, 3},
		{5, 1, 2, true, 3},
		{5, 2, 2, true, 3},
		{5, 3, 2, false, 3},
		{10, 3, 3, true, 4},
		{10, 0, 10, true, 1},
		{10, 0, 11, true, 1},
		{10, -1, 3, false, 0},
		{10, 0, 0, false, 0},
		{10, 0, -1, false, 0},
		{10, MAX_int32, 3, false, 4}
	};

	for (const FPageScenario& Scenario : PageScenarios)
	{
		const TArray<int32> Source = MakeSequentialValues(Scenario.ItemCount);
		TArray<int32> Page;
		int32 PageCount = INDEX_NONE;
		const bool bValid = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(
			&Source,
			ArrayProperty,
			Scenario.PageIndex,
			Scenario.PageSize,
			&Page,
			ArrayProperty,
			&PageCount);
		const FString Label = FString::Printf(
			TEXT("items=%d page=%d size=%d"),
			Scenario.ItemCount,
			Scenario.PageIndex,
			Scenario.PageSize);
		TestEqual(Label + TEXT(" validity"), bValid, Scenario.bExpectedValid);
		TestEqual(Label + TEXT(" page count"), PageCount, Scenario.ExpectedPageCount);

		TArray<int32> ExpectedPage;
		if (Scenario.bExpectedValid)
		{
			const int32 StartIndex = Scenario.PageIndex * Scenario.PageSize;
			for (int32 Index = StartIndex; Index < FMath::Min(StartIndex + Scenario.PageSize, Source.Num()); ++Index)
			{
				ExpectedPage.Add(Source[Index]);
			}
		}
		TestEqual(Label + TEXT(" values"), Page, ExpectedPage);
	}

	const TArray<int32> SliceSource = MakeSequentialValues(10);
	struct FSliceScenario
	{
		int32 StartIndex;
		int32 Count;
	};
	const TArray<FSliceScenario> SliceScenarios = {
		{-5, 3},
		{0, -1},
		{0, 0},
		{0, 10},
		{0, 20},
		{5, 3},
		{9, 5},
		{10, 1},
		{11, 1},
		{MAX_int32, MAX_int32}
	};
	for (const FSliceScenario& Scenario : SliceScenarios)
	{
		const int32 StartIndex = FMath::Clamp(Scenario.StartIndex, 0, SliceSource.Num());
		const int32 CopyCount = Scenario.Count > 0
			? FMath::Min(Scenario.Count, SliceSource.Num() - StartIndex)
			: 0;
		TArray<int32> ExpectedSlice;
		for (int32 Offset = 0; Offset < CopyCount; ++Offset)
		{
			ExpectedSlice.Add(SliceSource[StartIndex + Offset]);
		}

		TArray<int32> Slice;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
			&SliceSource,
			ArrayProperty,
			Scenario.StartIndex,
			Scenario.Count,
			&Slice,
			ArrayProperty);
		const FString Label = FString::Printf(TEXT("start=%d count=%d"), Scenario.StartIndex, Scenario.Count);
		TestEqual(Label + TEXT(" separate output"), Slice, ExpectedSlice);

		TArray<int32> AliasedSlice = SliceSource;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
			&AliasedSlice,
			ArrayProperty,
			Scenario.StartIndex,
			Scenario.Count,
			&AliasedSlice,
			ArrayProperty);
		TestEqual(Label + TEXT(" aliased output"), AliasedSlice, ExpectedSlice);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayRotationScenarioTest,
	"DirectiveUtilities.ArrayScenarios.Rotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayRotationScenarioTest::RunTest(const FString& Parameters)
{
	FArrayProperty* IntegerArrayProperty = GetIntegerArrayProperty();
	FArrayProperty* StringArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringArray));
	if (!TestNotNull("Integer array property should be available", IntegerArrayProperty)
		|| !TestNotNull("String array property should be available", StringArrayProperty))
	{
		return false;
	}

	for (const int32 ItemCount : {0, 1, 2, 3, 4, 5, 16, 257, 10000})
	{
		const TArray<int32> Source = MakeSequentialValues(ItemCount);
		const TArray<int32> Shifts = {
			0,
			1,
			-1,
			ItemCount,
			ItemCount + 1,
			-ItemCount - 1,
			MAX_int32,
			MIN_int32
		};
		for (const int32 Shift : Shifts)
		{
			TArray<int32> Rotated = Source;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&Rotated, IntegerArrayProperty, Shift);
			TestEqual(
				FString::Printf(TEXT("POD items=%d shift=%d"), ItemCount, Shift),
				Rotated,
				MakeRotationReference(Source, Shift));
		}
	}

	for (const int32 ItemCount : {0, 1, 2, 5, 32})
	{
		TArray<FString> Source;
		Source.Reserve(ItemCount);
		for (int32 Index = 0; Index < ItemCount; ++Index)
		{
			Source.Add(FString::Printf(TEXT("Value%d"), Index));
		}
		for (const int32 Shift : {0, 1, -1, 7, -11, MAX_int32, MIN_int32})
		{
			TArray<FString> Rotated = Source;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&Rotated, StringArrayProperty, Shift);
			TestEqual(
				FString::Printf(TEXT("managed items=%d shift=%d"), ItemCount, Shift),
				Rotated,
				MakeRotationReference(Source, Shift));
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayOrderingScenarioTest,
	"DirectiveUtilities.ArrayScenarios.NaturalOrdering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayOrderingScenarioTest::RunTest(const FString& Parameters)
{
	for (const int32 ItemCount : {0, 1, 2, 10, 100, 1000})
	{
		TArray<FString> Strings;
		TArray<FString> ExpectedStrings;
		TArray<FName> Names;
		TArray<FName> ExpectedNames;
		Strings.Reserve(ItemCount);
		ExpectedStrings.Reserve(ItemCount);
		Names.Reserve(ItemCount);
		ExpectedNames.Reserve(ItemCount);
		for (int32 Index = 0; Index < ItemCount; ++Index)
		{
			ExpectedStrings.Add(FString::Printf(TEXT("Item%d"), Index));
			ExpectedNames.Add(FName(*FString::Printf(TEXT("Actor%d"), Index)));
		}
		for (int32 Index = ItemCount - 1; Index >= 0; --Index)
		{
			Strings.Add(ExpectedStrings[Index]);
			Names.Add(ExpectedNames[Index]);
		}

		UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(Strings);
		UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(Names);
		const FString Label = FString::Printf(TEXT("items=%d"), ItemCount);
		TestEqual(Label + TEXT(" string ascending"), Strings, ExpectedStrings);
		TestEqual(Label + TEXT(" name ascending"), Names, ExpectedNames);

		UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(Strings, true);
		UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(Names, true);
		Algo::Reverse(ExpectedStrings);
		Algo::Reverse(ExpectedNames);
		TestEqual(Label + TEXT(" string descending"), Strings, ExpectedStrings);
		TestEqual(Label + TEXT(" name descending"), Names, ExpectedNames);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayDeterministicFuzzTest,
	"DirectiveUtilities.ArrayScenarios.DeterministicFuzz",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayDeterministicFuzzTest::RunTest(const FString& Parameters)
{
	FArrayProperty* ArrayProperty = GetIntegerArrayProperty();
	if (!TestNotNull("Integer array property should be available", ArrayProperty))
	{
		return false;
	}

	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();
	const TArray<int32> ItemCounts = {0, 1, 2, 3, 7, 16, 31, 32, 127, 128, 1024, 4096};
	for (const int32 Seed : {17, 271, 4099, 65537, 104729})
	{
		for (int32 CountIndex = 0; CountIndex < ItemCounts.Num(); ++CountIndex)
		{
			const int32 ItemCount = ItemCounts[CountIndex];
			FRandomStream Stream(Seed + ItemCount * 31);
			TArray<int32> Source;
			Source.Reserve(ItemCount);
			for (int32 Index = 0; Index < ItemCount; ++Index)
			{
				Source.Add(Stream.RandRange(-64, 64));
			}
			const FString Label = FString::Printf(TEXT("seed=%d items=%d"), Seed, ItemCount);

			const TArray<int32> ExpectedDistinct = MakeDistinctReference(Source);
			TestObject->TestArray = Source;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
			TestEqual(Label + TEXT(" RemoveDuplicates"), TestObject->TestArray, ExpectedDistinct);

			TArray<int32> DistinctResult;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(
				&Source,
				ArrayProperty,
				&DistinctResult,
				ArrayProperty);
			TestEqual(Label + TEXT(" GetDistinct"), DistinctResult, ExpectedDistinct);

			int32 ExpectedMostCommon = 0;
			int32 ExpectedMostCommonCount = 0;
			const bool bExpectedMostCommon = FindMostCommonReference(
				Source,
				ExpectedMostCommon,
				ExpectedMostCommonCount);
			int32 MostCommon = 0;
			int32 MostCommonCount = 0;
			const bool bFoundMostCommon = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(
				&Source,
				ArrayProperty,
				&MostCommon,
				&MostCommonCount);
			TestEqual(Label + TEXT(" GetMostCommon result"), bFoundMostCommon, bExpectedMostCommon);
			TestEqual(Label + TEXT(" GetMostCommon value"), MostCommon, ExpectedMostCommon);
			TestEqual(Label + TEXT(" GetMostCommon count"), MostCommonCount, ExpectedMostCommonCount);

			const int32 QueryValue = Stream.RandRange(-70, 70);
			TestEqual(
				Label + TEXT(" CountOccurrences"),
				UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&Source, ArrayProperty, &QueryValue),
				CountReference(Source, QueryValue));

			for (const int32 Shift : {MIN_int32, -ItemCount - 1, -1, 0, 1, ItemCount + 1, MAX_int32})
			{
				TestObject->TestArray = Source;
				UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, Shift);
				TestEqual(
					Label + FString::Printf(TEXT(" Rotate shift=%d"), Shift),
					TestObject->TestArray,
					MakeRotationReference(Source, Shift));
			}

			const int32 StartIndex = Stream.RandRange(-ItemCount - 2, ItemCount + 2);
			const int32 SliceCount = Stream.RandRange(-2, ItemCount + 2);
			const int32 ExpectedStart = FMath::Clamp(StartIndex, 0, ItemCount);
			const int32 ExpectedSliceCount = FMath::Max(0, FMath::Min(SliceCount, ItemCount - ExpectedStart));
			const TArray<int32> ExpectedSlice = SliceReference(Source, ExpectedStart, ExpectedSliceCount);
			TArray<int32> SliceResult;
			UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
				&Source,
				ArrayProperty,
				StartIndex,
				SliceCount,
				&SliceResult,
				ArrayProperty);
			TestEqual(Label + TEXT(" Slice"), SliceResult, ExpectedSlice);

			const int32 PageSize = CountIndex % 4 == 0 ? 0 : 1 << (CountIndex % 7);
			const int32 ExpectedPageCount = PageSize > 0 ? FMath::DivideAndRoundUp(ItemCount, PageSize) : 0;
			for (const int32 PageIndex : {-1, 0, FMath::Max(0, ExpectedPageCount - 1), ExpectedPageCount})
			{
				TArray<int32> PageResult;
				int32 PageCount = -1;
				const bool bPageValid = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(
					&Source,
					ArrayProperty,
					PageIndex,
					PageSize,
					&PageResult,
					ArrayProperty,
					&PageCount);
				const bool bExpectedValid = PageSize > 0 && PageIndex >= 0 && PageIndex < ExpectedPageCount;
				const TArray<int32> ExpectedPage = bExpectedValid
					? SliceReference(Source, PageIndex * PageSize, FMath::Min(PageSize, ItemCount - PageIndex * PageSize))
					: TArray<int32>();
				const FString PageLabel = Label + FString::Printf(TEXT(" Page index=%d size=%d"), PageIndex, PageSize);
				TestEqual(PageLabel + TEXT(" validity"), bPageValid, bExpectedValid);
				TestEqual(
					PageLabel + TEXT(" count"),
					PageCount,
					PageSize > 0 && PageIndex >= 0 ? ExpectedPageCount : 0);
				TestEqual(PageLabel + TEXT(" values"), PageResult, ExpectedPage);
			}
		}
	}

	return true;
}
