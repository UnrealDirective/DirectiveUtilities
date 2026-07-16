#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilMathFunctionLibraryTest, "DirectiveUtilities.MathFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMathFunctionLibraryTest::RunTest(const FString& Parameters)
{
	TestEqual("AngleBetweenVectors should return 0 for parallel vectors",
		UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(FVector::ForwardVector, FVector::ForwardVector), 0.0f);
	TestTrue("AngleBetweenVectors should return ~90 for perpendicular vectors",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(FVector::ForwardVector, FVector::RightVector), 90.0f, 0.01f));
	TestTrue("AngleBetweenVectors should return ~180 for opposite vectors",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(FVector::ForwardVector, -FVector::ForwardVector), 180.0f, 0.01f));
	TestTrue("AngleBetweenVectors should handle zero vector gracefully",
		FMath::IsFinite(UDirectiveUtilMathFunctionLibrary::AngleBetweenVectors(FVector::ZeroVector, FVector::ForwardVector)));

	{
		const FVector2D Sample2D(12.34f, 56.78f);
		const float Noise2DFirst = UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(Sample2D);
		const float Noise2DSecond = UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(Sample2D);
		TestEqual("PerlinNoise2D should be deterministic for the same input", Noise2DFirst, Noise2DSecond);
		TestTrue("PerlinNoise2D should return a finite value", FMath::IsFinite(Noise2DFirst));
		TestTrue("PerlinNoise2D should be within [-1, 1]",
			Noise2DFirst >= -1.0f - 1.e-4f && Noise2DFirst <= 1.0f + 1.e-4f);
	}

	{
		const FVector2D Samples2D[] = {
			FVector2D(0.5f, 0.5f),
			FVector2D(-3.25f, 7.1f),
			FVector2D(100.123f, -200.456f),
			FVector2D(0.0f, 0.0f)
		};
		for (const FVector2D& Sample : Samples2D)
		{
			const float Value = UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(Sample);
			TestTrue(FString::Printf(TEXT("PerlinNoise2D should be finite at %s"), *Sample.ToString()), FMath::IsFinite(Value));
			TestTrue(FString::Printf(TEXT("PerlinNoise2D should be within [-1,1] at %s"), *Sample.ToString()),
				Value >= -1.0f - 1.e-4f && Value <= 1.0f + 1.e-4f);
		}
	}

	TestTrue("PerlinNoise2D should be ~0 at an integer lattice point",
		FMath::IsNearlyZero(UDirectiveUtilMathFunctionLibrary::PerlinNoise2D(FVector2D(3.0f, 4.0f)), 1.e-4f));

	{
		const FVector Sample3D(12.34f, 56.78f, 90.12f);
		const float Noise3DFirst = UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(Sample3D);
		const float Noise3DSecond = UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(Sample3D);
		TestEqual("PerlinNoise3D should be deterministic for the same input", Noise3DFirst, Noise3DSecond);
		TestTrue("PerlinNoise3D should return a finite value", FMath::IsFinite(Noise3DFirst));
		TestTrue("PerlinNoise3D should be within [-1, 1]",
			Noise3DFirst >= -1.0f - 1.e-4f && Noise3DFirst <= 1.0f + 1.e-4f);
	}

	{
		const FVector Samples3D[] = {
			FVector(0.5f, 0.5f, 0.5f),
			FVector(-3.25f, 7.1f, -1.9f),
			FVector(100.123f, -200.456f, 33.7f),
			FVector(0.0f, 0.0f, 0.0f)
		};
		for (const FVector& Sample : Samples3D)
		{
			const float Value = UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(Sample);
			TestTrue(FString::Printf(TEXT("PerlinNoise3D should be finite at %s"), *Sample.ToString()), FMath::IsFinite(Value));
			TestTrue(FString::Printf(TEXT("PerlinNoise3D should be within [-1,1] at %s"), *Sample.ToString()),
				Value >= -1.0f - 1.e-4f && Value <= 1.0f + 1.e-4f);
		}
	}

	TestTrue("PerlinNoise3D should be ~0 at an integer lattice point",
		FMath::IsNearlyZero(UDirectiveUtilMathFunctionLibrary::PerlinNoise3D(FVector(3.0f, 4.0f, 5.0f)), 1.e-4f));

	const TArray<EDirectiveUtilEaseType> AllEaseTypes = {
		EDirectiveUtilEaseType::BackIn, EDirectiveUtilEaseType::BackOut, EDirectiveUtilEaseType::BackInOut,
		EDirectiveUtilEaseType::ElasticIn, EDirectiveUtilEaseType::ElasticOut, EDirectiveUtilEaseType::ElasticInOut,
		EDirectiveUtilEaseType::BounceIn, EDirectiveUtilEaseType::BounceOut, EDirectiveUtilEaseType::BounceInOut
	};
	for (const EDirectiveUtilEaseType EaseType : AllEaseTypes)
	{
		const FString TypeName = FString::FromInt(static_cast<int32>(EaseType));
		TestTrue(FString::Printf(TEXT("EaseAlpha(0) should be ~0 for type %s"), *TypeName),
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::EaseAlpha(0.0f, EaseType), 0.0f, 1.e-3f));
		TestTrue(FString::Printf(TEXT("EaseAlpha(1) should be ~1 for type %s"), *TypeName),
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::EaseAlpha(1.0f, EaseType), 1.0f, 1.e-3f));
		for (const float Sample : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
		{
			TestTrue(FString::Printf(TEXT("EaseAlpha(%.2f) should be finite for type %s"), Sample, *TypeName),
				FMath::IsFinite(UDirectiveUtilMathFunctionLibrary::EaseAlpha(Sample, EaseType)));
		}
	}

	TestEqual("EaseAlpha should clamp alpha below 0",
		UDirectiveUtilMathFunctionLibrary::EaseAlpha(-1.0f, EDirectiveUtilEaseType::BounceOut),
		UDirectiveUtilMathFunctionLibrary::EaseAlpha(0.0f, EDirectiveUtilEaseType::BounceOut));
	TestEqual("EaseAlpha should clamp alpha above 1",
		UDirectiveUtilMathFunctionLibrary::EaseAlpha(2.0f, EDirectiveUtilEaseType::BounceOut),
		UDirectiveUtilMathFunctionLibrary::EaseAlpha(1.0f, EDirectiveUtilEaseType::BounceOut));

	for (const EDirectiveUtilEaseType BounceType : {EDirectiveUtilEaseType::BounceIn, EDirectiveUtilEaseType::BounceOut, EDirectiveUtilEaseType::BounceInOut})
	{
		for (const float Sample : {0.1f, 0.3f, 0.6f, 0.9f})
		{
			const float Eased = UDirectiveUtilMathFunctionLibrary::EaseAlpha(Sample, BounceType);
			TestTrue("Bounce easing should stay within [0,1]", Eased >= -1.e-3f && Eased <= 1.0f + 1.e-3f);
		}
	}

	TestTrue("EaseFloat at alpha 0 returns A",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::EaseFloat(10.0f, 20.0f, 0.0f, EDirectiveUtilEaseType::BounceOut), 10.0f, 1.e-3f));
	TestTrue("EaseFloat at alpha 1 returns B",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::EaseFloat(10.0f, 20.0f, 1.0f, EDirectiveUtilEaseType::BounceOut), 20.0f, 1.e-3f));
	TestTrue("EaseVector at alpha 1 returns B",
		UDirectiveUtilMathFunctionLibrary::EaseVector(FVector::ZeroVector, FVector(1, 2, 3), 1.0f, EDirectiveUtilEaseType::BounceOut).Equals(FVector(1, 2, 3), 1.e-2f));
	TestTrue("EaseRotator at alpha 0 returns A",
		UDirectiveUtilMathFunctionLibrary::EaseRotator(FRotator(10, 20, 30), FRotator(40, 50, 60), 0.0f, EDirectiveUtilEaseType::BounceOut).Equals(FRotator(10, 20, 30), 1.e-1f));
	TestTrue("EaseColor at alpha 1 returns B",
		UDirectiveUtilMathFunctionLibrary::EaseColor(FLinearColor::Black, FLinearColor::White, 1.0f, EDirectiveUtilEaseType::BounceOut).Equals(FLinearColor::White, 1.e-2f));

	TestTrue("RoundToDecimals(3.14159, 2) ~= 3.14",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(3.14159f, 2), 3.14f, 1.e-4f));
	TestTrue("RoundToDecimals(2.71828, 2) ~= 2.72 (rounds up)",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(2.71828f, 2), 2.72f, 1.e-4f));
	TestTrue("RoundToDecimals(-1.2367, 2) ~= -1.24",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(-1.2367f, 2), -1.24f, 1.e-4f));
	TestTrue("RoundToDecimals with 0 decimals rounds to integer",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(1.6f, 0), 2.0f, 1.e-4f));
	TestTrue("RoundToDecimals(10.0, 5) ~= 10.0",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(10.0f, 5), 10.0f, 1.e-4f));
	TestTrue("RoundToDecimals clamps excessive decimals without crashing",
		FMath::IsFinite(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(1.23456789f, 50)));
	TestTrue("RoundToDecimalsAsText(3.14159, 2) reads ~3.14",
		FMath::IsNearlyEqual(FCString::Atof(*UDirectiveUtilMathFunctionLibrary::RoundToDecimalsAsText(3.14159f, 2).ToString()), 3.14f, 1.e-2f));
	TestTrue("RoundToDecimals(2.5, 0) rounds half away from zero to 3",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(2.5f, 0), 3.0f, 1.e-4f));
	TestTrue("RoundToDecimals(-2.5, 0) rounds half away from zero to -3",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(-2.5f, 0), -3.0f, 1.e-4f));
	TestTrue("RoundToDecimals(0.125, 2) rounds half away from zero to 0.13",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RoundToDecimals(0.125f, 2), 0.13f, 0.0001f));
	TestEqual("RoundToDecimalsAsText(2.5, 0) rounds half away from zero to \"3\"",
		UDirectiveUtilMathFunctionLibrary::RoundToDecimalsAsText(2.5f, 0).ToString(), FString(TEXT("3")));
	TestEqual("RoundToDecimalsAsText(-2.5, 0) rounds half away from zero to \"-3\"",
		UDirectiveUtilMathFunctionLibrary::RoundToDecimalsAsText(-2.5f, 0).ToString(), FString(TEXT("-3")));

	TestEqual("Weighted random on an empty array returns INDEX_NONE",
		UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights(TArray<float>()), static_cast<int32>(INDEX_NONE));
	TestEqual("Weighted random with all-zero weights returns INDEX_NONE",
		UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights({0.0f, 0.0f, 0.0f}), static_cast<int32>(INDEX_NONE));
	TestEqual("Weighted random ignores non-finite weights",
		UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights({std::numeric_limits<float>::quiet_NaN(), 1.0f, std::numeric_limits<float>::infinity()}), 1);
	TestTrue("Weighted random handles large finite totals",
		UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights({MAX_flt, MAX_flt}) != INDEX_NONE);
	for (int32 Iteration = 0; Iteration < 25; ++Iteration)
	{
		TestEqual("Weighted random {0,1,0} always selects index 1",
			UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights({0.0f, 1.0f, 0.0f}), 1);
		TestEqual("Weighted random {5,0,0} always selects index 0",
			UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights({5.0f, 0.0f, 0.0f}), 0);
	}

	TestEqual("FormatBytes(532) is \"532 B\"",
		UDirectiveUtilMathFunctionLibrary::FormatBytes(532).ToString(), FString(TEXT("532 B")));
	TestEqual("FormatBytes(1536) is \"1.5 KB\"",
		UDirectiveUtilMathFunctionLibrary::FormatBytes(1536).ToString(), FString(TEXT("1.5 KB")));
	TestEqual("FormatBytes(1450000, 1) is \"1.4 MB\"",
		UDirectiveUtilMathFunctionLibrary::FormatBytes(1450000, 1).ToString(), FString(TEXT("1.4 MB")));
	TestEqual("FormatBytes(0) is \"0 B\"",
		UDirectiveUtilMathFunctionLibrary::FormatBytes(0).ToString(), FString(TEXT("0 B")));

	TestEqual("FormatDuration(3785) is \"1h 03m 05s\"",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(3785.0f).ToString(), FString(TEXT("1h 03m 05s")));
	TestEqual("FormatDuration(3785, false) is \"1h 03m\"",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(3785.0f, false).ToString(), FString(TEXT("1h 03m")));
	TestEqual("FormatDuration(45) is \"45s\"",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(45.0f).ToString(), FString(TEXT("45s")));
	TestEqual("FormatDuration(-90) is \"-1m 30s\"",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(-90.0f).ToString(), FString(TEXT("-1m 30s")));
	TestEqual("FormatDuration(-45, false) is \"0m\"",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(-45.0f, false).ToString(), FString(TEXT("0m")));
	TestFalse("FormatDuration handles the largest finite float without wrapping negative",
		UDirectiveUtilMathFunctionLibrary::FormatDuration(MAX_flt).ToString().StartsWith(TEXT("-")));

	TestEqual("FormatRelativeTime 5 minutes back reads \"5 minutes ago\"",
		UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(FDateTime::Now() - FTimespan::FromMinutes(5)).ToString(), FString(TEXT("5 minutes ago")));
	TestEqual("FormatRelativeTime 10 seconds back reads \"just now\"",
		UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(FDateTime::Now() - FTimespan::FromSeconds(10)).ToString(), FString(TEXT("just now")));
	TestEqual("FormatRelativeTime 2 hours ahead reads \"in 2 hours\"",
		UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(FDateTime::Now() + FTimespan::FromHours(2)).ToString(), FString(TEXT("in 2 hours")));
	TestEqual("FormatRelativeTime 1 minute back reads \"1 minute ago\"",
		UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(FDateTime::Now() - FTimespan::FromMinutes(1)).ToString(), FString(TEXT("1 minute ago")));

	{
		const TArray<int32> IntValues = {1, 2, 3, 4};
		TestEqual("GetIntArraySum({1,2,3,4}) is 10",
			UDirectiveUtilMathFunctionLibrary::GetIntArraySum(IntValues), static_cast<int64>(10));
		TestTrue("GetIntArrayAverage({1,2,3,4}) is 2.5",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayAverage(IntValues), 2.5f, 1.e-4f));
		TestTrue("GetIntArrayMedian({1,2,3,4}) averages the two middle values to 2.5",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(IntValues), 2.5f, 1.e-4f));
		TestTrue("GetIntArrayMedian({1,2,3}) is 2",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian({1, 2, 3}), 2.0f, 1.e-4f));
		TestTrue("GetIntArrayMedian handles an unsorted input",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian({3, 1, 2}), 2.0f, 1.e-4f));
		TestTrue("GetIntArrayStandardDeviation({2,4,4,4,5,5,7,9}) is the population value 2",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayStandardDeviation({2, 4, 4, 4, 5, 5, 7, 9}), 2.0f, 1.e-4f));
		TestEqual("GetIntArraySum({MAX_int32, MAX_int32}) does not overflow",
			UDirectiveUtilMathFunctionLibrary::GetIntArraySum({MAX_int32, MAX_int32}), static_cast<int64>(MAX_int32) * 2);

		const TArray<float> FloatValues = {1.0f, 2.0f, 3.0f, 4.0f};
		TestTrue("GetFloatArraySum({1,2,3,4}) is 10",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArraySum(FloatValues), 10.0f, 1.e-4f));
		TestTrue("GetFloatArrayAverage({1,2,3,4}) is 2.5",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArrayAverage(FloatValues), 2.5f, 1.e-4f));
		TestTrue("GetFloatArrayMedian({1,2,3,4}) averages the two middle values to 2.5",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(FloatValues), 2.5f, 1.e-4f));
		TestTrue("GetFloatArrayMedian({1,2,3}) is 2",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian({1.0f, 2.0f, 3.0f}), 2.0f, 1.e-4f));
		TestTrue("GetFloatArrayStandardDeviation({2,4,4,4,5,5,7,9}) is the population value 2",
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArrayStandardDeviation({2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f}), 2.0f, 1.e-4f));

		const TArray<int32> EmptyInts;
		const TArray<float> EmptyFloats;
		TestEqual("GetIntArraySum of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetIntArraySum(EmptyInts), static_cast<int64>(0));
		TestEqual("GetIntArrayAverage of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetIntArrayAverage(EmptyInts), 0.0f);
		TestEqual("GetIntArrayMedian of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(EmptyInts), 0.0f);
		TestEqual("GetIntArrayStandardDeviation of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetIntArrayStandardDeviation(EmptyInts), 0.0f);
		TestEqual("GetFloatArraySum of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetFloatArraySum(EmptyFloats), 0.0f);
		TestEqual("GetFloatArrayAverage of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetFloatArrayAverage(EmptyFloats), 0.0f);
		TestEqual("GetFloatArrayMedian of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(EmptyFloats), 0.0f);
		TestEqual("GetFloatArrayStandardDeviation of an empty array is 0",
			UDirectiveUtilMathFunctionLibrary::GetFloatArrayStandardDeviation(EmptyFloats), 0.0f);
	}

	TestTrue("GetIntArrayMedian should average the full int32 range without overflow",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian({MIN_int32, MAX_int32}), -0.5f, 1.e-4f));
	TestEqual("GetIntArrayMedian should handle repeated values",
		UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian({7, 7, 7, 7, 7}), 7.0f);
	TestEqual("GetFloatArrayMedian should handle repeated values",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian({7.5f, 7.5f, 7.5f, 7.5f}), 7.5f);

	FRandomStream MedianStream(481516);
	for (int32 Iteration = 0; Iteration < 200; ++Iteration)
	{
		const int32 Count = MedianStream.RandRange(1, 257);
		TArray<int32> IntValues;
		TArray<float> FloatValues;
		IntValues.Reserve(Count);
		FloatValues.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const int32 Value = MedianStream.RandRange(-1000, 1000);
			IntValues.Add(Value);
			FloatValues.Add(static_cast<float>(Value) * 0.25f);
		}

		TArray<int32> SortedInts = IntValues;
		SortedInts.Sort();
		const int32 Middle = SortedInts.Num() / 2;
		const float ExpectedIntMedian = SortedInts.Num() % 2 == 0
			? static_cast<float>((static_cast<double>(SortedInts[Middle - 1]) + static_cast<double>(SortedInts[Middle])) * 0.5)
			: static_cast<float>(SortedInts[Middle]);
		TestTrue(
			FString::Printf(TEXT("Integer median fuzz case %d"), Iteration),
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(IntValues), ExpectedIntMedian, 1.e-4f));

		TArray<float> SortedFloats = FloatValues;
		SortedFloats.Sort();
		const float ExpectedFloatMedian = SortedFloats.Num() % 2 == 0
			? static_cast<float>((static_cast<double>(SortedFloats[Middle - 1]) + static_cast<double>(SortedFloats[Middle])) * 0.5)
			: SortedFloats[Middle];
		TestTrue(
			FString::Printf(TEXT("Float median fuzz case %d"), Iteration),
			FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(FloatValues), ExpectedFloatMedian, 1.e-4f));
	}

	FRandomStream StreamA(12345);
	FRandomStream StreamB(12345);
	const int32 StreamIndexA = UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeightsFromStream(StreamA, {1.0f, 1.0f, 1.0f, 1.0f});
	const int32 StreamIndexB = UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeightsFromStream(StreamB, {1.0f, 1.0f, 1.0f, 1.0f});
	TestEqual("Weighted random from stream is deterministic for the same seed", StreamIndexA, StreamIndexB);
	TestTrue("Weighted random from stream returns a valid index", StreamIndexA >= 0 && StreamIndexA < 4);
	FRandomStream EmptyStream(1);
	TestEqual("Weighted random from stream with all-zero weights returns INDEX_NONE",
		UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeightsFromStream(EmptyStream, {0.0f, 0.0f}), static_cast<int32>(INDEX_NONE));

	return true;
}
