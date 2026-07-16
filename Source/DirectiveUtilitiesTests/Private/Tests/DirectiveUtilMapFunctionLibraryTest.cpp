#include "Libraries/DirectiveUtilMapFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilMapFunctionLibraryTest, "DirectiveUtilities.MapFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMapFunctionLibraryTest::RunTest(const FString& Parameters)
{
	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();

	FMapProperty* MapProperty = FindFProperty<FMapProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestMap));
	TestNotNull("TestMap property should be found", MapProperty);
	if (!MapProperty)
	{
		return false;
	}

	TestObject->TestMap = {{1, 100}, {2, 200}};
	int32 ExistingKey = 2;
	int32 FoundValue = -1;
	UDirectiveUtilMapFunctionLibrary::GenericMap_FindOrAdd(&TestObject->TestMap, MapProperty, &ExistingKey, &FoundValue);
	TestEqual("FindOrAdd should return the existing value for a present key", FoundValue, 200);
	TestEqual("FindOrAdd should not change the map size for a present key", TestObject->TestMap.Num(), 2);

	int32 MissingKey = 3;
	int32 AddedValue = -1;
	UDirectiveUtilMapFunctionLibrary::GenericMap_FindOrAdd(&TestObject->TestMap, MapProperty, &MissingKey, &AddedValue);
	TestEqual("FindOrAdd should return the default value for a missing key", AddedValue, 0);
	TestEqual("FindOrAdd should grow the map for a missing key", TestObject->TestMap.Num(), 3);
	TestTrue("FindOrAdd should insert the missing key", TestObject->TestMap.Contains(3));
	if (const int32* AddedEntry = TestObject->TestMap.Find(3))
	{
		TestEqual("FindOrAdd should store a default value for the new key", *AddedEntry, 0);
	}

	TestObject->TestMap = {{1, 100}, {2, 200}, {3, 300}};
	UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(&TestObject->TestMap, MapProperty);
	TestEqual("ClearValues should preserve the number of entries", TestObject->TestMap.Num(), 3);
	TestTrue("ClearValues should preserve key 1", TestObject->TestMap.Contains(1));
	TestTrue("ClearValues should preserve key 2", TestObject->TestMap.Contains(2));
	TestTrue("ClearValues should preserve key 3", TestObject->TestMap.Contains(3));
	for (const TPair<int32, int32>& Pair : TestObject->TestMap)
	{
		TestEqual("ClearValues should reset every value to its default", Pair.Value, 0);
	}

	TestObject->TestMap.Empty();
	UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(&TestObject->TestMap, MapProperty);
	TestEqual("ClearValues on an empty map should leave it empty", TestObject->TestMap.Num(), 0);

	FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));
	FMapProperty* MapProperty2 = FindFProperty<FMapProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestMap2));
	FMapProperty* StringKeyMapProperty = FindFProperty<FMapProperty>(UDirectiveUtilTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringKeyMap));
	TestNotNull("TestArray property should be found", ArrayProperty);
	TestNotNull("TestMap2 property should be found", MapProperty2);
	TestNotNull("TestStringKeyMap property should be found", StringKeyMapProperty);
	if (!ArrayProperty || !MapProperty2 || !StringKeyMapProperty)
	{
		return false;
	}

	TestObject->TestMap = {{1, 10}, {2, 20}, {3, 10}};
	TestObject->TestArray = {42};
	int32 SearchValue = 10;
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(&TestObject->TestMap, MapProperty, &SearchValue, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetKeysByValue should find two keys for a shared value", TestObject->TestArray.Num(), 2);
	if (TestObject->TestArray.Num() == 2)
	{
		TestEqual("GetKeysByValue should return the first matching key", TestObject->TestArray[0], 1);
		TestEqual("GetKeysByValue should return the second matching key", TestObject->TestArray[1], 3);
	}

	SearchValue = 99;
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(&TestObject->TestMap, MapProperty, &SearchValue, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetKeysByValue should return no keys for an absent value", TestObject->TestArray.Num(), 0);

	TestObject->TestArray = {42};
	SearchValue = 10;
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(nullptr, MapProperty, &SearchValue, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetKeysByValue with a null map should empty the output array", TestObject->TestArray.Num(), 0);
	TestObject->TestArray = {42};
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(&TestObject->TestMap, nullptr, &SearchValue, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetKeysByValue with a null map property should empty the output array", TestObject->TestArray.Num(), 0);
	TestObject->TestStringKeyMap = {{TEXT("One"), 10}};
	TestObject->TestArray = {42};
	UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(&TestObject->TestStringKeyMap, StringKeyMapProperty, &SearchValue, &TestObject->TestArray, ArrayProperty);
	TestEqual("GetKeysByValue with a mismatched key type should empty the output array", TestObject->TestArray.Num(), 0);

	TestObject->TestMap = {{1, 10}, {2, 20}, {3, 10}};
	int32 QueryValue = 20;
	TestTrue("HasValue should find a present value", UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(&TestObject->TestMap, MapProperty, &QueryValue));
	QueryValue = 99;
	TestFalse("HasValue should not find an absent value", UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(&TestObject->TestMap, MapProperty, &QueryValue));
	TestObject->TestMap.Empty();
	QueryValue = 20;
	TestFalse("HasValue on an empty map should be false", UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(&TestObject->TestMap, MapProperty, &QueryValue));
	TestFalse("HasValue with a null map should be false", UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(nullptr, MapProperty, &QueryValue));
	TestFalse("HasValue with a null map property should be false", UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(&TestObject->TestMap, nullptr, &QueryValue));

	TestObject->TestMap = {{1, 10}, {2, 20}, {3, 10}};
	TestObject->TestArray = {1, 3, 99};
	int32 NumRemoved = UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(&TestObject->TestMap, MapProperty, &TestObject->TestArray, ArrayProperty);
	TestEqual("RemoveKeys should report two removed entries", NumRemoved, 2);
	TestEqual("RemoveKeys should leave one entry behind", TestObject->TestMap.Num(), 1);
	TestTrue("RemoveKeys should preserve the untouched key", TestObject->TestMap.Contains(2));

	TestEqual("RemoveKeys with a null map should remove nothing", UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(nullptr, MapProperty, &TestObject->TestArray, ArrayProperty), 0);
	TestEqual("RemoveKeys with a null map property should remove nothing", UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(&TestObject->TestMap, nullptr, &TestObject->TestArray, ArrayProperty), 0);
	TestEqual("RemoveKeys with a mismatched key type should remove nothing", UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(&TestObject->TestStringKeyMap, StringKeyMapProperty, &TestObject->TestArray, ArrayProperty), 0);
	TestEqual("RemoveKeys with a mismatched key type should not change the map", TestObject->TestStringKeyMap.Num(), 1);

	TestObject->TestMap = {{1, 10}};
	TestObject->TestMap2 = {{1, 99}, {4, 40}};
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(&TestObject->TestMap, MapProperty, &TestObject->TestMap2, MapProperty2, true);
	TestEqual("Append with overwrite should end with two entries", TestObject->TestMap.Num(), 2);
	if (const int32* OverwrittenValue = TestObject->TestMap.Find(1))
	{
		TestEqual("Append with overwrite should replace the existing value", *OverwrittenValue, 99);
	}
	if (const int32* AppendedValue = TestObject->TestMap.Find(4))
	{
		TestEqual("Append with overwrite should copy the new pair", *AppendedValue, 40);
	}

	TestObject->TestMap = {{1, 10}};
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(&TestObject->TestMap, MapProperty, &TestObject->TestMap2, MapProperty2, false);
	TestEqual("Append without overwrite should end with two entries", TestObject->TestMap.Num(), 2);
	if (const int32* PreservedValue = TestObject->TestMap.Find(1))
	{
		TestEqual("Append without overwrite should preserve the existing value", *PreservedValue, 10);
	}
	if (const int32* NewValue = TestObject->TestMap.Find(4))
	{
		TestEqual("Append without overwrite should still copy the new pair", *NewValue, 40);
	}

	TestObject->TestMap = {{1, 10}};
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(nullptr, MapProperty, &TestObject->TestMap2, MapProperty2, true);
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(&TestObject->TestMap, nullptr, &TestObject->TestMap2, MapProperty2, true);
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(&TestObject->TestMap, MapProperty, &TestObject->TestStringKeyMap, StringKeyMapProperty, true);
	TestEqual("Append with null or mismatched properties should not change the target", TestObject->TestMap.Num(), 1);
	if (const int32* UntouchedValue = TestObject->TestMap.Find(1))
	{
		TestEqual("Append with null or mismatched properties should preserve the value", *UntouchedValue, 10);
	}

	TestObject->TestMap = {{1, 10}, {2, 20}};
	UDirectiveUtilMapFunctionLibrary::GenericMap_Append(&TestObject->TestMap, MapProperty, &TestObject->TestMap, MapProperty, true);
	TestEqual("Append with the same map should preserve its size", TestObject->TestMap.Num(), 2);
	TestEqual("Append with the same map should preserve the first value", TestObject->TestMap.FindRef(1), 10);
	TestEqual("Append with the same map should preserve the second value", TestObject->TestMap.FindRef(2), 20);

	return true;
}
