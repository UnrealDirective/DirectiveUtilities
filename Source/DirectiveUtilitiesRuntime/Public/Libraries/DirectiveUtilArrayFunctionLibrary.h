// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Net/Core/PushModel/PushModel.h"
#include "DirectiveUtilArrayFunctionLibrary.generated.h"

/**
 * UDirectiveUtilArrayFunctionLibrary
 * A collection of array utility functions that improve the usability of arrays in Blueprints.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilArrayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Returns the next index in the array.
	* If the next index is greater than the last array index and bLoop is enabled, the index will loop back to the start of the array.
	 * Otherwise, the last index will be returned.
	 * Returns INDEX_NONE for an empty array.
	 * @param TargetArray - The array to get the next index for.
	 * @param Index - The current index.
	 * @param bLoop - If true, the index will loop back to the beginning of the array when the next index is greater than the last array index.
	 * Otherwise, the last index will be returned.
	 * @returns The next index in the array.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Next Index", CompactNodeTitle = "NEXT INDEX", ArrayParm = "TargetArray", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static int32 Array_NextIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);

	/**
	 * Returns the previous index in the array.
	 * If the previous index is less than 0 and bLoop is enabled, the index will loop back to the end of the array.
	 * Otherwise, 0 will be returned.
	 * Returns INDEX_NONE for an empty array.
	 * @param TargetArray - The array to get the previous index for.
	 * @param Index - The current index.
	 * @param bLoop - If the next index is greater than the last array index and bLoop is enabled, the index will loop back to the start of the array.
	 * Otherwise, the last index will be returned.
	 * @returns The previous index in the array.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Previous Index", CompactNodeTitle = "PREV INDEX", ArrayParm = "TargetArray", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static int32 Array_PreviousIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);

	/**
	 * Removes duplicate elements from the array in-place.
	 * @param TargetArray - The array to remove duplicates from.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Remove Duplicates", CompactNodeTitle = "REMOVE DUPLICATES", ArrayParm = "TargetArray"), Category="Directive Utilities|Array")
	static void Array_RemoveDuplicates(UPARAM(ref) TArray<int32>& TargetArray);

	/**
	 * Returns a copy of the first element of the array.
	 * @param TargetArray - The array to read from.
	 * @param OutItem - [out] A copy of the first element, or the default value if the array is empty.
	 * @returns True if the array contained a valid first element.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Valid First Array Item (Copy)", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static bool Array_GetValidFirstItemCopy(const TArray<int32>& TargetArray, int32& OutItem);

	/**
	 * Returns a copy of the last element of the array.
	 * @param TargetArray - The array to read from.
	 * @param OutItem - [out] A copy of the last element, or the default value if the array is empty.
	 * @returns True if the array contained a valid last element.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Valid Last Array Item (Copy)", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static bool Array_GetValidLastItemCopy(const TArray<int32>& TargetArray, int32& OutItem);

	/**
	 * Returns a copy of the element at the given index, if the index is valid.
	 * @param TargetArray - The array to read from.
	 * @param Index - The index to read.
	 * @param OutItem - [out] A copy of the element, or the default value if the index is invalid.
	 * @returns True if the index was valid.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Valid Array Item From Index (Copy)", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static bool Array_GetValidItemFromIndexCopy(const TArray<int32>& TargetArray, const int32 Index, int32& OutItem);

	/**
	 * Returns a copy of a random element from the array.
	 * @param TargetArray - The array to read from.
	 * @param OutItem - [out] A copy of the randomly selected element, or the default value if the array is empty.
	 * @param OutIndex - [out] The index of the selected element, or INDEX_NONE if the array is empty.
	 * @returns True if a valid element was selected.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Get Random Valid Array Item", CompactNodeTitle = "RANDOM", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem"), Category="Directive Utilities|Array")
	static bool Array_GetRandomItem(const TArray<int32>& TargetArray, int32& OutItem, int32& OutIndex);

	/**
	 * Returns a copy of the last element of the array without removing it.
	 * @param TargetArray - The array to read from.
	 * @param OutItem - [out] A copy of the last element, or the default value if the array is empty.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Last Value", CompactNodeTitle = "LAST", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static void Array_LastValue(const TArray<int32>& TargetArray, int32& OutItem);

	/**
	 * Removes the last element of the array and returns a copy of it.
	 * @param TargetArray - The array to pop from.
	 * @param OutItem - [out] A copy of the removed element, or the default value if the array is empty.
	 * @returns True if an element was removed.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Pop", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem"), Category="Directive Utilities|Array")
	static bool Array_Pop(UPARAM(ref) TArray<int32>& TargetArray, int32& OutItem);

	/**
	 * Removes the first element of the array and returns a copy of it.
	 * @param TargetArray - The array to pop from.
	 * @param OutItem - [out] A copy of the removed element, or the default value if the array is empty.
	 * @returns True if an element was removed.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Pop First", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem"), Category="Directive Utilities|Array")
	static bool Array_PopFirst(UPARAM(ref) TArray<int32>& TargetArray, int32& OutItem);

	/**
	 * Removes the element at the given index by swapping it with the last element (does not preserve order).
	 * This is O(1) but changes the position of the previously-last element.
	 * @param TargetArray - The array to remove from.
	 * @param Index - The index to remove.
	 * @returns True if the index was valid and an element was removed.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Remove At Swap", ArrayParm = "TargetArray"), Category="Directive Utilities|Array")
	static bool Array_RemoveAtSwap(UPARAM(ref) TArray<int32>& TargetArray, const int32 Index);

	/**
	 * Returns a copy of a contiguous range of the array. The range is clamped to the array bounds.
	 * @param TargetArray - The array to slice.
	 * @param StartIndex - The index to start copying from (clamped to [0, Length]).
	 * @param Count - The number of elements to copy. Values <= 0 produce an empty array.
	 * @param OutArray - [out] The sliced copy.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Slice", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static void Array_Slice(const TArray<int32>& TargetArray, const int32 StartIndex, const int32 Count, TArray<int32>& OutArray);

	/**
	 * Cyclically rotates the elements of the array in place.
	 * @param TargetArray - The array to rotate.
	 * @param Shift - The number of positions to rotate. Positive rotates toward the end; negative toward the start.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Rotate", ArrayParm = "TargetArray"), Category="Directive Utilities|Array")
	static void Array_Rotate(UPARAM(ref) TArray<int32>& TargetArray, const int32 Shift);

	/**
	 * Returns a copy of the array with duplicates removed, keeping the first occurrence and preserving order.
	 * Unlike Remove Duplicates, this does not modify the input array.
	 * @param TargetArray - The array to read from.
	 * @param OutArray - [out] The de-duplicated copy.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Distinct (Copy)", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static void Array_GetDistinct(const TArray<int32>& TargetArray, TArray<int32>& OutArray);

	/**
	 * Counts how many times an item appears in the array.
	 * @param TargetArray - The array to search.
	 * @param ItemToCount - The item to count.
	 * @returns The number of occurrences.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Count Occurrences", ArrayParm = "TargetArray", ArrayTypeDependentParams = "ItemToCount", AutoCreateRefTerm = "ItemToCount", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static int32 Array_CountOccurrences(const TArray<int32>& TargetArray, const int32& ItemToCount);

	/**
	 * Returns the most frequently occurring element of the array (ties resolve to the earliest such element).
	 * @param TargetArray - The array to read from.
	 * @param OutItem - [out] A copy of the most common element, or the default value if the array is empty.
	 * @param OutCount - [out] The number of times the most common element occurs.
	 * @returns True if the array was non-empty.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Most Common", ArrayParm = "TargetArray", ArrayTypeDependentParams = "OutItem", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static bool Array_GetMostCommon(const TArray<int32>& TargetArray, int32& OutItem, int32& OutCount);

	/**
	 * Returns randomly selected elements from the array.
	 * @param TargetArray The array to sample.
	 * @param Count The requested number of elements.
	 * @param bWithReplacement Whether the same source element can be selected more than once.
	 * @param OutArray The sampled elements.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Sample Array", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray"), Category="Directive Utilities|Array")
	static void Array_Sample(const TArray<int32>& TargetArray, int32 Count, bool bWithReplacement, TArray<int32>& OutArray);

	/**
	 * Returns randomly selected elements using a random stream.
	 * @param TargetArray The array to sample.
	 * @param Count The requested number of elements.
	 * @param bWithReplacement Whether the same source element can be selected more than once.
	 * @param RandomStream The stream used to select elements.
	 * @param OutArray The sampled elements.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Sample Array from Stream", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray"), Category="Directive Utilities|Array")
	static void Array_SampleFromStream(const TArray<int32>& TargetArray, int32 Count, bool bWithReplacement, UPARAM(ref) FRandomStream& RandomStream, TArray<int32>& OutArray);

	/**
	 * Returns randomly selected elements using per-element weights.
	 * @param TargetArray The array to sample.
	 * @param Weights The selection weight for each source element.
	 * @param Count The requested number of elements.
	 * @param bWithReplacement Whether the same source element can be selected more than once.
	 * @param OutArray The sampled elements.
	 * @returns True when the inputs were valid and the sample was produced.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Sample Weighted Array", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray"), Category="Directive Utilities|Array")
	static bool Array_SampleWeighted(const TArray<int32>& TargetArray, const TArray<float>& Weights, int32 Count, bool bWithReplacement, TArray<int32>& OutArray);

	/**
	 * Returns randomly selected elements using per-element weights and a random stream.
	 * @param TargetArray The array to sample.
	 * @param Weights The selection weight for each source element.
	 * @param Count The requested number of elements.
	 * @param bWithReplacement Whether the same source element can be selected more than once.
	 * @param RandomStream The stream used to select elements.
	 * @param OutArray The sampled elements.
	 * @returns True when the inputs were valid and the sample was produced.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(DisplayName = "Sample Weighted Array from Stream", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray"), Category="Directive Utilities|Array")
	static bool Array_SampleWeightedFromStream(const TArray<int32>& TargetArray, const TArray<float>& Weights, int32 Count, bool bWithReplacement, UPARAM(ref) FRandomStream& RandomStream, TArray<int32>& OutArray);

	/**
	 * Returns one zero-based page from the array.
	 * @param TargetArray The array to read.
	 * @param PageIndex The zero-based page index.
	 * @param PageSize The maximum number of elements per page.
	 * @param OutArray The requested page.
	 * @param OutPageCount The total number of pages.
	 * @returns True when the page index and size are valid.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Get Array Page", ArrayParm = "TargetArray,OutArray", ArrayTypeDependentParams = "OutArray", BlueprintThreadSafe), Category="Directive Utilities|Array")
	static bool Array_GetPage(const TArray<int32>& TargetArray, int32 PageIndex, int32 PageSize, TArray<int32>& OutArray, int32& OutPageCount);

	/** Sorts strings in natural order so embedded numbers are compared numerically. */
	UFUNCTION(BlueprintCallable, Category="Directive Utilities|Array")
	static void NaturalSortStringArray(UPARAM(ref) TArray<FString>& TargetArray, bool bDescending = false);

	/** Sorts names in natural order so embedded numbers are compared numerically. */
	UFUNCTION(BlueprintCallable, Category="Directive Utilities|Array")
	static void NaturalSortNameArray(UPARAM(ref) TArray<FName>& TargetArray, bool bDescending = false);


	/*~
	 * Native functions that will be called by the below custom thunk layers, which read off the property address and call the appropriate native handler.
	 * Based off UKismetArrayLibrary implementation
	 ~*/

	static int32 GenericArray_NextIndex(const void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index, bool bLoop);
	static int32 GenericArray_PreviousIndex(const void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index, bool bLoop);
	static void GenericArray_RemoveDuplicates(void* TargetArray, const FArrayProperty* ArrayProperty);
	static bool GenericArray_GetItemAtIndex(const void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index, void* OutItemPtr);
	static bool GenericArray_GetFirstItem(const void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr);
	static bool GenericArray_GetLastItem(const void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr);
	static bool GenericArray_GetRandomItem(const void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr, int32* OutIndex);
	static bool GenericArray_Pop(void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr);
	static bool GenericArray_PopFirst(void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr);
	static bool GenericArray_RemoveAtSwap(void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index);
	static void GenericArray_Slice(const void* TargetArray, const FArrayProperty* TargetArrayProperty, int32 StartIndex, int32 Count, void* OutArray, const FArrayProperty* OutArrayProperty);
	static void GenericArray_Rotate(void* TargetArray, const FArrayProperty* ArrayProperty, int32 Shift);
	static void GenericArray_GetDistinct(const void* TargetArray, const FArrayProperty* TargetArrayProperty, void* OutArray, const FArrayProperty* OutArrayProperty);
	static int32 GenericArray_CountOccurrences(const void* TargetArray, const FArrayProperty* ArrayProperty, const void* ItemToCount);
	static bool GenericArray_GetMostCommon(const void* TargetArray, const FArrayProperty* ArrayProperty, void* OutItemPtr, int32* OutCount);
	static void GenericArray_Sample(const void* TargetArray, const FArrayProperty* TargetArrayProperty, int32 Count, bool bWithReplacement, FRandomStream* RandomStream, void* OutArray, const FArrayProperty* OutArrayProperty);
	static bool GenericArray_SampleWeighted(const void* TargetArray, const FArrayProperty* TargetArrayProperty, const TArray<float>& Weights, int32 Count, bool bWithReplacement, FRandomStream* RandomStream, void* OutArray, const FArrayProperty* OutArrayProperty);
	static bool GenericArray_GetPage(const void* TargetArray, const FArrayProperty* TargetArrayProperty, int32 PageIndex, int32 PageSize, void* OutArray, const FArrayProperty* OutArrayProperty, int32* OutPageCount);

	/*~
	 * Custom thunk layers that read off the property address and call the appropriate native handler.
	 * Based off UKismetArrayLibrary implementation
	 ~*/

	DECLARE_FUNCTION(execArray_NextIndex)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);
		P_GET_UBOOL(bLoop);
		P_FINISH;
		P_NATIVE_BEGIN;

		*static_cast<int32*>(RESULT_PARAM) = GenericArray_NextIndex(ArrayAddr, ArrayProperty, Index, bLoop);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_PreviousIndex)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);
		P_GET_UBOOL(bLoop);
		P_FINISH;
		P_NATIVE_BEGIN;

		*static_cast<int32*>(RESULT_PARAM) = GenericArray_PreviousIndex(ArrayAddr, ArrayProperty, Index, bLoop);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_RemoveDuplicates)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, ArrayProperty);
		GenericArray_RemoveDuplicates(ArrayAddr, ArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_GetValidFirstItemCopy)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetFirstItem(ArrayAddr, ArrayProperty, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_GetValidLastItemCopy)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetLastItem(ArrayAddr, ArrayProperty, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_GetValidItemFromIndexCopy)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetItemAtIndex(ArrayAddr, ArrayProperty, Index, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_GetRandomItem)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.StepCompiledIn<FProperty>(nullptr);
		int32* OutIndex = reinterpret_cast<int32*>(Stack.MostRecentPropertyAddress);

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetRandomItem(ArrayAddr, ArrayProperty, ItemPtr, OutIndex);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_LastValue)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_GetLastItem(ArrayAddr, ArrayProperty, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_Pop)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, ArrayProperty);
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_Pop(ArrayAddr, ArrayProperty, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_PopFirst)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, ArrayProperty);
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_PopFirst(ArrayAddr, ArrayProperty, ItemPtr);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_RemoveAtSwap)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);
		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, ArrayProperty);
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_RemoveAtSwap(ArrayAddr, ArrayProperty, Index);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_Slice)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, StartIndex);
		P_GET_PROPERTY(FIntProperty, Count);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_Slice(SourceArrayAddr, SourceArrayProperty, StartIndex, Count, OutArrayAddr, OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_Rotate)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Shift);
		P_FINISH;
		P_NATIVE_BEGIN;
		MARK_PROPERTY_DIRTY(Stack.Object, ArrayProperty);
		GenericArray_Rotate(ArrayAddr, ArrayProperty, Shift);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_GetDistinct)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_GetDistinct(SourceArrayAddr, SourceArrayProperty, OutArrayAddr, OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_CountOccurrences)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<int32*>(RESULT_PARAM) = GenericArray_CountOccurrences(ArrayAddr, ArrayProperty, StorageSpace);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_GetMostCommon)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		const FProperty* InnerProp = ArrayProperty->Inner;
		const int32 PropertySize = InnerProp->GetElementSize() * InnerProp->ArrayDim;
		void* StorageSpace = FMemory_Alloca(PropertySize);
		InnerProp->InitializeValue(StorageSpace);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentPropertyContainer = nullptr;
		Stack.StepCompiledIn<FProperty>(StorageSpace);
		void* ItemPtr;
		if (Stack.MostRecentPropertyAddress != nullptr && Stack.MostRecentProperty != nullptr
			&& PropertySize == Stack.MostRecentProperty->GetElementSize() * Stack.MostRecentProperty->ArrayDim
			&& (Stack.MostRecentProperty->GetClass()->IsChildOf(InnerProp->GetClass())
				|| InnerProp->GetClass()->IsChildOf(Stack.MostRecentProperty->GetClass())))
		{
			ItemPtr = Stack.MostRecentPropertyAddress;
		}
		else
		{
			ItemPtr = StorageSpace;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.StepCompiledIn<FProperty>(nullptr);
		int32* OutCount = reinterpret_cast<int32*>(Stack.MostRecentPropertyAddress);

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetMostCommon(ArrayAddr, ArrayProperty, ItemPtr, OutCount);
		P_NATIVE_END;
		InnerProp->DestroyValue(StorageSpace);
	}

	DECLARE_FUNCTION(execArray_Sample)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Count);
		P_GET_UBOOL(bWithReplacement);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_Sample(SourceArrayAddr, SourceArrayProperty, Count, bWithReplacement, nullptr, OutArrayAddr, OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_SampleFromStream)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Count);
		P_GET_UBOOL(bWithReplacement);
		P_GET_STRUCT_REF(FRandomStream, RandomStream);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		GenericArray_Sample(SourceArrayAddr, SourceArrayProperty, Count, bWithReplacement, &RandomStream, OutArrayAddr, OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_SampleWeighted)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_TARRAY_REF(float, Weights);
		P_GET_PROPERTY(FIntProperty, Count);
		P_GET_UBOOL(bWithReplacement);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_SampleWeighted(
			SourceArrayAddr,
			SourceArrayProperty,
			Weights,
			Count,
			bWithReplacement,
			nullptr,
			OutArrayAddr,
			OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_SampleWeightedFromStream)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_TARRAY_REF(float, Weights);
		P_GET_PROPERTY(FIntProperty, Count);
		P_GET_UBOOL(bWithReplacement);
		P_GET_STRUCT_REF(FRandomStream, RandomStream);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_SampleWeighted(
			SourceArrayAddr,
			SourceArrayProperty,
			Weights,
			Count,
			bWithReplacement,
			&RandomStream,
			OutArrayAddr,
			OutArrayProperty);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_GetPage)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* SourceArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* SourceArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!SourceArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, PageIndex);
		P_GET_PROPERTY(FIntProperty, PageSize);

		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		void* OutArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* OutArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!OutArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}

		Stack.MostRecentProperty = nullptr;
		Stack.MostRecentPropertyAddress = nullptr;
		Stack.StepCompiledIn<FProperty>(nullptr);
		int32* OutPageCount = reinterpret_cast<int32*>(Stack.MostRecentPropertyAddress);

		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = GenericArray_GetPage(SourceArrayAddr, SourceArrayProperty, PageIndex, PageSize, OutArrayAddr, OutArrayProperty, OutPageCount);
		P_NATIVE_END;
	}
};
