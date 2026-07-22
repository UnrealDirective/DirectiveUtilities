// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"

#include "Algo/BinarySearch.h"
#include "Algo/Sort.h"
#include "Containers/ScriptArray.h"
#include "Misc/ComparisonUtility.h"

namespace
{
	constexpr int32 MaxSampleCount = 1'000'000;

	class FArrayResultBuilder
	{
	public:
		FArrayResultBuilder(const FArrayProperty* InArrayProperty, const int32 Count)
			: ArrayProperty(InArrayProperty)
			, ArrayHelper(InArrayProperty, &Array)
		{
			ArrayHelper.AddValues(Count);
		}

		~FArrayResultBuilder()
		{
			ArrayHelper.EmptyValues();
		}

		uint8* GetRawPtr(const int32 Index)
		{
			return ArrayHelper.GetRawPtr(Index);
		}

		void MoveTo(void* OutArray)
		{
			FScriptArrayHelper OutHelper(ArrayProperty, OutArray);
			OutHelper.MoveAssign(&Array);
		}

	private:
		FScriptArray Array;
		const FArrayProperty* ArrayProperty;
		FScriptArrayHelper ArrayHelper;
	};

	struct FPropertyValueKey
	{
		const FProperty* Property;
		const void* Value;
		uint32 Hash;

		friend uint32 GetTypeHash(const FPropertyValueKey& Key)
		{
			return Key.Hash;
		}

		friend bool operator==(const FPropertyValueKey& Left, const FPropertyValueKey& Right)
		{
			return Left.Property == Right.Property && Left.Property->Identical(Left.Value, Right.Value);
		}
	};

	struct FValueGroup
	{
		int32 SourceIndex;
		int32 Count;
	};

	struct FWeightedSampleCandidate
	{
		double Priority;
		int32 SourceIndex;
	};

	bool HaveMatchingElementTypes(const FArrayProperty* SourceProperty, const FArrayProperty* OutProperty)
	{
		return SourceProperty && OutProperty && SourceProperty->Inner->SameType(OutProperty->Inner);
	}

	void BuildValueGroups(
		FScriptArrayHelper& SourceHelper,
		const FProperty* InnerProperty,
		TArray<FValueGroup>& OutGroups)
	{
		const int32 Num = SourceHelper.Num();
		OutGroups.Reset(Num);
		if (InnerProperty->HasAllPropertyFlags(CPF_HasGetValueTypeHash))
		{
			TMap<FPropertyValueKey, int32> GroupIndices;
			GroupIndices.Reserve(Num);
			for (int32 SourceIndex = 0; SourceIndex < Num; ++SourceIndex)
			{
				const void* Value = SourceHelper.GetRawPtr(SourceIndex);
				const FPropertyValueKey Key{InnerProperty, Value, InnerProperty->GetValueTypeHash(Value)};
				if (const int32* GroupIndex = GroupIndices.Find(Key))
				{
					++OutGroups[*GroupIndex].Count;
					continue;
				}

				const int32 GroupIndex = OutGroups.Num();
				OutGroups.Add({SourceIndex, 1});
				GroupIndices.Add(Key, GroupIndex);
			}
			return;
		}

		for (int32 SourceIndex = 0; SourceIndex < Num; ++SourceIndex)
		{
			const void* Value = SourceHelper.GetRawPtr(SourceIndex);
			int32 GroupIndex = INDEX_NONE;
			for (int32 CandidateIndex = 0; CandidateIndex < OutGroups.Num(); ++CandidateIndex)
			{
				if (InnerProperty->Identical(Value, SourceHelper.GetRawPtr(OutGroups[CandidateIndex].SourceIndex)))
				{
					GroupIndex = CandidateIndex;
					break;
				}
			}

			if (GroupIndex == INDEX_NONE)
			{
				OutGroups.Add({SourceIndex, 1});
			}
			else
			{
				++OutGroups[GroupIndex].Count;
			}
		}
	}

	void CopyValues(
		FScriptArrayHelper& SourceHelper,
		const FProperty* SourceProperty,
		const TArray<int32>& SourceIndices,
		void* OutArray,
		const FArrayProperty* OutArrayProperty)
	{
		FArrayResultBuilder Result(OutArrayProperty, SourceIndices.Num());
		for (int32 OutIndex = 0; OutIndex < SourceIndices.Num(); ++OutIndex)
		{
			SourceProperty->CopySingleValueToScriptVM(
				Result.GetRawPtr(OutIndex),
				SourceHelper.GetRawPtr(SourceIndices[OutIndex]));
		}
		Result.MoveTo(OutArray);
	}
}

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
	if (ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}

	if (Index < -1)
	{
		return 0;
	}

	if (Index < ArrayHelper.Num() - 1)
	{
		return Index + 1;
	}

	if (bLoop)
	{
		return 0;
	}

	return ArrayHelper.Num() - 1;
}

