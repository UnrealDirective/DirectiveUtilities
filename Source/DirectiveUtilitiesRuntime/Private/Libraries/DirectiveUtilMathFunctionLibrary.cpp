// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

namespace
{
	double EaseBackIn(double t)
	{
		const double s = 1.70158;
		return t * t * ((s + 1.0) * t - s);
	}

	double EaseBackOut(double t)
	{
		const double s = 1.70158;
		t -= 1.0;
		return t * t * ((s + 1.0) * t + s) + 1.0;
	}

	double EaseBackInOut(double t)
	{
		const double s = 1.70158 * 1.525;
		t *= 2.0;
		if (t < 1.0)
		{
			return 0.5 * (t * t * ((s + 1.0) * t - s));
		}
		t -= 2.0;
		return 0.5 * (t * t * ((s + 1.0) * t + s) + 2.0);
	}

	double EaseElasticIn(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3;
		const double s = p / 4.0;
		t -= 1.0;
		return -(FMath::Pow(2.0, 10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p));
	}

	double EaseElasticOut(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3;
		const double s = p / 4.0;
		return FMath::Pow(2.0, -10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p) + 1.0;
	}

	double EaseElasticInOut(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3 * 1.5;
		const double s = p / 4.0;
		t *= 2.0;
		if (t < 1.0)
		{
			t -= 1.0;
			return -0.5 * (FMath::Pow(2.0, 10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p));
		}
		t -= 1.0;
		return FMath::Pow(2.0, -10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p) * 0.5 + 1.0;
	}

	double EaseBounceOut(double t)
	{
		const double n1 = 7.5625;
		const double d1 = 2.75;
		if (t < 1.0 / d1)
		{
			return n1 * t * t;
		}
		if (t < 2.0 / d1)
		{
			t -= 1.5 / d1;
			return n1 * t * t + 0.75;
		}
		if (t < 2.5 / d1)
		{
			t -= 2.25 / d1;
			return n1 * t * t + 0.9375;
		}
		t -= 2.625 / d1;
		return n1 * t * t + 0.984375;
	}

	double EaseBounceIn(double t)
	{
		return 1.0 - EaseBounceOut(1.0 - t);
	}

	double EaseBounceInOut(double t)
	{
		return t < 0.5
			? (1.0 - EaseBounceOut(1.0 - 2.0 * t)) * 0.5
			: (1.0 + EaseBounceOut(2.0 * t - 1.0)) * 0.5;
	}

	float GetUsableWeight(const float Weight)
	{
		return FMath::IsFinite(Weight) && Weight > 0.0f ? Weight : 0.0f;
	}

	template <typename ValueType>
	ValueType SelectNth(TArray<ValueType>& Values, const int32 NthIndex)
	{
		int32 Left = 0;
		int32 Right = Values.Num() - 1;
		int32 RemainingDepth = FMath::FloorLog2(static_cast<uint32>(Values.Num())) * 2;
		while (Left < Right)
		{
			if (RemainingDepth-- <= 0)
			{
				Values.Sort();
				return Values[NthIndex];
			}

			const int32 Middle = Left + (Right - Left) / 2;
			if (Values[Middle] < Values[Left])
			{
				Values.Swap(Middle, Left);
			}
			if (Values[Right] < Values[Left])
			{
				Values.Swap(Right, Left);
			}
			if (Values[Right] < Values[Middle])
			{
				Values.Swap(Right, Middle);
			}
			const ValueType Pivot = Values[Middle];

			int32 LessEnd = Left;
			int32 Current = Left;
			int32 GreaterStart = Right;
			while (Current <= GreaterStart)
			{
				if (Values[Current] < Pivot)
				{
					Values.Swap(LessEnd++, Current++);
				}
				else if (Pivot < Values[Current])
				{
					Values.Swap(Current, GreaterStart--);
				}
				else
				{
					++Current;
				}
			}

			if (NthIndex < LessEnd)
			{
				Right = LessEnd - 1;
			}
			else if (NthIndex > GreaterStart)
			{
				Left = GreaterStart + 1;
			}
			else
			{
				return Values[NthIndex];
			}
		}
		return Values[Left];
	}

