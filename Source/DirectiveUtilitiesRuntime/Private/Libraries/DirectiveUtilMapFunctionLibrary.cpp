// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilMapFunctionLibrary.h"

void UDirectiveUtilMapFunctionLibrary::Map_FindOrAdd(const TMap<int32, int32>& TargetMap, const int32& Key, int32& Value)
{
	checkNoEntry();
}

void UDirectiveUtilMapFunctionLibrary::Map_ClearValues(const TMap<int32, int32>& TargetMap)
{
	checkNoEntry();
}

void UDirectiveUtilMapFunctionLibrary::Map_GetKeysByValue(const TMap<int32, int32>& TargetMap, const int32& Value, TArray<int32>& Keys)
{
	checkNoEntry();
}

bool UDirectiveUtilMapFunctionLibrary::Map_HasValue(const TMap<int32, int32>& TargetMap, const int32& Value)
{
	checkNoEntry();
	return false;
}

int32 UDirectiveUtilMapFunctionLibrary::Map_RemoveKeys(const TMap<int32, int32>& TargetMap, const TArray<int32>& Keys)
{
	checkNoEntry();
	return 0;
}

void UDirectiveUtilMapFunctionLibrary::Map_Append(const TMap<int32, int32>& TargetMap, const TMap<int32, int32>& SourceMap, bool bOverwriteExisting)
{
	checkNoEntry();
}

void UDirectiveUtilMapFunctionLibrary::GenericMap_FindOrAdd(
	const void* TargetMap,
	const FMapProperty* MapProperty,
	const void* KeyPtr,
	void* ValuePtr)
{
	if (!TargetMap || !MapProperty || !KeyPtr)
	{
		return;
	}

	FScriptMapHelper MapHelper(MapProperty, TargetMap);

	void* ValueInMap = MapHelper.FindOrAdd(KeyPtr);
	if (ValuePtr && ValueInMap)
	{
		MapProperty->ValueProp->CopyCompleteValueFromScriptVM(ValuePtr, ValueInMap);
	}
}

void UDirectiveUtilMapFunctionLibrary::GenericMap_ClearValues(
	const void* TargetMap,
	const FMapProperty* MapProperty)
{
	if (!TargetMap || !MapProperty)
	{
		return;
	}

	FScriptMapHelper MapHelper(MapProperty, TargetMap);
	const FProperty* ValueProp = MapProperty->ValueProp;

	const int32 MaxIndex = MapHelper.GetMaxIndex();
	for (int32 InternalIndex = 0; InternalIndex < MaxIndex; ++InternalIndex)
	{
		if (MapHelper.IsValidIndex(InternalIndex))
		{
			ValueProp->ClearValue(MapHelper.GetValuePtr(InternalIndex));
		}
	}
}

void UDirectiveUtilMapFunctionLibrary::GenericMap_GetKeysByValue(
	const void* TargetMap,
	const FMapProperty* MapProperty,
	const void* ValuePtr,
	const void* TargetArray,
	const FArrayProperty* ArrayProperty)
{
	if (!TargetArray || !ArrayProperty)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	ArrayHelper.EmptyValues();

	if (!TargetMap || !MapProperty || !ValuePtr || !ArrayProperty->Inner->SameType(MapProperty->KeyProp))
	{
		return;
	}

	FScriptMapHelper MapHelper(MapProperty, TargetMap);
	const FProperty* ValueProp = MapProperty->ValueProp;
	const FProperty* InnerProp = ArrayProperty->Inner;

	const int32 MaxIndex = MapHelper.GetMaxIndex();
	for (int32 InternalIndex = 0; InternalIndex < MaxIndex; ++InternalIndex)
	{
		if (MapHelper.IsValidIndex(InternalIndex) && ValueProp->Identical(MapHelper.GetValuePtr(InternalIndex), ValuePtr))
		{
			const int32 LastIndex = ArrayHelper.AddValue();
			InnerProp->CopySingleValueToScriptVM(ArrayHelper.GetRawPtr(LastIndex), MapHelper.GetKeyPtr(InternalIndex));
		}
	}
}

bool UDirectiveUtilMapFunctionLibrary::GenericMap_HasValue(
	const void* TargetMap,
	const FMapProperty* MapProperty,
	const void* ValuePtr)
{
	if (!TargetMap || !MapProperty || !ValuePtr)
	{
		return false;
	}

	FScriptMapHelper MapHelper(MapProperty, TargetMap);
	const FProperty* ValueProp = MapProperty->ValueProp;

	const int32 MaxIndex = MapHelper.GetMaxIndex();
	for (int32 InternalIndex = 0; InternalIndex < MaxIndex; ++InternalIndex)
	{
		if (MapHelper.IsValidIndex(InternalIndex) && ValueProp->Identical(MapHelper.GetValuePtr(InternalIndex), ValuePtr))
		{
			return true;
		}
	}

	return false;
}

int32 UDirectiveUtilMapFunctionLibrary::GenericMap_RemoveKeys(
	const void* TargetMap,
	const FMapProperty* MapProperty,
	const void* TargetArray,
	const FArrayProperty* ArrayProperty)
{
	if (!TargetMap || !MapProperty || !TargetArray || !ArrayProperty
		|| !ArrayProperty->Inner->SameType(MapProperty->KeyProp))
	{
		return 0;
	}

	FScriptMapHelper MapHelper(MapProperty, TargetMap);
	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);

	int32 NumRemoved = 0;
	const int32 Num = ArrayHelper.Num();
	for (int32 Index = 0; Index < Num; ++Index)
	{
		if (MapHelper.RemovePair(ArrayHelper.GetRawPtr(Index)))
		{
			++NumRemoved;
		}
	}

	return NumRemoved;
}

void UDirectiveUtilMapFunctionLibrary::GenericMap_Append(
	const void* TargetMap,
	const FMapProperty* TargetMapProperty,
	const void* SourceMap,
	const FMapProperty* SourceMapProperty,
	const bool bOverwriteExisting)
{
	if (!TargetMap || !TargetMapProperty || !SourceMap || !SourceMapProperty
		|| !TargetMapProperty->KeyProp->SameType(SourceMapProperty->KeyProp)
		|| !TargetMapProperty->ValueProp->SameType(SourceMapProperty->ValueProp))
	{
		return;
	}

	if (TargetMap == SourceMap)
	{
		return;
	}

	FScriptMapHelper TargetHelper(TargetMapProperty, TargetMap);
	FScriptMapHelper SourceHelper(SourceMapProperty, SourceMap);

	const int32 MaxIndex = SourceHelper.GetMaxIndex();
	for (int32 InternalIndex = 0; InternalIndex < MaxIndex; ++InternalIndex)
	{
		if (!SourceHelper.IsValidIndex(InternalIndex))
		{
			continue;
		}

		const uint8* KeyPtr = SourceHelper.GetKeyPtr(InternalIndex);
		if (bOverwriteExisting || !TargetHelper.FindValueFromHash(KeyPtr))
		{
			TargetHelper.AddPair(KeyPtr, SourceHelper.GetValuePtr(InternalIndex));
		}
	}
}