void UDirectiveUtilArrayFunctionLibrary::Array_RemoveDuplicates(TArray<int32>& TargetArray)
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
	if (ArrayHelper.Num() <= 1)
	{
		return;
	}

	TArray<FValueGroup> Groups;
	BuildValueGroups(ArrayHelper, InnerProp, Groups);
	if (Groups.Num() == ArrayHelper.Num())
	{
		return;
	}

	TArray<int32> SourceIndices;
	SourceIndices.Reserve(Groups.Num());
	for (const FValueGroup& Group : Groups)
	{
		SourceIndices.Add(Group.SourceIndex);
	}
	CopyValues(ArrayHelper, InnerProp, SourceIndices, TargetArray, ArrayProperty);
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
	if (ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}

	if (Index > ArrayHelper.Num())
	{
		return ArrayHelper.Num() - 1;
	}

	if (Index > 0)
	{
		return Index - 1;
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

bool UDirectiveUtilArrayFunctionLibrary::Array_Pop(TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_PopFirst(TArray<int32>& TargetArray, int32& OutItem)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_RemoveAtSwap(TArray<int32>& TargetArray, const int32 Index)
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

void UDirectiveUtilArrayFunctionLibrary::Array_Rotate(TArray<int32>& TargetArray, const int32 Shift)
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
	if (!TargetArray || !OutArray || !HaveMatchingElementTypes(TargetArrayProperty, OutArrayProperty))
	{
		return;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	const int32 Num = SourceHelper.Num();
	if (Num == 0 || Count <= 0)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return;
	}

	const FProperty* InnerProp = TargetArrayProperty->Inner;
	const int32 Start = FMath::Clamp(StartIndex, 0, Num);
	const int32 NumToCopy = FMath::Min(Count, Num - Start);
	FArrayResultBuilder Result(OutArrayProperty, NumToCopy);
	for (int32 Offset = 0; Offset < NumToCopy; ++Offset)
	{
		InnerProp->CopySingleValueToScriptVM(Result.GetRawPtr(Offset), SourceHelper.GetRawPtr(Start + Offset));
	}
	Result.MoveTo(OutArray);
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

	const FProperty* InnerProperty = ArrayProperty->Inner;
	if (InnerProperty->HasAllPropertyFlags(CPF_IsPlainOldData))
	{
		const int32 ElementSize = InnerProperty->GetElementSize();
		const int32 LeftCount = Num - Normalized;
		const int32 ScratchCount = FMath::Min(LeftCount, Normalized);
		TArray<uint8> Scratch;
		Scratch.SetNumUninitialized(ScratchCount * ElementSize);
		uint8* Data = ArrayHelper.GetRawPtr(0);
		if (Normalized <= LeftCount)
		{
			FMemory::Memcpy(Scratch.GetData(), Data + LeftCount * ElementSize, Scratch.Num());
			FMemory::Memmove(Data + Normalized * ElementSize, Data, LeftCount * ElementSize);
			FMemory::Memcpy(Data, Scratch.GetData(), Scratch.Num());
		}
		else
		{
			FMemory::Memcpy(Scratch.GetData(), Data, Scratch.Num());
			FMemory::Memmove(Data, Data + LeftCount * ElementSize, Normalized * ElementSize);
			FMemory::Memcpy(Data + Normalized * ElementSize, Scratch.GetData(), Scratch.Num());
		}
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
	if (!TargetArray || !OutArray || !HaveMatchingElementTypes(TargetArrayProperty, OutArrayProperty))
	{
		return;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	const FProperty* InnerProp = TargetArrayProperty->Inner;
	TArray<FValueGroup> Groups;
	BuildValueGroups(SourceHelper, InnerProp, Groups);

	TArray<int32> SourceIndices;
	SourceIndices.Reserve(Groups.Num());
	for (const FValueGroup& Group : Groups)
	{
		SourceIndices.Add(Group.SourceIndex);
	}
	CopyValues(SourceHelper, InnerProp, SourceIndices, OutArray, OutArrayProperty);
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

	TArray<FValueGroup> Groups;
	BuildValueGroups(ArrayHelper, InnerProp, Groups);
	int32 BestGroupIndex = 0;
	for (int32 GroupIndex = 1; GroupIndex < Groups.Num(); ++GroupIndex)
	{
		if (Groups[GroupIndex].Count > Groups[BestGroupIndex].Count)
		{
			BestGroupIndex = GroupIndex;
		}
	}

	const FValueGroup& BestGroup = Groups[BestGroupIndex];
	if (OutItemPtr)
	{
		InnerProp->CopyCompleteValueFromScriptVM(OutItemPtr, ArrayHelper.GetRawPtr(BestGroup.SourceIndex));
	}
	if (OutCount)
	{
		*OutCount = BestGroup.Count;
	}
	return true;
}

void UDirectiveUtilArrayFunctionLibrary::Array_Sample(
	const TArray<int32>& TargetArray,
	const int32 Count,
	const bool bWithReplacement,
	TArray<int32>& OutArray)
{
	checkNoEntry();
}

void UDirectiveUtilArrayFunctionLibrary::Array_SampleFromStream(
	const TArray<int32>& TargetArray,
	const int32 Count,
	const bool bWithReplacement,
	FRandomStream& RandomStream,
	TArray<int32>& OutArray)
{
	checkNoEntry();
}

bool UDirectiveUtilArrayFunctionLibrary::Array_SampleWeighted(
	const TArray<int32>& TargetArray,
	const TArray<float>& Weights,
	const int32 Count,
	const bool bWithReplacement,
	TArray<int32>& OutArray)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_SampleWeightedFromStream(
	const TArray<int32>& TargetArray,
	const TArray<float>& Weights,
	const int32 Count,
	const bool bWithReplacement,
	FRandomStream& RandomStream,
	TArray<int32>& OutArray)
{
	checkNoEntry();
	return false;
}

void UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(
	const void* TargetArray,
	const FArrayProperty* TargetArrayProperty,
	const int32 Count,
	const bool bWithReplacement,
	FRandomStream* RandomStream,
	void* OutArray,
	const FArrayProperty* OutArrayProperty)
{
	if (!TargetArray || !OutArray || !HaveMatchingElementTypes(TargetArrayProperty, OutArrayProperty))
	{
		return;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	const int32 SourceCount = SourceHelper.Num();
	if (Count <= 0 || Count > MaxSampleCount || SourceCount == 0)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return;
	}

	const int32 SampleCount = bWithReplacement ? Count : FMath::Min(Count, SourceCount);
	FArrayResultBuilder Result(OutArrayProperty, SampleCount);
	const FProperty* InnerProperty = TargetArrayProperty->Inner;
	auto CopySourceElement = [&](const int32 OutIndex, const int32 SourceIndex)
	{
		InnerProperty->CopySingleValueToScriptVM(Result.GetRawPtr(OutIndex), SourceHelper.GetRawPtr(SourceIndex));
	};
	auto RandomIndex = [RandomStream](const int32 MaxIndex)
	{
		return RandomStream ? RandomStream->RandRange(0, MaxIndex) : FMath::RandRange(0, MaxIndex);
	};

	if (bWithReplacement)
	{
		for (int32 SampleIndex = 0; SampleIndex < Count; ++SampleIndex)
		{
			CopySourceElement(SampleIndex, RandomIndex(SourceCount - 1));
		}
		Result.MoveTo(OutArray);
		return;
	}

	if (SampleCount <= SourceCount / 4)
	{
		TMap<int32, int32> RemappedIndices;
		RemappedIndices.Reserve(SampleCount);
		auto ResolveIndex = [&RemappedIndices](const int32 Position)
		{
			if (const int32* RemappedIndex = RemappedIndices.Find(Position))
			{
				return *RemappedIndex;
			}
			return Position;
		};

		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			const int32 LastPosition = SourceCount - SampleIndex - 1;
			const int32 SelectedPosition = RandomIndex(LastPosition);
			const int32 SelectedSourceIndex = ResolveIndex(SelectedPosition);
			if (SelectedPosition != LastPosition)
			{
				const int32 LastSourceIndex = ResolveIndex(LastPosition);
				if (LastSourceIndex == SelectedPosition)
				{
					RemappedIndices.Remove(SelectedPosition);
				}
				else
				{
					RemappedIndices.Add(SelectedPosition, LastSourceIndex);
				}
			}
			RemappedIndices.Remove(LastPosition);
			CopySourceElement(SampleIndex, SelectedSourceIndex);
		}
		Result.MoveTo(OutArray);
		return;
	}

	TArray<int32> AvailableIndices;
	AvailableIndices.SetNumUninitialized(SourceCount);
	for (int32 Index = 0; Index < SourceCount; ++Index)
	{
		AvailableIndices[Index] = Index;
	}

	for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		const int32 SelectedIndex = RandomIndex(AvailableIndices.Num() - 1);
		CopySourceElement(SampleIndex, AvailableIndices[SelectedIndex]);
		AvailableIndices.RemoveAtSwap(SelectedIndex, 1, EAllowShrinking::No);
	}
	Result.MoveTo(OutArray);
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(
	const void* TargetArray,
	const FArrayProperty* TargetArrayProperty,
	const TArray<float>& Weights,
	const int32 Count,
	const bool bWithReplacement,
	FRandomStream* RandomStream,
	void* OutArray,
	const FArrayProperty* OutArrayProperty)
{
	if (!TargetArray || !OutArray || !HaveMatchingElementTypes(TargetArrayProperty, OutArrayProperty))
	{
		return false;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	const int32 SourceCount = SourceHelper.Num();
	if (Count < 0 || Count > MaxSampleCount || Weights.Num() != SourceCount)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return false;
	}
	if (Count == 0)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return true;
	}

	TArray<double> PositiveWeights;
	PositiveWeights.SetNumUninitialized(SourceCount);
	double TotalWeight = 0.0;
	int32 PositiveWeightCount = 0;
	for (int32 SourceIndex = 0; SourceIndex < SourceCount; ++SourceIndex)
	{
		const float Weight = Weights[SourceIndex];
		const double PositiveWeight = FMath::IsFinite(Weight) && Weight > 0.0f
			? static_cast<double>(Weight)
			: 0.0;
		PositiveWeights[SourceIndex] = PositiveWeight;
		TotalWeight += PositiveWeight;
		PositiveWeightCount += PositiveWeight > 0.0 ? 1 : 0;
	}

	if (PositiveWeightCount == 0)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return false;
	}

	const FProperty* InnerProperty = TargetArrayProperty->Inner;
	auto RandomFraction = [RandomStream]()
	{
		return static_cast<double>(RandomStream ? RandomStream->FRand() : FMath::FRand());
	};
	if (bWithReplacement)
	{
		double PrefixWeight = 0.0;
		for (double& Weight : PositiveWeights)
		{
			PrefixWeight += Weight;
			Weight = PrefixWeight;
		}

		FArrayResultBuilder Result(OutArrayProperty, Count);
		for (int32 SampleIndex = 0; SampleIndex < Count; ++SampleIndex)
		{
			const int32 SourceIndex = FMath::Min(
				Algo::UpperBound(PositiveWeights, RandomFraction() * TotalWeight),
				SourceCount - 1);
			InnerProperty->CopySingleValueToScriptVM(Result.GetRawPtr(SampleIndex), SourceHelper.GetRawPtr(SourceIndex));
		}
		Result.MoveTo(OutArray);
		return true;
	}

	const int32 SampleCount = FMath::Min(Count, PositiveWeightCount);
	auto IsLowerPriority = [](const FWeightedSampleCandidate& Left, const FWeightedSampleCandidate& Right)
	{
		return Left.Priority < Right.Priority ||
			(Left.Priority == Right.Priority && Left.SourceIndex > Right.SourceIndex);
	};
	auto IsHigherPriority = [&IsLowerPriority](const FWeightedSampleCandidate& Left, const FWeightedSampleCandidate& Right)
	{
		return IsLowerPriority(Right, Left);
	};

	TArray<FWeightedSampleCandidate> SelectedCandidates;
	SelectedCandidates.Reserve(SampleCount);
	for (int32 SourceIndex = 0; SourceIndex < SourceCount; ++SourceIndex)
	{
		const double Weight = PositiveWeights[SourceIndex];
		if (Weight <= 0.0)
		{
			continue;
		}

		const double RandomValue = FMath::Max(RandomFraction(), TNumericLimits<double>::Min());
		const FWeightedSampleCandidate Candidate{FMath::Loge(RandomValue) / Weight, SourceIndex};
		if (SelectedCandidates.Num() < SampleCount)
		{
			SelectedCandidates.HeapPush(Candidate, IsLowerPriority);
			continue;
		}

		if (IsHigherPriority(Candidate, SelectedCandidates[0]))
		{
			FWeightedSampleCandidate RemovedCandidate;
			SelectedCandidates.HeapPop(RemovedCandidate, IsLowerPriority, EAllowShrinking::No);
			SelectedCandidates.HeapPush(Candidate, IsLowerPriority);
		}
	}
	Algo::Sort(SelectedCandidates, IsHigherPriority);

	FArrayResultBuilder Result(OutArrayProperty, SampleCount);
	for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		const int32 SourceIndex = SelectedCandidates[SampleIndex].SourceIndex;
		InnerProperty->CopySingleValueToScriptVM(Result.GetRawPtr(SampleIndex), SourceHelper.GetRawPtr(SourceIndex));
	}
	Result.MoveTo(OutArray);
	return true;
}

