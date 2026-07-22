// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMapFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

#include "Misc/AutomationTest.h"

namespace
{
	template <typename PropertyType>
	PropertyType* GetTestProperty(const FName PropertyName)
	{
		return FindFProperty<PropertyType>(UDirectiveUtilTestObject::StaticClass(), PropertyName);
	}

	TSet<int32> MakeIntegerSet(const TArray<int32>& Values)
	{
		TSet<int32> Result;
		for (const int32 Value : Values)
		{
			Result.Add(Value);
		}
		return Result;
	}

	TSet<FString> MakeStringSet(const TArray<FString>& Values)
	{
		TSet<FString> Result;
		for (const FString& Value : Values)
		{
			Result.Add(Value);
		}
		return Result;
	}

	TSet<FName> MakeNameSet(const TArray<FName>& Values)
	{
		TSet<FName> Result;
		for (const FName Value : Values)
		{
			Result.Add(Value);
		}
		return Result;
	}

	template <typename ValueType>
	bool HaveSameValues(const TSet<ValueType>& Left, const TSet<ValueType>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}
		for (const ValueType& Value : Left)
		{
			if (!Right.Contains(Value))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilMapScenarioTest,
	"DirectiveUtilities.MapScenarios.CardinalityAndTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMapScenarioTest::RunTest(const FString& Parameters)
{
	FMapProperty* IntegerMapProperty = GetTestProperty<FMapProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestMap));
	FArrayProperty* IntegerArrayProperty = GetTestProperty<FArrayProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));
	FMapProperty* StringMapProperty = GetTestProperty<FMapProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringMap));
	FMapProperty* StringMapProperty2 = GetTestProperty<FMapProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringMap2));
	FArrayProperty* StringArrayProperty = GetTestProperty<FArrayProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringArray));
	FMapProperty* StructMapProperty = GetTestProperty<FMapProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStructValueMap));
	FArrayProperty* NameArrayProperty = GetTestProperty<FArrayProperty>(GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestNameArray));
	if (!TestNotNull("Integer map property should be available", IntegerMapProperty)
		|| !TestNotNull("Integer array property should be available", IntegerArrayProperty)
		|| !TestNotNull("String map property should be available", StringMapProperty)
		|| !TestNotNull("Second string map property should be available", StringMapProperty2)
		|| !TestNotNull("String array property should be available", StringArrayProperty)
		|| !TestNotNull("Struct map property should be available", StructMapProperty)
		|| !TestNotNull("Name array property should be available", NameArrayProperty))
	{
		return false;
	}

	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();
	for (const int32 ItemCount : {0, 1, 2, 16, 257, 4096})
	{
		TestObject->TestMap.Reset();
		for (int32 Key = 0; Key < ItemCount; ++Key)
		{
			TestObject->TestMap.Add(Key, Key % 7);
		}
		const FString Label = FString::Printf(TEXT("integer items=%d"), ItemCount);
		const int32 QueryValue = 3;
		TArray<int32> ExpectedKeys;
		for (int32 Key = 0; Key < ItemCount; ++Key)
		{
			if (Key % 7 == QueryValue)
			{
				ExpectedKeys.Add(Key);
			}
		}

		TestObject->TestArray.Reset();
		UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(
			&TestObject->TestMap,
			IntegerMapProperty,
			&QueryValue,
			&TestObject->TestArray,
			IntegerArrayProperty);
		TestEqual(Label + TEXT(" GetKeysByValue count"), TestObject->TestArray.Num(), ExpectedKeys.Num());
		TestTrue(Label + TEXT(" GetKeysByValue values"), HaveSameValues(MakeIntegerSet(TestObject->TestArray), MakeIntegerSet(ExpectedKeys)));
		TestEqual(
			Label + TEXT(" HasValue"),
			UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(&TestObject->TestMap, IntegerMapProperty, &QueryValue),
			!ExpectedKeys.IsEmpty());

		const int32 ExistingKey = ItemCount > 0 ? ItemCount / 2 : 0;
		int32 FoundValue = -1;
		UDirectiveUtilMapFunctionLibrary::GenericMap_FindOrAdd(
			&TestObject->TestMap,
			IntegerMapProperty,
			&ExistingKey,
			&FoundValue);
		TestEqual(Label + TEXT(" FindOrAdd value"), FoundValue, ItemCount > 0 ? ExistingKey % 7 : 0);

		UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(&TestObject->TestMap, IntegerMapProperty);
		TestEqual(Label + TEXT(" ClearValues count"), TestObject->TestMap.Num(), FMath::Max(ItemCount, 1));
		for (const TPair<int32, int32>& Pair : TestObject->TestMap)
		{
			TestEqual(Label + TEXT(" ClearValues value"), Pair.Value, 0);
		}

		TestObject->TestArray.Reset();
		for (int32 Key = 0; Key < ItemCount; Key += 3)
		{
			TestObject->TestArray.Add(Key);
		}
		const int32 ExpectedRemoved = TestObject->TestArray.Num();
		const int32 Removed = UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(
			&TestObject->TestMap,
			IntegerMapProperty,
			&TestObject->TestArray,
			IntegerArrayProperty);
		TestEqual(Label + TEXT(" RemoveKeys count"), Removed, ExpectedRemoved);
		TestEqual(Label + TEXT(" RemoveKeys remaining"), TestObject->TestMap.Num(), FMath::Max(ItemCount, 1) - ExpectedRemoved);
	}

	for (const int32 ItemCount : {0, 1, 16, 257})
	{
		TestObject->TestStringMap.Reset();
		TestObject->TestStringMap2.Reset();
		for (int32 Index = 0; Index < ItemCount; ++Index)
		{
			const FString Key = FString::Printf(TEXT("Key%d"), Index);
			TestObject->TestStringMap.Add(Key, FString::Printf(TEXT("Original%d"), Index));
			TestObject->TestStringMap2.Add(Key, FString::Printf(TEXT("Replacement%d"), Index));
		}
		TestObject->TestStringMap2.Add(TEXT("Added"), TEXT("AddedValue"));
		const FString Label = FString::Printf(TEXT("string items=%d"), ItemCount);

		UDirectiveUtilMapFunctionLibrary::GenericMap_Append(
			&TestObject->TestStringMap,
			StringMapProperty,
			&TestObject->TestStringMap2,
			StringMapProperty2,
			false);
		TestEqual(Label + TEXT(" Append count"), TestObject->TestStringMap.Num(), ItemCount + 1);
		TestEqual(Label + TEXT(" Append new value"), TestObject->TestStringMap.FindRef(TEXT("Added")), FString(TEXT("AddedValue")));
		if (ItemCount > 0)
		{
			TestEqual(Label + TEXT(" Append preserve"), TestObject->TestStringMap.FindRef(TEXT("Key0")), FString(TEXT("Original0")));
		}

		UDirectiveUtilMapFunctionLibrary::GenericMap_Append(
			&TestObject->TestStringMap,
			StringMapProperty,
			&TestObject->TestStringMap2,
			StringMapProperty2,
			true);
		if (ItemCount > 0)
		{
			TestEqual(Label + TEXT(" Append overwrite"), TestObject->TestStringMap.FindRef(TEXT("Key0")), FString(TEXT("Replacement0")));
		}

		const int32 CountBeforeSelfAppend = TestObject->TestStringMap.Num();
		UDirectiveUtilMapFunctionLibrary::GenericMap_Append(
			&TestObject->TestStringMap,
			StringMapProperty,
			&TestObject->TestStringMap,
			StringMapProperty,
			true);
		TestEqual(Label + TEXT(" self append"), TestObject->TestStringMap.Num(), CountBeforeSelfAppend);

		const FString QueryValue = ItemCount > 0 ? TEXT("Replacement0") : TEXT("missing");
		TestObject->TestStringArray.Reset();
		UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(
			&TestObject->TestStringMap,
			StringMapProperty,
			&QueryValue,
			&TestObject->TestStringArray,
			StringArrayProperty);
		const TArray<FString> ExpectedKeys = ItemCount > 0 ? TArray<FString>({TEXT("Key0")}) : TArray<FString>();
		TestTrue(Label + TEXT(" managed GetKeysByValue"), HaveSameValues(MakeStringSet(TestObject->TestStringArray), MakeStringSet(ExpectedKeys)));

		UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(&TestObject->TestStringMap, StringMapProperty);
		for (const TPair<FString, FString>& Pair : TestObject->TestStringMap)
		{
			TestTrue(Label + TEXT(" managed ClearValues"), Pair.Value.IsEmpty());
		}
	}

	FDirectiveUtilCollisionValue FirstStructValue;
	FirstStructValue.Value = 1;
	FDirectiveUtilCollisionValue SecondStructValue;
	SecondStructValue.Value = 2;
	TestObject->TestStructValueMap = {
		{TEXT("Alpha"), FirstStructValue},
		{TEXT("Beta"), SecondStructValue},
		{TEXT("Gamma"), FirstStructValue}
	};
	const FDirectiveUtilCollisionValue StructQuery = FirstStructValue;
	TestObject->TestNameArray.Reset();
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(
		&TestObject->TestStructValueMap,
		StructMapProperty,
		&StructQuery,
		&TestObject->TestNameArray,
		NameArrayProperty);
	TestTrue(
		"Struct GetKeysByValue",
		HaveSameValues(
			MakeNameSet(TestObject->TestNameArray),
			MakeNameSet(TArray<FName>({TEXT("Alpha"), TEXT("Gamma")}))));
	TestTrue(
		"Struct HasValue",
		UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(
			&TestObject->TestStructValueMap,
			StructMapProperty,
			&StructQuery));
	UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(&TestObject->TestStructValueMap, StructMapProperty);
	for (const TPair<FName, FDirectiveUtilCollisionValue>& Pair : TestObject->TestStructValueMap)
	{
		TestEqual("Struct ClearValues", Pair.Value.Value, 0);
	}

	return true;
}