	template <typename ValueType>
	double CalculateMedian(TArray<ValueType>& Values)
	{
		const int32 Middle = Values.Num() / 2;
		const ValueType UpperMiddle = SelectNth(Values, Middle);
		if (Values.Num() % 2 != 0)
		{
			return static_cast<double>(UpperMiddle);
		}

		ValueType LowerMiddle = Values[0];
		for (int32 Index = 1; Index < Middle; ++Index)
		{
			LowerMiddle = FMath::Max(LowerMiddle, Values[Index]);
		}
		return (static_cast<double>(LowerMiddle) + static_cast<double>(UpperMiddle)) * 0.5;
	}
}

float UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(const FVector2D Position)
{
	return FMath::PerlinNoise2D(Position);
}

float UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(const FVector& Position)
{
	return FMath::PerlinNoise3D(Position);
}

float UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(const FVector& A, const FVector& B)
{
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()), -1.0, 1.0)));
}

float UDirectiveUtilMathFunctionLibrary::EaseAlpha(const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	const double t = static_cast<double>(FMath::Clamp(Alpha, 0.0f, 1.0f));
	double Result = t;
	switch (EaseType)
	{
	case EDirectiveUtilEaseType::BackIn: Result = EaseBackIn(t); break;
	case EDirectiveUtilEaseType::BackOut: Result = EaseBackOut(t); break;
	case EDirectiveUtilEaseType::BackInOut: Result = EaseBackInOut(t); break;
	case EDirectiveUtilEaseType::ElasticIn: Result = EaseElasticIn(t); break;
	case EDirectiveUtilEaseType::ElasticOut: Result = EaseElasticOut(t); break;
	case EDirectiveUtilEaseType::ElasticInOut: Result = EaseElasticInOut(t); break;
	case EDirectiveUtilEaseType::BounceIn: Result = EaseBounceIn(t); break;
	case EDirectiveUtilEaseType::BounceOut: Result = EaseBounceOut(t); break;
	case EDirectiveUtilEaseType::BounceInOut: Result = EaseBounceInOut(t); break;
	}
	return static_cast<float>(Result);
}

float UDirectiveUtilMathFunctionLibrary::EaseFloat(const float A, const float B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, EaseAlpha(Alpha, EaseType));
}

FVector UDirectiveUtilMathFunctionLibrary::EaseVector(const FVector& A, const FVector& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, static_cast<double>(EaseAlpha(Alpha, EaseType)));
}

FRotator UDirectiveUtilMathFunctionLibrary::EaseRotator(const FRotator& A, const FRotator& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FQuat::Slerp(A.Quaternion(), B.Quaternion(), EaseAlpha(Alpha, EaseType)).Rotator();
}

FLinearColor UDirectiveUtilMathFunctionLibrary::EaseColor(const FLinearColor& A, const FLinearColor& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, EaseAlpha(Alpha, EaseType));
}

float UDirectiveUtilMathFunctionLibrary::RoundToDecimals(const float Value, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 10);
	if (Decimals == 0)
	{
		return FMath::RoundHalfFromZero(Value);
	}
	const double Factor = FMath::Pow(10.0, static_cast<double>(Decimals));
	return static_cast<float>(FMath::RoundHalfFromZero(static_cast<double>(Value) * Factor) / Factor);
}

FText UDirectiveUtilMathFunctionLibrary::RoundToDecimalsAsText(const float Value, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 10);
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 0;
	Options.MaximumFractionalDigits = Decimals;
	Options.RoundingMode = ERoundingMode::HalfFromZero;
	return FText::AsNumber(Value, &Options);
}

