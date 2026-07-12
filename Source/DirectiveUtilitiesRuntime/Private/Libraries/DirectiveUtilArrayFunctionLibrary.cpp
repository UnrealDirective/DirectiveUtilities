// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"

int32 UDirectiveUtilArrayFunctionLibrary::Array_NextIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop)
{
	checkNoEntry();
	return 0;
}

int32 UDirectiveUtilArrayFunctionLibrary::Array_PreviousIndex(
	const TArray<int32>& TargetArray,
	const int32 Index,
	const bool bLoop)
{
	checkNoEntry();
	return 0;
}

int32 UDirectiveUtilArrayFunctionLibrary::GenericArray_NextIndex(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index,
	const bool bLoop)
{
	if (!TargetArray || !ArrayProperty)
	{
		return INDEX_NONE;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 NextIndex = Index + 1;

	if(ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}

	if(NextIndex < 0)
	{
		return 0;
	}

	if (NextIndex <= ArrayHelper.Num() - 1)
	{
		return NextIndex;
	}

	if (bLoop)
	{
		return 0;
	}

	return ArrayHelper.Num() - 1;
}

void UDirectiveUtilArrayFunctionLibrary::Array_RemoveDuplicates(const TArray<int32>& TargetArray)
{
	checkNoEntry();
}

void UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(
	void* TargetArray,
	const FArrayProperty* ArrayProperty)
{
	if (!TargetArray || !ArrayProperty)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;

	for (int32 OuterIndex = ArrayHelper.Num() - 1; OuterIndex > 0; --OuterIndex)
	{
		for (int32 InnerIndex = 0; InnerIndex < OuterIndex; ++InnerIndex)
		{
			if (InnerProp->Identical(ArrayHelper.GetElementPtr(OuterIndex), ArrayHelper.GetElementPtr(InnerIndex)))
			{
				ArrayHelper.RemoveValues(OuterIndex);
				break;
			}
		}
	}
}

int32 UDirectiveUtilArrayFunctionLibrary::GenericArray_PreviousIndex(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index,
	const bool bLoop)
{
	if (!TargetArray || !ArrayProperty)
	{
		return INDEX_NONE;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 PreviousIndex = Index - 1;

	if(ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}

	if(PreviousIndex > ArrayHelper.Num() - 1)
	{
		return ArrayHelper.Num() - 1;
	}

	if (PreviousIndex >= 0)
	{
		return PreviousIndex;
	}

	if (bLoop)
	{
		return ArrayHelper.Num() - 1;
	}

	return 0;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetValidFirstItemCopy(const TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetValidLastItemCopy(const TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetValidItemFromIndexCopy(const TArray<int32>& TargetArray, const int32 Index, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetRandomItem(const TArray<int32>& TargetArray, int32& OutItem, int32& OutIndex)
{
	checkNoEntry();
	return false;
}

void UDirectiveUtilArrayFunctionLibrary::Array_LastValue(const TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
}

bool UDirectiveUtilArrayFunctionLibrary::Array_Pop(const TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_PopFirst(const TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_RemoveAtSwap(const TArray<int32>& TargetArray, const int32 Index)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetItemAtIndex(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index,
	void* OutItemPtr)
{
	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;

	if (!ArrayHelper.IsValidIndex(Index))
	{
		if (OutItemPtr)
		{
			InnerProp->ClearValue(OutItemPtr);
		}
		return false;
	}

	if (OutItemPtr)
	{
		InnerProp->CopyCompleteValueFromScriptVM(OutItemPtr, ArrayHelper.GetRawPtr(Index));
	}
	return true;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetFirstItem(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr)
{
	return GenericArray_GetItemAtIndex(TargetArray, ArrayProperty, 0, OutItemPtr);
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetLastItem(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr)
{
	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	return GenericArray_GetItemAtIndex(TargetArray, ArrayProperty, ArrayHelper.Num() - 1, OutItemPtr);
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetRandomItem(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr,
	int32* OutIndex)
{
	if (OutIndex)
	{
		*OutIndex = INDEX_NONE;
	}

	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 Num = ArrayHelper.Num();
	if (Num <= 0)
	{
		if (OutItemPtr)
		{
			ArrayProperty->Inner->ClearValue(OutItemPtr);
		}
		return false;
	}

	const int32 Index = FMath::RandRange(0, Num - 1);
	if (OutIndex)
	{
		*OutIndex = Index;
	}
	return GenericArray_GetItemAtIndex(TargetArray, ArrayProperty, Index, OutItemPtr);
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_Pop(
	void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr)
{
	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;
	const int32 LastIndex = ArrayHelper.Num() - 1;

	if (LastIndex < 0)
	{
		if (OutItemPtr)
		{
			InnerProp->ClearValue(OutItemPtr);
		}
		return false;
	}

	if (OutItemPtr)
	{
		InnerProp->CopyCompleteValueFromScriptVM(OutItemPtr, ArrayHelper.GetRawPtr(LastIndex));
	}
	ArrayHelper.RemoveValues(LastIndex, 1);
	return true;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_PopFirst(
	void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr)
{
	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;

	if (ArrayHelper.Num() <= 0)
	{
		if (OutItemPtr)
		{
			InnerProp->ClearValue(OutItemPtr);
		}
		return false;
	}

	if (OutItemPtr)
	{
		InnerProp->CopyCompleteValueFromScriptVM(OutItemPtr, ArrayHelper.GetRawPtr(0));
	}
	ArrayHelper.RemoveValues(0, 1);
	return true;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAtSwap(
	void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index)
{
	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	if (!ArrayHelper.IsValidIndex(Index))
	{
		return false;
	}

	const int32 LastIndex = ArrayHelper.Num() - 1;
	if (Index != LastIndex)
	{
		ArrayHelper.SwapValues(Index, LastIndex);
	}
	ArrayHelper.RemoveValues(LastIndex, 1);
	return true;
}

void UDirectiveUtilArrayFunctionLibrary::Array_Slice(const TArray<int32>& TargetArray, const int32 StartIndex, const int32 Count, TArray<int32>& OutArray)
{
	checkNoEntry();
}

void UDirectiveUtilArrayFunctionLibrary::Array_Rotate(const TArray<int32>& TargetArray, const int32 Shift)
{
	checkNoEntry();
}

void UDirectiveUtilArrayFunctionLibrary::Array_GetDistinct(const TArray<int32>& TargetArray, TArray<int32>& OutArray)
{
	checkNoEntry();
}

int32 UDirectiveUtilArrayFunctionLibrary::Array_CountOccurrences(const TArray<int32>& TargetArray, const int32& ItemToCount)
{
	checkNoEntry();
	return 0;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetMostCommon(const TArray<int32>& TargetArray, int32& OutItem, int32& OutCount)
{
	checkNoEntry();
	return false;
}

void UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(
	const void* TargetArray,
	const FArrayProperty* TargetArrayProperty,
	const int32 StartIndex,
	const int32 Count,
	void* OutArray,
	const FArrayProperty* OutArrayProperty)
{
	if (!TargetArray || !OutArray || !TargetArrayProperty || !OutArrayProperty)
	{
		return;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	FScriptArrayHelper OutHelper(OutArrayProperty, OutArray);
	OutHelper.EmptyValues();

	const int32 Num = SourceHelper.Num();
	if (Num == 0 || Count <= 0)
	{
		return;
	}

	const FProperty* InnerProp = TargetArrayProperty->Inner;
	const int32 Start = FMath::Clamp(StartIndex, 0, Num);
	const int32 NumToCopy = FMath::Min(Count, Num - Start);
	for (int32 Offset = 0; Offset < NumToCopy; ++Offset)
	{
		const int32 OutIndex = OutHelper.AddValue();
		InnerProp->CopySingleValueToScriptVM(OutHelper.GetRawPtr(OutIndex), SourceHelper.GetRawPtr(Start + Offset));
	}
}

void UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(
	void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Shift)
{
	if (!TargetArray || !ArrayProperty)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 Num = ArrayHelper.Num();
	if (Num <= 1)
	{
		return;
	}

	int32 Normalized = Shift % Num;
	if (Normalized < 0)
	{
		Normalized += Num;
	}
	if (Normalized == 0)
	{
		return;
	}

	auto ReverseRange = [&ArrayHelper](int32 Low, int32 High)
	{
		while (Low < High)
		{
			ArrayHelper.SwapValues(Low, High);
			++Low;
			--High;
		}
	};
	ReverseRange(0, Num - 1);
	ReverseRange(0, Normalized - 1);
	ReverseRange(Normalized, Num - 1);
}

void UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(
	const void* TargetArray,
	const FArrayProperty* TargetArrayProperty,
	void* OutArray,
	const FArrayProperty* OutArrayProperty)
{
	if (!TargetArray || !OutArray || !TargetArrayProperty || !OutArrayProperty)
	{
		return;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	FScriptArrayHelper OutHelper(OutArrayProperty, OutArray);
	OutHelper.EmptyValues();

	const FProperty* InnerProp = TargetArrayProperty->Inner;
	const int32 Num = SourceHelper.Num();
	for (int32 SourceIndex = 0; SourceIndex < Num; ++SourceIndex)
	{
		const uint8* SourceElement = SourceHelper.GetRawPtr(SourceIndex);

		bool bIsDuplicate = false;
		for (int32 ExistingIndex = 0; ExistingIndex < OutHelper.Num(); ++ExistingIndex)
		{
			if (InnerProp->Identical(SourceElement, OutHelper.GetRawPtr(ExistingIndex)))
			{
				bIsDuplicate = true;
				break;
			}
		}

		if (!bIsDuplicate)
		{
			const int32 OutIndex = OutHelper.AddValue();
			InnerProp->CopySingleValueToScriptVM(OutHelper.GetRawPtr(OutIndex), SourceElement);
		}
	}
}

int32 UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const void* ItemToCount)
{
	if (!TargetArray || !ArrayProperty || !ItemToCount)
	{
		return 0;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;

	int32 Count = 0;
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		if (InnerProp->Identical(ArrayHelper.GetRawPtr(Index), ItemToCount))
		{
			++Count;
		}
	}
	return Count;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	void* OutItemPtr,
	int32* OutCount)
{
	if (OutCount)
	{
		*OutCount = 0;
	}

	if (!TargetArray || !ArrayProperty)
	{
		return false;
	}

	FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const FProperty* InnerProp = ArrayProperty->Inner;
	const int32 Num = ArrayHelper.Num();
	if (Num == 0)
	{
		if (OutItemPtr)
		{
			InnerProp->ClearValue(OutItemPtr);
		}
		return false;
	}

	int32 BestIndex = 0;
	int32 BestCount = 0;
	for (int32 Index = 0; Index < Num; ++Index)
	{
		int32 CurrentCount = 0;
		for (int32 CompareIndex = 0; CompareIndex < Num; ++CompareIndex)
		{
			if (InnerProp->Identical(ArrayHelper.GetRawPtr(Index), ArrayHelper.GetRawPtr(CompareIndex)))
			{
				++CurrentCount;
			}
		}
		if (CurrentCount > BestCount)
		{
			BestCount = CurrentCount;
			BestIndex = Index;
		}
	}

	if (OutItemPtr)
	{
		InnerProp->CopyCompleteValueFromScriptVM(OutItemPtr, ArrayHelper.GetRawPtr(BestIndex));
	}
	if (OutCount)
	{
		*OutCount = BestCount;
	}
	return true;
}
