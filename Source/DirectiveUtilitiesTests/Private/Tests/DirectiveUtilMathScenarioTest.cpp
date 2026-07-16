#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include "Misc/AutomationTest.h"

namespace
{
	template <typename ValueType>
	double GetSortedMedian(TArray<ValueType> Values)
	{
		Values.Sort();
		const int32 Middle = Values.Num() / 2;
		if (Values.Num() % 2 != 0)
		{
			return static_cast<double>(Values[Middle]);
		}
		return (static_cast<double>(Values[Middle - 1]) + static_cast<double>(Values[Middle])) * 0.5;
	}

	TArray<int32> MakeMedianValues(const int32 Count, const int32 Pattern)
	{
		TArray<int32> Values;
		Values.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			switch (Pattern)
			{
			case 0:
				Values.Add(Index - Count / 2);
				break;
			case 1:
				Values.Add(Count - Index);
				break;
			case 2:
				Values.Add(Index % 7);
				break;
			default:
				Values.Add(Index % 2 == 0 ? MIN_int32 : MAX_int32);
				break;
			}
		}
		return Values;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilMathCardinalityTest,
	"DirectiveUtilities.MathScenarios.MedianCardinality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMathCardinalityTest::RunTest(const FString& Parameters)
{
	for (const int32 ItemCount : {1, 2, 3, 4, 5, 16, 17, 256, 257, 4096, 4097})
	{
		for (int32 Pattern = 0; Pattern < 4; ++Pattern)
		{
			const TArray<int32> IntegerValues = MakeMedianValues(ItemCount, Pattern);
			TArray<float> FloatValues;
			FloatValues.Reserve(ItemCount);
			for (const int32 Value : IntegerValues)
			{
				FloatValues.Add(static_cast<float>(Value) * 0.25f);
			}

			const FString Label = FString::Printf(TEXT("items=%d pattern=%d"), ItemCount, Pattern);
			TestTrue(
				Label + TEXT(" integer median"),
				FMath::IsNearlyEqual(
					UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(IntegerValues),
					static_cast<float>(GetSortedMedian(IntegerValues)),
					1.e-4f));
			TestTrue(
				Label + TEXT(" float median"),
				FMath::IsNearlyEqual(
					UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(FloatValues),
					static_cast<float>(GetSortedMedian(FloatValues)),
					1.e-4f));
		}
	}

	return true;
}