bool UDirectiveUtilArrayFunctionLibrary::Array_GetPage(
	const TArray<int32>& TargetArray,
	const int32 PageIndex,
	const int32 PageSize,
	TArray<int32>& OutArray,
	int32& OutPageCount)
{
	checkNoEntry();
	return false;
}

bool UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(
	const void* TargetArray,
	const FArrayProperty* TargetArrayProperty,
	const int32 PageIndex,
	const int32 PageSize,
	void* OutArray,
	const FArrayProperty* OutArrayProperty,
	int32* OutPageCount)
{
	if (OutPageCount)
	{
		*OutPageCount = 0;
	}
	if (!TargetArray || !OutArray || !HaveMatchingElementTypes(TargetArrayProperty, OutArrayProperty))
	{
		return false;
	}

	FScriptArrayHelper SourceHelper(TargetArrayProperty, TargetArray);
	if (PageIndex < 0 || PageSize <= 0)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return false;
	}

	const int32 SourceCount = SourceHelper.Num();
	const int32 PageCount = SourceCount == 0 ? 0 : 1 + (SourceCount - 1) / PageSize;
	if (OutPageCount)
	{
		*OutPageCount = PageCount;
	}
	if (PageIndex >= PageCount)
	{
		FArrayResultBuilder EmptyResult(OutArrayProperty, 0);
		EmptyResult.MoveTo(OutArray);
		return false;
	}

	const int32 StartIndex = PageIndex * PageSize;
	const int32 EndIndex = StartIndex + FMath::Min(PageSize, SourceCount - StartIndex);
	const FProperty* InnerProperty = TargetArrayProperty->Inner;
	FArrayResultBuilder Result(OutArrayProperty, EndIndex - StartIndex);
	for (int32 SourceIndex = StartIndex; SourceIndex < EndIndex; ++SourceIndex)
	{
		InnerProperty->CopySingleValueToScriptVM(Result.GetRawPtr(SourceIndex - StartIndex), SourceHelper.GetRawPtr(SourceIndex));
	}
	Result.MoveTo(OutArray);
	return true;
}

void UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(TArray<FString>& TargetArray, const bool bDescending)
{
	struct FIndexedString
	{
		const FString* Value;
		int32 OriginalIndex;
	};

	TArray<FIndexedString> IndexedStrings;
	IndexedStrings.Reserve(TargetArray.Num());
	for (int32 Index = 0; Index < TargetArray.Num(); ++Index)
	{
		IndexedStrings.Add({&TargetArray[Index], Index});
	}

	Algo::Sort(IndexedStrings, [bDescending](const FIndexedString& Left, const FIndexedString& Right)
	{
		const int32 Comparison = UE::ComparisonUtility::CompareNaturalOrder(*Left.Value, *Right.Value);
		if (Comparison == 0)
		{
			return Left.OriginalIndex < Right.OriginalIndex;
		}
		return bDescending ? Comparison > 0 : Comparison < 0;
	});

	TArray<FString> SortedArray;
	SortedArray.Reserve(TargetArray.Num());
	for (const FIndexedString& IndexedString : IndexedStrings)
	{
		SortedArray.Add(MoveTemp(TargetArray[IndexedString.OriginalIndex]));
	}
	TargetArray = MoveTemp(SortedArray);
}

void UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(TArray<FName>& TargetArray, const bool bDescending)
{
	struct FIndexedName
	{
		FString Value;
		int32 OriginalIndex;
	};

	TArray<FIndexedName> IndexedNames;
	IndexedNames.Reserve(TargetArray.Num());
	for (int32 Index = 0; Index < TargetArray.Num(); ++Index)
	{
		IndexedNames.Add({TargetArray[Index].ToString(), Index});
	}

	Algo::Sort(IndexedNames, [bDescending](const FIndexedName& Left, const FIndexedName& Right)
	{
		const int32 Comparison = UE::ComparisonUtility::CompareNaturalOrder(Left.Value, Right.Value);
		if (Comparison == 0)
		{
			return Left.OriginalIndex < Right.OriginalIndex;
		}
		return bDescending ? Comparison > 0 : Comparison < 0;
	});

	TArray<FName> SortedArray;
	SortedArray.Reserve(TargetArray.Num());
	for (const FIndexedName& IndexedName : IndexedNames)
	{
		SortedArray.Add(TargetArray[IndexedName.OriginalIndex]);
	}
	TargetArray = MoveTemp(SortedArray);
}
