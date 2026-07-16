#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Misc/AutomationTest.h"

namespace
{
	TArray<int32> BuildReferenceSample(const int32 SourceCount, const int32 Count, const int32 Seed)
	{
		TArray<int32> AvailableIndices;
		AvailableIndices.SetNumUninitialized(SourceCount);
		for (int32 Index = 0; Index < SourceCount; ++Index)
		{
			AvailableIndices[Index] = Index;
		}

		FRandomStream RandomStream(Seed);
		TArray<int32> Result;
		const int32 SampleCount = FMath::Min(SourceCount, Count);
		Result.Reserve(SampleCount);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			const int32 SelectedIndex = RandomStream.RandRange(0, AvailableIndices.Num() - 1);
			Result.Add(AvailableIndices[SelectedIndex]);
			AvailableIndices.RemoveAtSwap(SelectedIndex, 1, EAllowShrinking::No);
		}
		return Result;
	}

	TArray<int32> BuildReferenceDistinct(const TArray<int32>& Values)
	{
		TArray<int32> Result;
		for (const int32 Value : Values)
		{
			if (!Result.Contains(Value))
			{
				Result.Add(Value);
			}
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilArrayFunctionLibraryTest, "DirectiveUtilities.ArrayFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayFunctionLibraryTest::RunTest(const FString& Parameters)
{
	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();

	TestObject->TestArray = {1, 2, 3, 4, 5};

	FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));

	const int32 NextIndex = UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 2, false);
	const int32 PreviousIndex = UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 2, false);

	TestEqual("Array_NextIndex should return the next index in the array", NextIndex, 3);
	TestEqual("Array_PreviousIndex should return the previous index in the array", PreviousIndex, 1);

	const int32 NextIndexLooped = UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 4, true);
	const int32 PreviousIndexLooped = UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 0, true);

	TestEqual("Array_NextIndex should return the first index when looping", NextIndexLooped, 0);
	TestEqual("Array_PreviousIndex should return the last index when looping", PreviousIndexLooped, 4);

	const int32 NextIndexNonLooped = UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 4, false);
	const int32 PreviousIndexNonLooped = UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 0, false);

	TestEqual("Array_NextIndex should return the last index when not looping", NextIndexNonLooped, 4);
	TestEqual("Array_PreviousIndex should return the first index when not looping", PreviousIndexNonLooped, 0);

	const int32 NextIndexOutOfBounds = UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 5, false);
	const int32 PreviousIndexOutOfBounds = UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, -1, false);

	TestEqual("Array_NextIndex should return the last index when out of bounds", NextIndexOutOfBounds, 4);
	TestEqual("Array_PreviousIndex should return the first index when out of bounds", PreviousIndexOutOfBounds, 0);

	TestEqual("Array_NextIndex should clamp a negative input index to 0 (no loop)",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, -2, false), 0);
	TestEqual("Array_NextIndex should clamp a negative input index to 0 (loop)",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, -2, true), 0);
	TestEqual("Array_NextIndex should wrap a large input index to 0 when looping",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 10, true), 0);
	TestEqual("Array_NextIndex should clamp a large input index to the last index when not looping",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 10, false), 4);
	TestEqual("Array_PreviousIndex should clamp a large input index to the last index (no loop)",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 10, false), 4);
	TestEqual("Array_PreviousIndex should clamp a large input index to the last index (loop)",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 10, true), 4);
	TestEqual("Array_PreviousIndex should wrap a negative input index to the last index when looping",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, -1, true), 4);
	TestEqual("Array_NextIndex should handle the maximum integer index",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, MAX_int32, false), 4);
	TestEqual("Array_PreviousIndex should handle the minimum integer index",
		UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, MIN_int32, false), 0);

	TestObject->TestArray = {1, 2, 3, 4, 5};

	int32 FirstItem = -1;
	const bool bGotFirst = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetFirstItem(&TestObject->TestArray, ArrayProperty, &FirstItem);
	TestTrue("GetFirstItem should succeed on a non-empty array", bGotFirst);
	TestEqual("GetFirstItem should return the first element", FirstItem, 1);

	int32 LastItem = -1;
	const bool bGotLast = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetLastItem(&TestObject->TestArray, ArrayProperty, &LastItem);
	TestTrue("GetLastItem should succeed on a non-empty array", bGotLast);
	TestEqual("GetLastItem should return the last element", LastItem, 5);

	int32 IndexItem = -1;
	const bool bGotIndex = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetItemAtIndex(&TestObject->TestArray, ArrayProperty, 2, &IndexItem);
	TestTrue("GetItemAtIndex should succeed for a valid index", bGotIndex);
	TestEqual("GetItemAtIndex should return the element at the index", IndexItem, 3);

	int32 OutOfRangeItem = 777;
	const bool bGotOutOfRange = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetItemAtIndex(&TestObject->TestArray, ArrayProperty, 10, &OutOfRangeItem);
	TestFalse("GetItemAtIndex should fail for an out-of-range index", bGotOutOfRange);
	TestEqual("GetItemAtIndex should reset the output to default on failure", OutOfRangeItem, 0);

	int32 RandomItem = -1;
	int32 RandomIndex = -1;
	const bool bGotRandom = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetRandomItem(&TestObject->TestArray, ArrayProperty, &RandomItem, &RandomIndex);
	TestTrue("GetRandomItem should succeed on a non-empty array", bGotRandom);
	TestTrue("GetRandomItem should return a valid index", TestObject->TestArray.IsValidIndex(RandomIndex));
	if (TestObject->TestArray.IsValidIndex(RandomIndex))
	{
		TestEqual("GetRandomItem item should match the element at the returned index", RandomItem, TestObject->TestArray[RandomIndex]);
	}

	TestObject->TestArray.Empty();

	int32 EmptyItem = 999;
	TestFalse("GetFirstItem should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetFirstItem(&TestObject->TestArray, ArrayProperty, &EmptyItem));
	TestEqual("GetFirstItem should reset the output to default on an empty array", EmptyItem, 0);

	EmptyItem = 999;
	TestFalse("GetLastItem should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetLastItem(&TestObject->TestArray, ArrayProperty, &EmptyItem));
	TestEqual("GetLastItem should reset the output to default on an empty array", EmptyItem, 0);

	int32 EmptyRandomItem = 999;
	int32 EmptyRandomIndex = 5;
	TestFalse("GetRandomItem should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetRandomItem(&TestObject->TestArray, ArrayProperty, &EmptyRandomItem, &EmptyRandomIndex));
	TestEqual("GetRandomItem should return INDEX_NONE on an empty array", EmptyRandomIndex, static_cast<int32>(INDEX_NONE));
	TestEqual("GetRandomItem should reset the output to default on an empty array", EmptyRandomItem, 0);

	TestObject->TestArray = {1, 2, 3};
	int32 PoppedItem = -1;
	const bool bPopped = UDirectiveUtilArrayFunctionLibrary::GenericArray_Pop(&TestObject->TestArray, ArrayProperty, &PoppedItem);
	TestTrue("Pop should succeed on a non-empty array", bPopped);
	TestEqual("Pop should return the last element", PoppedItem, 3);
	TestEqual("Pop should shrink the array by one", TestObject->TestArray.Num(), 2);
	TestEqual("Pop should leave the new last element intact", TestObject->TestArray.Last(), 2);

	TestObject->TestArray = {1, 2, 3};
	int32 PoppedFirst = -1;
	const bool bPoppedFirst = UDirectiveUtilArrayFunctionLibrary::GenericArray_PopFirst(&TestObject->TestArray, ArrayProperty, &PoppedFirst);
	TestTrue("PopFirst should succeed on a non-empty array", bPoppedFirst);
	TestEqual("PopFirst should return the first element", PoppedFirst, 1);
	TestEqual("PopFirst should shrink the array by one", TestObject->TestArray.Num(), 2);
	TestEqual("PopFirst should shift the remaining elements down", TestObject->TestArray[0], 2);

	TestObject->TestArray.Empty();
	int32 PoppedEmpty = 888;
	TestFalse("Pop should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_Pop(&TestObject->TestArray, ArrayProperty, &PoppedEmpty));
	TestEqual("Pop should reset the output to default on an empty array", PoppedEmpty, 0);
	TestFalse("PopFirst should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_PopFirst(&TestObject->TestArray, ArrayProperty, &PoppedEmpty));

	TestObject->TestArray = {10, 20, 30, 40};
	const bool bRemoved = UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAtSwap(&TestObject->TestArray, ArrayProperty, 1);
	TestTrue("RemoveAtSwap should succeed for a valid index", bRemoved);
	TestEqual("RemoveAtSwap should shrink the array by one", TestObject->TestArray.Num(), 3);
	TestFalse("RemoveAtSwap should have removed the target element", TestObject->TestArray.Contains(20));
	TestEqual("RemoveAtSwap should move the previously-last element into the removed slot", TestObject->TestArray[1], 40);

	const bool bRemovedOutOfRange = UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAtSwap(&TestObject->TestArray, ArrayProperty, 99);
	TestFalse("RemoveAtSwap should fail for an out-of-range index", bRemovedOutOfRange);
	TestEqual("RemoveAtSwap should not change the array on failure", TestObject->TestArray.Num(), 3);

	TestObject->TestArray = {10, 20, 30};
	const bool bRemovedLast = UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAtSwap(&TestObject->TestArray, ArrayProperty, 2);
	TestTrue("RemoveAtSwap should succeed when removing the last element", bRemovedLast);
	TestEqual("RemoveAtSwap on the last element should shrink the array", TestObject->TestArray.Num(), 2);
	TestEqual("RemoveAtSwap on the last element should preserve the order of the rest", TestObject->TestArray[1], 20);

	TestObject->TestArray = {1, 2, 2, 3, 1, 4};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
	TestEqual("RemoveDuplicates should remove all duplicate entries", TestObject->TestArray.Num(), 4);
	if (TestObject->TestArray.Num() == 4)
	{
		TestEqual("RemoveDuplicates should keep the first occurrence (index 0)", TestObject->TestArray[0], 1);
		TestEqual("RemoveDuplicates should keep the first occurrence (index 1)", TestObject->TestArray[1], 2);
		TestEqual("RemoveDuplicates should keep the first occurrence (index 2)", TestObject->TestArray[2], 3);
		TestEqual("RemoveDuplicates should keep the first occurrence (index 3)", TestObject->TestArray[3], 4);
	}

	TestObject->TestArray = {5, 5, 5};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
	TestEqual("RemoveDuplicates should collapse an all-duplicates array to one element", TestObject->TestArray.Num(), 1);
	if (TestObject->TestArray.Num() == 1)
	{
		TestEqual("RemoveDuplicates should keep the single remaining value", TestObject->TestArray[0], 5);
	}

	TestObject->TestArray.Empty();
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
	TestEqual("RemoveDuplicates on an empty array should leave it empty", TestObject->TestArray.Num(), 0);

	TestObject->TestArray = {10, 20, 30, 40, 50};
	TArray<int32> SliceOut;
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 1, 3, &SliceOut, ArrayProperty);
	TestEqual("Slice should copy a contiguous range", SliceOut, TArray<int32>({20, 30, 40}));
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 3, 99, &SliceOut, ArrayProperty);
	TestEqual("Slice should clamp Count to the available elements", SliceOut, TArray<int32>({40, 50}));
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, -5, 2, &SliceOut, ArrayProperty);
	TestEqual("Slice should clamp a negative start index to 0", SliceOut, TArray<int32>({10, 20}));
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 0, 0, &SliceOut, ArrayProperty);
	TestEqual("Slice with Count 0 should be empty", SliceOut.Num(), 0);
	TestObject->TestArray = {10, 20, 30, 40};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 1, 2, &TestObject->TestArray, ArrayProperty);
	TestEqual("Slice should support using the source array as its output", TestObject->TestArray, TArray<int32>({20, 30}));

	TestObject->TestArray = {1, 2, 3, 4, 5};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, 2);
	TestEqual("Rotate by +2 should rotate toward the end", TestObject->TestArray, TArray<int32>({4, 5, 1, 2, 3}));
	TestObject->TestArray = {1, 2, 3, 4, 5};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, -1);
	TestEqual("Rotate by -1 should rotate toward the start", TestObject->TestArray, TArray<int32>({2, 3, 4, 5, 1}));
	TestObject->TestArray = {1, 2, 3};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, 3);
	TestEqual("Rotate by Length should be a no-op", TestObject->TestArray, TArray<int32>({1, 2, 3}));

	TestObject->TestArray = {1, 2, 2, 3, 1, 4};
	TArray<int32> DistinctOut;
	UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &DistinctOut, ArrayProperty);
	TestEqual("GetDistinct should keep first occurrences in order", DistinctOut, TArray<int32>({1, 2, 3, 4}));
	TestEqual("GetDistinct should not modify the source array", TestObject->TestArray.Num(), 6);
	TestObject->TestArray = {1, 2, 2, 3, 1};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetDistinct should support using the source array as its output", TestObject->TestArray, TArray<int32>({1, 2, 3}));

	FArrayProperty* DistinctStringArrayProperty = FindFProperty<FArrayProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringArray));
	TestNotNull("TestStringArray property should be found", DistinctStringArrayProperty);
	if (DistinctStringArrayProperty)
	{
		TestObject->TestStringArray = {TEXT("Alpha"), TEXT("Beta"), TEXT("Beta")};
		UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(
			&TestObject->TestStringArray,
			DistinctStringArrayProperty,
			&TestObject->TestStringArray,
			DistinctStringArrayProperty);
		TestEqual("GetDistinct should preserve in-place string values", TestObject->TestStringArray, TArray<FString>({TEXT("Alpha"), TEXT("Beta")}));
	}

	TestObject->TestArray = {5, 1, 5, 2, 5, 3};
	int32 ItemToCount = 5;
	TestEqual("CountOccurrences should count matches", UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&TestObject->TestArray, ArrayProperty, &ItemToCount), 3);
	int32 MissingItem = 99;
	TestEqual("CountOccurrences should return 0 for an absent item", UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&TestObject->TestArray, ArrayProperty, &MissingItem), 0);

	TestObject->TestArray = {7, 7, 8, 7, 9, 8};
	int32 MostCommonItem = -1;
	int32 MostCommonCount = -1;
	const bool bGotMostCommon = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &MostCommonItem, &MostCommonCount);
	TestTrue("GetMostCommon should succeed on a non-empty array", bGotMostCommon);
	TestEqual("GetMostCommon should return the most frequent element", MostCommonItem, 7);
	TestEqual("GetMostCommon should return the occurrence count", MostCommonCount, 3);
	TestObject->TestArray.Empty();
	int32 EmptyMostItem = 5;
	int32 EmptyMostCount = 5;
	TestFalse("GetMostCommon should fail on an empty array", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &EmptyMostItem, &EmptyMostCount));
	TestEqual("GetMostCommon should reset the count on an empty array", EmptyMostCount, 0);

	const TArray<FName> MutatingFunctions = {
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveDuplicates),
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Pop),
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_PopFirst),
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveAtSwap),
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Rotate)
	};
	for (const FName FunctionName : MutatingFunctions)
	{
		const UFunction* Function = UDirectiveUtilArrayFunctionLibrary::StaticClass()->FindFunctionByName(FunctionName);
		const FArrayProperty* TargetArrayProperty = Function ? FindFProperty<FArrayProperty>(Function, TEXT("TargetArray")) : nullptr;
		TestNotNull(*FString::Printf(TEXT("%s should expose a TargetArray parameter"), *FunctionName.ToString()), TargetArrayProperty);
		if (TargetArrayProperty)
		{
			TestFalse(*FString::Printf(TEXT("%s should expose TargetArray as mutable"), *FunctionName.ToString()), TargetArrayProperty->HasAnyPropertyFlags(CPF_ConstParm));
		}
	}

	TestObject->TestArray = {10, 20, 30, 40, 50};
	TArray<int32> SampledValues;
	FRandomStream FirstStream(1337);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 3, false, &FirstStream, &SampledValues, ArrayProperty);
	TestEqual("Sample without replacement should return the requested count", SampledValues.Num(), 3);
	TestEqual("Sample without replacement should contain unique values", TSet<int32>(SampledValues).Num(), 3);

	TArray<int32> RepeatedSample;
	FRandomStream SecondStream(1337);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 3, false, &SecondStream, &RepeatedSample, ArrayProperty);
	TestEqual("Sample from stream should be deterministic", RepeatedSample, SampledValues);

	FRandomStream ClampedStream(42);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 20, false, &ClampedStream, &SampledValues, ArrayProperty);
	TestEqual("Sample without replacement should clamp to the source length", SampledValues.Num(), TestObject->TestArray.Num());

	FRandomStream ReplacementStream(7);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 20, true, &ReplacementStream, &SampledValues, ArrayProperty);
	TestEqual("Sample with replacement should return the requested count", SampledValues.Num(), 20);

	TestObject->TestArray = {99};
	FRandomStream SingleValueStream(7);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 4, true, &SingleValueStream, &SampledValues, ArrayProperty);
	TestEqual("Sample with replacement should repeat the only available value", SampledValues, TArray<int32>({99, 99, 99, 99}));

	TestObject->TestArray.Empty();
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 4, false, nullptr, &SampledValues, ArrayProperty);
	TestTrue("Sampling an empty array should return an empty array", SampledValues.IsEmpty());

	TestObject->TestArray = {10, 20, 30, 40, 50};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 0, false, nullptr, &SampledValues, ArrayProperty);
	TestTrue("Sampling zero values should return an empty array", SampledValues.IsEmpty());

	TArray<int32> Page;
	int32 PageCount = 0;
	TestTrue("GetPage should return an available page", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, 1, 2, &Page, ArrayProperty, &PageCount));
	TestEqual("GetPage should return the second page", Page, TArray<int32>({30, 40}));
	TestEqual("GetPage should return the total page count", PageCount, 3);
	TestFalse("GetPage should reject a negative page", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, -1, 2, &Page, ArrayProperty, &PageCount));
	TestTrue("GetPage should clear output for a negative page", Page.IsEmpty());
	TestFalse("GetPage should reject a non-positive page size", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, 0, 0, &Page, ArrayProperty, &PageCount));
	TestEqual("GetPage should report zero pages for an invalid page size", PageCount, 0);

	TestObject->TestArray = {1, 2, 3, 4};
	FRandomStream InPlaceStream(11);
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 2, false, &InPlaceStream, &TestObject->TestArray, ArrayProperty);
	TestEqual("Sample should support the same source and output array", TestObject->TestArray.Num(), 2);
	TestObject->TestArray = {1, 2, 3, 4};
	TestTrue("GetPage should support the same source and output array", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, 1, 2, &TestObject->TestArray, ArrayProperty, &PageCount));
	TestEqual("In-place GetPage should return the requested values", TestObject->TestArray, TArray<int32>({3, 4}));

	TArray<FString> NaturalStrings = {TEXT("Item10"), TEXT("Item2"), TEXT("Item1"), TEXT("アイテム2")};
	UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(NaturalStrings);
	TestEqual("Natural string sort should compare embedded numbers", NaturalStrings[0], FString(TEXT("Item1")));
	TestEqual("Natural string sort should place Item2 before Item10", NaturalStrings[1], FString(TEXT("Item2")));
	TestEqual("Natural string sort should place Item10 after Item2", NaturalStrings[2], FString(TEXT("Item10")));
	TestEqual("Natural string sort should preserve Unicode strings", NaturalStrings[3], FString(TEXT("アイテム2")));
	UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(NaturalStrings, true);
	TestEqual("Natural string sort should support descending order", NaturalStrings.Last(), FString(TEXT("Item1")));

	TArray<FName> NaturalNames = {FName(TEXT("Actor12")), FName(TEXT("Actor3")), FName(TEXT("Actor1"))};
	UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(NaturalNames);
	TestEqual("Natural name sort should compare embedded numbers", NaturalNames, TArray<FName>({FName(TEXT("Actor1")), FName(TEXT("Actor3")), FName(TEXT("Actor12"))}));

	TestObject->TestArray = {1, 2, 3, 4, 5};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 1, 3, &TestObject->TestArray, ArrayProperty);
	TestEqual("Slice should support the same source and output array", TestObject->TestArray, TArray<int32>({2, 3, 4}));

	TestObject->TestArray = {3, 1, 3, 2, 1};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetDistinct should support the same source and output array", TestObject->TestArray, TArray<int32>({3, 1, 2}));

	FArrayProperty* StringArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringArray));
	FArrayProperty* TextArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestTextArray));
	FArrayProperty* CollisionArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestCollisionArray));
	FArrayProperty* ObjectArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestObjectArray));
	TestNotNull("String array property should be available", StringArrayProperty);
	TestNotNull("Text array property should be available", TextArrayProperty);
	TestNotNull("Collision array property should be available", CollisionArrayProperty);
	TestNotNull("Object array property should be available", ObjectArrayProperty);

	TestObject->TestStringArray = {TEXT("One"), TEXT("Two"), TEXT("Three"), TEXT("Four")};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestStringArray, StringArrayProperty, 2);
	TestEqual(
		"Rotate should preserve non-trivial values",
		TestObject->TestStringArray,
		TArray<FString>({TEXT("Three"), TEXT("Four"), TEXT("One"), TEXT("Two")}));
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestStringArray, StringArrayProperty, -2);
	TestEqual(
		"Rotate should support negative shifts for non-trivial values",
		TestObject->TestStringArray,
		TArray<FString>({TEXT("One"), TEXT("Two"), TEXT("Three"), TEXT("Four")}));

	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
		&TestObject->TestStringArray,
		StringArrayProperty,
		1,
		2,
		&TestObject->TestStringArray,
		StringArrayProperty);
	TestEqual("Aliased Slice should preserve string values", TestObject->TestStringArray, TArray<FString>({TEXT("Two"), TEXT("Three")}));

	UObject* FirstObject = NewObject<UDirectiveUtilTestObject>(TestObject);
	UObject* SecondObject = NewObject<UDirectiveUtilTestObject>(TestObject);
	UObject* ThirdObject = NewObject<UDirectiveUtilTestObject>(TestObject);
	TestObject->TestObjectArray = {FirstObject, SecondObject, FirstObject};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestObjectArray, ObjectArrayProperty);
	TestEqual("RemoveDuplicates should preserve object references", TestObject->TestObjectArray.Num(), 2);
	if (TestObject->TestObjectArray.Num() == 2)
	{
		TestEqual("RemoveDuplicates should retain the first object", TestObject->TestObjectArray[0].Get(), FirstObject);
		TestEqual("RemoveDuplicates should retain the second object", TestObject->TestObjectArray[1].Get(), SecondObject);
	}

	TestObject->TestObjectArray = {FirstObject, SecondObject, ThirdObject};
	TestTrue("Aliased object GetPage should succeed", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(
		&TestObject->TestObjectArray,
		ObjectArrayProperty,
		1,
		2,
		&TestObject->TestObjectArray,
		ObjectArrayProperty,
		&PageCount));
	TestEqual("Aliased object GetPage should preserve the selected reference", TestObject->TestObjectArray.Num(), 1);
	if (TestObject->TestObjectArray.Num() == 1)
	{
		TestEqual("Aliased object GetPage should return the final object", TestObject->TestObjectArray[0].Get(), ThirdObject);
	}

	TestObject->TestStringArray = {TEXT("unchanged")};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
		&TestObject->TestArray,
		ArrayProperty,
		0,
		1,
		&TestObject->TestStringArray,
		StringArrayProperty);
	TestEqual("Slice should reject mismatched array types without changing output", TestObject->TestStringArray, TArray<FString>({TEXT("unchanged")}));

	const FText AlphaText = FText::FromString(TEXT("Alpha"));
	const FText BetaText = FText::FromString(TEXT("Beta"));
	TestObject->TestTextArray = {AlphaText, BetaText, AlphaText};
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestTextArray, TextArrayProperty);
	TestEqual("RemoveDuplicates should support unhashable property types", TestObject->TestTextArray.Num(), 2);
	if (TestObject->TestTextArray.Num() == 2)
	{
		TestTrue("RemoveDuplicates should retain the first unhashable value", TestObject->TestTextArray[0].IdenticalTo(AlphaText));
		TestTrue("RemoveDuplicates should retain the second unhashable value", TestObject->TestTextArray[1].IdenticalTo(BetaText));
	}

	auto AddCollisionValue = [&TestObject](const int32 Value)
	{
		FDirectiveUtilCollisionValue& Entry = TestObject->TestCollisionArray.AddDefaulted_GetRef();
		Entry.Value = Value;
	};
	TestObject->TestCollisionArray.Reset();
	for (const int32 Value : {1, 2, 1, 3, 2})
	{
		AddCollisionValue(Value);
	}
	TestTrue("Collision test property should expose a value hash", CollisionArrayProperty->Inner->HasAllPropertyFlags(CPF_HasGetValueTypeHash));
	UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestCollisionArray, CollisionArrayProperty);
	TestEqual("RemoveDuplicates should resolve hash collisions", TestObject->TestCollisionArray.Num(), 3);
	if (TestObject->TestCollisionArray.Num() == 3)
	{
		TestEqual("Hash collision result should retain the first value", TestObject->TestCollisionArray[0].Value, 1);
		TestEqual("Hash collision result should retain the second value", TestObject->TestCollisionArray[1].Value, 2);
		TestEqual("Hash collision result should retain the third value", TestObject->TestCollisionArray[2].Value, 3);
	}

	TestObject->TestCollisionArray.Reset();
	for (const int32 Value : {4, 5, 5, 4})
	{
		AddCollisionValue(Value);
	}
	FDirectiveUtilCollisionValue MostCommonCollision;
	MostCommonCount = 0;
	TestTrue("GetMostCommon should resolve hash collisions", UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(
		&TestObject->TestCollisionArray,
		CollisionArrayProperty,
		&MostCommonCollision,
		&MostCommonCount));
	TestEqual("GetMostCommon should use first occurrence to break collision ties", MostCommonCollision.Value, 4);
	TestEqual("GetMostCommon should report the collision value count", MostCommonCount, 2);

	TestObject->TestArray.SetNumUninitialized(100);
	for (int32 Index = 0; Index < TestObject->TestArray.Num(); ++Index)
	{
		TestObject->TestArray[Index] = Index;
	}
	for (const int32 Count : {10, 75})
	{
		FRandomStream CompatibilityStream(9917);
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
			&TestObject->TestArray,
			ArrayProperty,
			Count,
			false,
			&CompatibilityStream,
			&SampledValues,
			ArrayProperty);
		TestEqual(
			FString::Printf(TEXT("Sampling %d values should match dense Fisher-Yates"), Count),
			SampledValues,
			BuildReferenceSample(TestObject->TestArray.Num(), Count, 9917));
	}

	TArray<int32> SelectionCounts;
	SelectionCounts.Init(0, 5);
	TestObject->TestArray = {0, 1, 2, 3, 4};
	FRandomStream UniformityStream(77123);
	for (int32 Iteration = 0; Iteration < 5000; ++Iteration)
	{
		UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
			&TestObject->TestArray,
			ArrayProperty,
			1,
			false,
			&UniformityStream,
			&SampledValues,
			ArrayProperty);
		if (SampledValues.Num() == 1 && SelectionCounts.IsValidIndex(SampledValues[0]))
		{
			++SelectionCounts[SampledValues[0]];
		}
	}
	for (int32 Value = 0; Value < SelectionCounts.Num(); ++Value)
	{
		TestTrue(
			FString::Printf(TEXT("Sampling frequency for value %d should remain within tolerance"), Value),
			FMath::Abs(SelectionCounts[Value] - 1000) <= 150);
	}

	FRandomStream FuzzStream(18181);
	for (int32 Iteration = 0; Iteration < 100; ++Iteration)
	{
		TArray<int32> SourceValues;
		const int32 ValueCount = FuzzStream.RandRange(0, 128);
		SourceValues.Reserve(ValueCount);
		for (int32 Index = 0; Index < ValueCount; ++Index)
		{
			SourceValues.Add(FuzzStream.RandRange(-12, 12));
		}
		const TArray<int32> ExpectedDistinct = BuildReferenceDistinct(SourceValues);
		TestObject->TestArray = SourceValues;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty);
		TestEqual(FString::Printf(TEXT("RemoveDuplicates fuzz case %d"), Iteration), TestObject->TestArray, ExpectedDistinct);
		TestObject->TestArray = SourceValues;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &DistinctOut, ArrayProperty);
		TestEqual(FString::Printf(TEXT("GetDistinct fuzz case %d"), Iteration), DistinctOut, ExpectedDistinct);
	}

	TArray<FString> StableNaturalStrings = {TEXT("ItemA"), TEXT("itema"), TEXT("ItemB"), TEXT("itemb")};
	UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(StableNaturalStrings);
	TestEqual(
		"Natural string sort should preserve equivalent-key order",
		StableNaturalStrings,
		TArray<FString>({TEXT("ItemA"), TEXT("itema"), TEXT("ItemB"), TEXT("itemb")}));
	UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(StableNaturalStrings, true);
	TestEqual(
		"Descending natural string sort should preserve equivalent-key order",
		StableNaturalStrings,
		TArray<FString>({TEXT("ItemB"), TEXT("itemb"), TEXT("ItemA"), TEXT("itema")}));

	return true;
}
