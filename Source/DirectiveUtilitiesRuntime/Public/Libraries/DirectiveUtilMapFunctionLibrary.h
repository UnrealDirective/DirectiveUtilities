// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Net/Core/PushModel/PushModel.h"
#include "DirectiveUtilMapFunctionLibrary.generated.h"

/**
 * UDirectiveUtilMapFunctionLibrary
 * A collection of map (TMap) utility functions that improve the usability of maps in Blueprints.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Finds the value associated with Key, adding a new default-constructed entry if the key is not present.
	 * @param TargetMap - The map to search or add to.
	 * @param Key - The key to look up.
	 * @param Value - [out] A copy of the existing or newly added value.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Find Or Add", CompactNodeTitle = "FIND OR ADD", MapParam = "TargetMap", MapKeyParam = "Key", MapValueParam = "Value", AutoCreateRefTerm = "Key, Value"), Category="Directive Utilities|Map")
	static void Map_FindOrAdd(const TMap<int32, int32>& TargetMap, const int32& Key, int32& Value);

	/**
	 * Resets every value in the map to its default while preserving all keys.
	 * @param TargetMap - The map whose values will be reset.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Clear Values", CompactNodeTitle = "CLEAR VALUES", MapParam = "TargetMap"), Category="Directive Utilities|Map")
	static void Map_ClearValues(const TMap<int32, int32>& TargetMap);

	/**
	 * Gathers every key whose value is identical to Value, in map order.
	 * @param TargetMap - The map to search.
	 * @param Value - The value to look for.
	 * @param Keys - [out] Every key associated with Value.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Keys By Value", CompactNodeTitle = "KEYS BY VALUE", MapParam = "TargetMap", MapValueParam = "Value", MapKeyParam = "Keys", AutoCreateRefTerm = "Value, Keys"), Category="Directive Utilities|Map")
	static void Map_GetKeysByValue(const TMap<int32, int32>& TargetMap, const int32& Value, TArray<int32>& Keys);

	/**
	 * Checks whether any value in the map is identical to Value.
	 * @param TargetMap - The map to search.
	 * @param Value - The value to look for.
	 * @returns True if at least one entry holds Value.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Has Value", CompactNodeTitle = "HAS VALUE", MapParam = "TargetMap", MapValueParam = "Value", AutoCreateRefTerm = "Value"), Category="Directive Utilities|Map")
	static bool Map_HasValue(const TMap<int32, int32>& TargetMap, const int32& Value);

	/**
	 * Removes every key in Keys from the map.
	 * @param TargetMap - The map to remove from.
	 * @param Keys - The keys to remove.
	 * @returns The number of entries that were actually removed.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Remove Keys", CompactNodeTitle = "REMOVE KEYS", MapParam = "TargetMap", MapKeyParam = "Keys", AutoCreateRefTerm = "Keys"), Category="Directive Utilities|Map")
	static int32 Map_RemoveKeys(const TMap<int32, int32>& TargetMap, const TArray<int32>& Keys);

	/**
	 * Copies every pair from SourceMap into TargetMap. Both maps must share key and value types.
	 * @param TargetMap - The map to copy into.
	 * @param SourceMap - The map to copy from.
	 * @param bOverwriteExisting - If true, keys already present in TargetMap are overwritten with SourceMap's values.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Append", CompactNodeTitle = "APPEND", MapParam = "TargetMap|SourceMap"), Category="Directive Utilities|Map")
	static void Map_Append(const TMap<int32, int32>& TargetMap, const TMap<int32, int32>& SourceMap, bool bOverwriteExisting = true);

	/*~
	 * Native functions called by the custom thunk layers below, which read off the property address
	 * and operate on the underlying map. Based off UBlueprintMapLibrary implementation.
	 ~*/

	static void GenericMap_FindOrAdd(const void* TargetMap, const FMapProperty* MapProperty, const void* KeyPtr, void* ValuePtr);
	static void GenericMap_ClearValues(const void* TargetMap, const FMapProperty* MapProperty);
	static void GenericMap_GetKeysByValue(const void* TargetMap, const FMapProperty* MapProperty, const void* ValuePtr, const void* TargetArray, const FArrayProperty* ArrayProperty);
	static bool GenericMap_HasValue(const void* TargetMap, const FMapProperty* MapProperty, const void* ValuePtr);
	static int32 GenericMap_RemoveKeys(const void* TargetMap, const FMapProperty* MapProperty, const void* TargetArray, const FArrayProperty* ArrayProperty);
	static void GenericMap_Append(const void* TargetMap, const FMapProperty* TargetMapProperty, const void* SourceMap, const FMapProperty* SourceMapProperty, bool bOverwriteExisting);

	/*~
	 * Custom thunk layers that read off the property address and call the appropriate native handler.
	 * Based off UBlueprintMapLibrary implementation.
	 ~*/

	DECLARE_FUNCTION(execMap_FindOrAdd)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* MapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* MapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!MapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* CurrKeyProp = MapProperty->KeyProp;
		const int32 KeyPropertySize = CurrKeyProp->GetElementSize() * CurrKeyProp->ArrayDim;
		void* KeyStorageSpace = FMemory_Alloca(KeyPropertySize);
		CurrKeyProp->InitializeValue(KeyStorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(KeyStorageSpace);

		const FProperty* CurrValueProp = MapProperty->ValueProp;
		const int32 ValuePropertySize = CurrValueProp->GetElementSize() * CurrValueProp->ArrayDim;
		void* ValueStorageSpace = FMemory_Alloca(ValuePropertySize);
		CurrValueProp->InitializeValue(ValueStorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(ValueStorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& ValuePropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(CurrValueProp->GetClass())
				|| CurrValueProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = ValueStorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, MapProperty);
		GenericMap_FindOrAdd(MapAddr, MapProperty, KeyStorageSpace, ItemPtr);
		P_NATIVE_END;

		CurrValueProp->DestroyValue(ValueStorageSpace);
		CurrKeyProp->DestroyValue(KeyStorageSpace);
	}

	DECLARE_FUNCTION(execMap_ClearValues)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* MapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* MapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!MapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, MapProperty);
		GenericMap_ClearValues(MapAddr, MapProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execMap_GetKeysByValue)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* MapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* MapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!MapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* CurrValueProp = MapProperty->ValueProp;
		const int32 ValuePropertySize = CurrValueProp->GetElementSize() * CurrValueProp->ArrayDim;
		void* ValueStorageSpace = FMemory_Alloca(ValuePropertySize);
		CurrValueProp->InitializeValue(ValueStorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(ValueStorageSpace);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty || !ArrayProperty->Inner->SameType(MapProperty->KeyProp))
		{
			Stack.bArrayContextFailed = true;
			CurrValueProp->DestroyValue(ValueStorageSpace);
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericMap_GetKeysByValue(MapAddr, MapProperty, ValueStorageSpace, ArrayAddr, ArrayProperty);
		P_NATIVE_END;

		CurrValueProp->DestroyValue(ValueStorageSpace);
	}

	DECLARE_FUNCTION(execMap_HasValue)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* MapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* MapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!MapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* CurrValueProp = MapProperty->ValueProp;
		const int32 ValuePropertySize = CurrValueProp->GetElementSize() * CurrValueProp->ArrayDim;
		void* ValueStorageSpace = FMemory_Alloca(ValuePropertySize);
		CurrValueProp->InitializeValue(ValueStorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(ValueStorageSpace);

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericMap_HasValue(MapAddr, MapProperty, ValueStorageSpace);
		P_NATIVE_END;

		CurrValueProp->DestroyValue(ValueStorageSpace);
	}

	DECLARE_FUNCTION(execMap_RemoveKeys)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* MapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* MapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!MapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty || !ArrayProperty->Inner->SameType(MapProperty->KeyProp))
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, MapProperty);
		*static_cast<int32*>(RESULT_PARAM) = GenericMap_RemoveKeys(MapAddr, MapProperty, ArrayAddr, ArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execMap_Append)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* TargetMapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* TargetMapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!TargetMapProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FMapProperty>(nullptr);
		void* SourceMapAddr = Stack.MostRecentPropertyAddress;
		FMapProperty* SourceMapProperty = CastField<FMapProperty>(Stack.MostRecentProperty);
		if (!SourceMapProperty
			|| !SourceMapProperty->KeyProp->SameType(TargetMapProperty->KeyProp)
			|| !SourceMapProperty->ValueProp->SameType(TargetMapProperty->ValueProp))
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_GET_UBOOL(bOverwriteExisting);

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, TargetMapProperty);
		GenericMap_Append(TargetMapAddr, TargetMapProperty, SourceMapAddr, SourceMapProperty, bOverwriteExisting);
		P_NATIVE_END;
	}
};