FText UDirectiveUtilMathFunctionLibrary::FormatBytes(const int64 Bytes, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 3);

	static const TCHAR* Suffixes[] = { TEXT("B"), TEXT("KB"), TEXT("MB"), TEXT("GB"), TEXT("TB"), TEXT("PB") };
	const bool bNegative = Bytes < 0;
	double Value = FMath::Abs(static_cast<double>(Bytes));
	int32 SuffixIndex = 0;
	while (Value >= 1024.0 && SuffixIndex < UE_ARRAY_COUNT(Suffixes) - 1)
	{
		Value /= 1024.0;
		++SuffixIndex;
	}

	return FText::FromString(FString::Printf(TEXT("%s%.*f %s"),
		bNegative ? TEXT("-") : TEXT(""), SuffixIndex == 0 ? 0 : Decimals, Value, Suffixes[SuffixIndex]));
}

FText UDirectiveUtilMathFunctionLibrary::FormatDuration(const float Seconds, const bool bIncludeSeconds)
{
	if (!FMath::IsFinite(Seconds))
	{
		return FText::FromString(TEXT("0s"));
	}

	const double AbsoluteSeconds = FMath::Abs(static_cast<double>(Seconds));
	const int64 TotalSeconds = AbsoluteSeconds >= static_cast<double>(TNumericLimits<int64>::Max())
		? TNumericLimits<int64>::Max()
		: static_cast<int64>(AbsoluteSeconds);
	const int64 VisibleSeconds = bIncludeSeconds ? TotalSeconds : (TotalSeconds / 60) * 60;
	const bool bNegative = Seconds < 0.0f && VisibleSeconds > 0;

	const int64 UnitValues[] = { TotalSeconds / 86400, (TotalSeconds / 3600) % 24, (TotalSeconds / 60) % 60, TotalSeconds % 60 };
	static const TCHAR* UnitSuffixes[] = { TEXT("d"), TEXT("h"), TEXT("m"), TEXT("s") };
	const int32 NumUnits = bIncludeSeconds ? 4 : 3;

	int32 FirstUnit = NumUnits - 1;
	for (int32 Index = 0; Index < NumUnits; ++Index)
	{
		if (UnitValues[Index] != 0)
		{
			FirstUnit = Index;
			break;
		}
	}
	int32 LastUnit = FirstUnit;
	for (int32 Index = NumUnits - 1; Index >= FirstUnit; --Index)
	{
		if (UnitValues[Index] != 0)
		{
			LastUnit = Index;
			break;
		}
	}

	FString Result = bNegative ? TEXT("-") : TEXT("");
	for (int32 Index = FirstUnit; Index <= LastUnit; ++Index)
	{
		if (Index == FirstUnit)
		{
			Result += FString::Printf(TEXT("%lld%s"), UnitValues[Index], UnitSuffixes[Index]);
		}
		else
		{
			Result += FString::Printf(TEXT(" %02lld%s"), UnitValues[Index], UnitSuffixes[Index]);
		}
	}
	return FText::FromString(Result);
}

FText UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(const FDateTime& Timestamp)
{
	const FTimespan Delta = FDateTime::Now() - Timestamp;
	const bool bFuture = Delta.GetTicks() < 0;
	// Round to whole seconds so clock-adjacent inputs (e.g. Now() + 2 hours) land in the intended bucket.
	const int64 SecondsAbs = static_cast<int64>(FMath::RoundToDouble(FMath::Abs(Delta.GetTotalSeconds())));

	if (SecondsAbs < 60)
	{
		return FText::FromString(TEXT("just now"));
	}

	int64 Count;
	const TCHAR* Unit;
	if (SecondsAbs < 3600)
	{
		Count = SecondsAbs / 60;
		Unit = TEXT("minute");
	}
	else if (SecondsAbs < 86400)
	{
		Count = SecondsAbs / 3600;
		Unit = TEXT("hour");
	}
	else
	{
		Count = SecondsAbs / 86400;
		Unit = TEXT("day");
	}

	const FString Quantity = FString::Printf(TEXT("%lld %s%s"), Count, Unit, Count == 1 ? TEXT("") : TEXT("s"));
	return FText::FromString(bFuture
		? FString::Printf(TEXT("in %s"), *Quantity)
		: FString::Printf(TEXT("%s ago"), *Quantity));
}

