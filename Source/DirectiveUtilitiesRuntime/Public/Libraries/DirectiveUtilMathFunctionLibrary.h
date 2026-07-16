// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilMathTypes.h"
#include "DirectiveUtilMathFunctionLibrary.generated.h"

/**
 * UDirectiveUtilMathFunctionLibrary
 *
 * Contains math functions for the Directive Utilities plugin.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilMathFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise2D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Random")
	static float PerlinNoise2D(FVector2D Position);

	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise3D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Random")
	static float PerlinNoise3D(const FVector& Position);

	/**
	 * Returns the angle in degrees between two vectors.
	 * @param A - The first vector.
	 * @param B - The second vector.
	 * @returns The angle between the two vectors in degrees.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Vector")
	static float AngleBetweenVectors(const FVector& A, const FVector& B);

	/**
	 * Applies a Back/Elastic/Bounce easing curve to a normalized alpha.
	 * @note These are the Penner easing curves the engine's built-in "Ease" node (EEasingFunc) does not provide.
	 * For Sinusoidal/Exponential/Circular/power easings, use the engine's "Ease" node instead.
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased alpha. Note that Back and Elastic curves intentionally overshoot the [0, 1] range.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease Alpha", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static float EaseAlpha(float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a float from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start value (returned at Alpha 0).
	 * @param B - The target value (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased value between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Float)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static float EaseFloat(float A, float B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a vector from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start vector (returned at Alpha 0).
	 * @param B - The target vector (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased vector between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FVector EaseVector(const FVector& A, const FVector& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a rotator from A to B using a Back/Elastic/Bounce easing curve (shortest-path interpolation).
	 * @param A - The start rotator (returned at Alpha 0).
	 * @param B - The target rotator (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased rotator between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Rotator)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FRotator EaseRotator(const FRotator& A, const FRotator& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Eases a color from A to B using a Back/Elastic/Bounce easing curve.
	 * @param A - The start color (returned at Alpha 0).
	 * @param B - The target color (returned at Alpha 1).
	 * @param Alpha - The input alpha. Clamped to the [0, 1] range.
	 * @param EaseType - The easing curve to apply.
	 * @returns The eased color between A and B.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Color)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Easing")
	static FLinearColor EaseColor(const FLinearColor& A, const FLinearColor& B, float Alpha, EDirectiveUtilEaseType EaseType);

	/**
	 * Rounds a float to a given number of decimal places. Rounds half away from zero,
	 * matching "Round To Decimals (Text)".
	 * @note Due to floating-point representation the returned value may not display exactly;
	 * use "Round To Decimals (Text)" for clean display.
	 * @param Value - The value to round.
	 * @param Decimals - The number of decimal places to round to. Clamped to the [0, 10] range.
	 * @returns The rounded value.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Round To Decimals", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static float RoundToDecimals(float Value, int32 Decimals);

	/**
	 * Rounds a float to a given number of decimal places and returns it as display text.
	 * Rounds half away from zero, matching "Round To Decimals".
	 * @param Value - The value to round.
	 * @param Decimals - The maximum number of decimal places to display. Clamped to the [0, 10] range.
	 * @returns The rounded value as text, formatted with the current locale.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Round To Decimals (Text)", BlueprintThreadSafe), Category = "Directive Utilities|Math|Float")
	static FText RoundToDecimalsAsText(float Value, int32 Decimals);

	/**
	 * Formats a byte count as a human-readable size using binary units (1024): B, KB, MB, GB, TB, PB.
	 * Decimals are applied only from KB up ("532 B", "1.4 MB"). Negative input formats the absolute
	 * value with a leading minus sign. Output is English-only.
	 * @param Bytes - The byte count to format.
	 * @param Decimals - The number of decimal places to show from KB up. Clamped to the [0, 3] range.
	 * @returns The formatted size text.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Math|Formatting")
	static FText FormatBytes(int64 Bytes, int32 Decimals = 1);

	/**
	 * Formats a duration in seconds as d/h/m/s units from the largest nonzero unit down, with
	 * two-digit padding after the first ("1h 03m 05s", "2d 04h", "45s"). With bIncludeSeconds
	 * false the seconds unit is dropped and sub-minute durations return "0m". Negative input gets
	 * a leading minus sign when a nonzero unit remains; non-finite input returns "0s". Output is English-only.
	 * @param Seconds - The duration in seconds.
	 * @param bIncludeSeconds - Whether to include the seconds unit.
	 * @returns The formatted duration text.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Math|Formatting")
	static FText FormatDuration(float Seconds, bool bIncludeSeconds = true);

	/**
	 * Formats a timestamp relative to the current local time: "just now" (under a minute),
	 * "N minute(s)/hour(s)/day(s) ago", or "in N ..." for future timestamps. Uses local time,
	 * pairing with Get Save Slot Timestamp. Output is English-only.
	 * @note Not pure: reads the current clock each call.
	 * @param Timestamp - The local timestamp to describe.
	 * @returns The formatted relative time text.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Math|Formatting")
	static FText FormatRelativeTime(const FDateTime& Timestamp);

	/**
	 * Returns the sum of an integer array as a 64-bit integer, so large arrays cannot overflow int32.
	 * @param Values - The values to sum.
	 * @returns The sum of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static int64 GetIntArraySum(const TArray<int32>& Values);

	/**
	 * Returns the arithmetic mean of an integer array.
	 * @param Values - The values to average.
	 * @returns The average of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayAverage(const TArray<int32>& Values);

	/**
	 * Returns the median of an integer array (computed on a sorted copy; the input is not modified).
	 * For an even count, returns the average of the two middle values.
	 * @param Values - The values to take the median of.
	 * @returns The median of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayMedian(const TArray<int32>& Values);

	/**
	 * Returns the population standard deviation of an integer array (divides by N, not N-1).
	 * @param Values - The values to measure.
	 * @returns The population standard deviation, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetIntArrayStandardDeviation(const TArray<int32>& Values);

	/**
	 * Returns the sum of a float array. Accumulates in double internally for precision.
	 * @param Values - The values to sum.
	 * @returns The sum of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArraySum(const TArray<float>& Values);

	/**
	 * Returns the arithmetic mean of a float array. Accumulates in double internally for precision.
	 * @param Values - The values to average.
	 * @returns The average of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayAverage(const TArray<float>& Values);

	/**
	 * Returns the median of a float array (computed on a sorted copy; the input is not modified).
	 * For an even count, returns the average of the two middle values.
	 * @param Values - The values to take the median of.
	 * @returns The median of the values, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayMedian(const TArray<float>& Values);

	/**
	 * Returns the population standard deviation of a float array (divides by N, not N-1).
	 * Accumulates in double internally for precision.
	 * @param Values - The values to measure.
	 * @returns The population standard deviation, or 0 if the array is empty.
	 */
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Directive Utilities|Math|Array")
	static float GetFloatArrayStandardDeviation(const TArray<float>& Values);

	/**
	 * Returns a random index into the Weights array, where each index's probability is proportional to its weight.
	 * Useful for loot tables and weighted spawning. Negative and non-finite weights are treated as zero.
	 * @param Weights - The per-index weights.
	 * @returns The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Random Index From Weights"), Category = "Directive Utilities|Math|Random")
	static int32 GetRandomIndexFromWeights(const TArray<float>& Weights);

	/**
	 * Deterministic version of Get Random Index From Weights that draws from (and advances) the provided random stream.
	 * @param Stream - The random stream to draw from.
	 * @param Weights - The per-index weights. Negative and non-finite weights are treated as zero.
	 * @returns The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Random Index From Weights (Stream)"), Category = "Directive Utilities|Math|Random")
	static int32 GetRandomIndexFromWeightsFromStream(UPARAM(ref) FRandomStream& Stream, const TArray<float>& Weights);
};