int64 UDirectiveUtilMathFunctionLibrary::GetIntArraySum(const TArray<int32>& Values)
{
	int64 Sum = 0;
	for (const int32 Value : Values)
	{
		Sum += Value;
	}
	return Sum;
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayAverage(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}
	return static_cast<float>(static_cast<double>(GetIntArraySum(Values)) / Values.Num());
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	TArray<int32> WorkingValues = Values;
	return static_cast<float>(CalculateMedian(WorkingValues));
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayStandardDeviation(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	const double Mean = static_cast<double>(GetIntArraySum(Values)) / Values.Num();
	double SquaredDeltaSum = 0.0;
	for (const int32 Value : Values)
	{
		const double Delta = static_cast<double>(Value) - Mean;
		SquaredDeltaSum += Delta * Delta;
	}
	return static_cast<float>(FMath::Sqrt(SquaredDeltaSum / Values.Num()));
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArraySum(const TArray<float>& Values)
{
	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	return static_cast<float>(Sum);
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayAverage(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	return static_cast<float>(Sum / Values.Num());
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	TArray<float> WorkingValues = Values;
	if (WorkingValues.ContainsByPredicate([](const float Value) { return FMath::IsNaN(Value); }))
	{
		WorkingValues.Sort();
		const int32 Middle = WorkingValues.Num() / 2;
		if (WorkingValues.Num() % 2 == 0)
		{
			return static_cast<float>((static_cast<double>(WorkingValues[Middle - 1]) + static_cast<double>(WorkingValues[Middle])) * 0.5);
		}
		return WorkingValues[Middle];
	}
	return static_cast<float>(CalculateMedian(WorkingValues));
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayStandardDeviation(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	const double Mean = Sum / Values.Num();

	double SquaredDeltaSum = 0.0;
	for (const float Value : Values)
	{
		const double Delta = static_cast<double>(Value) - Mean;
		SquaredDeltaSum += Delta * Delta;
	}
	return static_cast<float>(FMath::Sqrt(SquaredDeltaSum / Values.Num()));
}

int32 UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights(const TArray<float>& Weights)
{
	double Total = 0.0;
	for (const float Weight : Weights)
	{
		Total += GetUsableWeight(Weight);
	}

	if (Total <= 0.0)
	{
		return INDEX_NONE;
	}

	const double Roll = static_cast<double>(FMath::FRand()) * Total;
	double Accumulated = 0.0;
	int32 LastPositiveIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Weights.Num(); ++Index)
	{
		const float Weight = GetUsableWeight(Weights[Index]);
		if (Weight <= 0.0f)
		{
			continue;
		}
		LastPositiveIndex = Index;
		Accumulated += Weight;
		if (Roll < Accumulated)
		{
			return Index;
		}
	}

	return LastPositiveIndex;
}

int32 UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeightsFromStream(FRandomStream& Stream, const TArray<float>& Weights)
{
	double Total = 0.0;
	for (const float Weight : Weights)
	{
		Total += GetUsableWeight(Weight);
	}

	if (Total <= 0.0)
	{
		return INDEX_NONE;
	}

	const double Roll = static_cast<double>(Stream.FRand()) * Total;
	double Accumulated = 0.0;
	int32 LastPositiveIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Weights.Num(); ++Index)
	{
		const float Weight = GetUsableWeight(Weights[Index]);
		if (Weight <= 0.0f)
		{
			continue;
		}
		LastPositiveIndex = Index;
		Accumulated += Weight;
		if (Roll < Accumulated)
		{
			return Index;
		}
	}

	return LastPositiveIndex;
}
