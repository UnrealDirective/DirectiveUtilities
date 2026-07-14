# Math Function Library

> Contains math functions for the Directive Utilities plugin.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilMathFunctionLibrary.h`

---

## Perlin Noise 2D
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static float PerlinNoise2D(FVector2D Position);
```

Returns a perlin noise value between -1 and 1 at the given position. Exposes the engine's built-in PerlinNoise2D function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| Position | `FVector2D` | The position to get the noise value for. |

**Returns:** The noise value at the given position.

## Perlin Noise 3D
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static float PerlinNoise3D(const FVector& Position);
```

Returns a perlin noise value between -1 and 1 at the given position. Exposes the engine's built-in PerlinNoise3D function to Blueprints.

| Parameter | Type | Description |
|-----------|------|-------------|
| Position | `const FVector&` | The position to get the noise value for. |

**Returns:** The noise value at the given position.

## Angle Between Vectors
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Vector`

```cpp
static float AngleBetweenVectors(const FVector& A, const FVector& B);
```

Returns the angle in degrees between two vectors.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FVector&` | The first vector. |
| B | `const FVector&` | The second vector. |

**Returns:** The angle between the two vectors in degrees.

## Ease Alpha
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static float EaseAlpha(float Alpha, EDirectiveUtilEaseType EaseType);
```

Applies a Back/Elastic/Bounce easing curve to a normalized alpha. These are the Penner easing curves the engine's built-in "Ease" node (EEasingFunc) does not provide; for Sinusoidal/Exponential/Circular/power easings, use the engine's "Ease" node instead.

| Parameter | Type | Description |
|-----------|------|-------------|
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased alpha. Back and Elastic curves intentionally overshoot the [0, 1] range.

## Ease (Float)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static float EaseFloat(float A, float B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a float from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `float` | The start value (returned at Alpha 0). |
| B | `float` | The target value (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased value between A and B.

## Ease (Vector)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FVector EaseVector(const FVector& A, const FVector& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a vector from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FVector&` | The start vector (returned at Alpha 0). |
| B | `const FVector&` | The target vector (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased vector between A and B.

## Ease (Rotator)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FRotator EaseRotator(const FRotator& A, const FRotator& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a rotator from A to B using a Back/Elastic/Bounce easing curve (shortest-path interpolation).

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FRotator&` | The start rotator (returned at Alpha 0). |
| B | `const FRotator&` | The target rotator (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased rotator between A and B.

## Ease (Color)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Easing`

```cpp
static FLinearColor EaseColor(const FLinearColor& A, const FLinearColor& B, float Alpha, EDirectiveUtilEaseType EaseType);
```

Eases a color from A to B using a Back/Elastic/Bounce easing curve.

| Parameter | Type | Description |
|-----------|------|-------------|
| A | `const FLinearColor&` | The start color (returned at Alpha 0). |
| B | `const FLinearColor&` | The target color (returned at Alpha 1). |
| Alpha | `float` | The input alpha. Clamped to the [0, 1] range. |
| EaseType | `EDirectiveUtilEaseType` | The easing curve to apply. |

**Returns:** The eased color between A and B.

## Round To Decimals
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static float RoundToDecimals(float Value, int32 Decimals);
```

Rounds a float to a given number of decimal places, rounding halves away from zero (matching "Round To Decimals (Text)"). Due to floating-point representation the returned value may not display exactly; use "Round To Decimals (Text)" for clean display.

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to round. |
| Decimals | `int32` | The number of decimal places to round to. Clamped to the [0, 10] range. |

**Returns:** The rounded value.

## Round To Decimals (Text)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Float`

```cpp
static FText RoundToDecimalsAsText(float Value, int32 Decimals);
```

Rounds a float to a given number of decimal places and returns it as display text, rounding halves away from zero (matching "Round To Decimals").

| Parameter | Type | Description |
|-----------|------|-------------|
| Value | `float` | The value to round. |
| Decimals | `int32` | The maximum number of decimal places to display. Clamped to the [0, 10] range. |

**Returns:** The rounded value as text, formatted with the current locale.

## Format Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatBytes(int64 Bytes, int32 Decimals = 1);
```

Formats a byte count as a human-readable size using binary units (1024): B, KB, MB, GB, TB, PB. Decimals are applied only from KB up (`532 B`, `1.4 MB`). Negative input formats the absolute value with a leading minus sign. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Bytes | `int64` | The byte count to format. |
| Decimals | `int32` | The number of decimal places to show from KB up. Clamped to the [0, 3] range. |

**Returns:** The formatted size text.

## Format Duration
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatDuration(float Seconds, bool bIncludeSeconds = true);
```

Formats a duration in seconds as d/h/m/s units from the largest nonzero unit down, with two-digit padding after the first (`1h 03m 05s`, `2d 04h`, `45s`). With `bIncludeSeconds` false the seconds unit is dropped and sub-minute durations return `0m`. Negative input gets a leading minus sign; non-finite input returns `0s`. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Seconds | `float` | The duration in seconds. |
| bIncludeSeconds | `bool` | Whether to include the seconds unit. |

**Returns:** The formatted duration text.

## Format Relative Time
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Formatting`

```cpp
static FText FormatRelativeTime(const FDateTime& Timestamp);
```

Formats a timestamp relative to the current local time: `just now` (under a minute), `N minute(s)/hour(s)/day(s) ago`, or `in N ...` for future timestamps. Uses local time, pairing with Get Save Slot Timestamp. Not pure because it reads the current clock each call. Output is English-only.

| Parameter | Type | Description |
|-----------|------|-------------|
| Timestamp | `const FDateTime&` | The local timestamp to describe. |

**Returns:** The formatted relative time text.

## Get Int Array Sum
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static int64 GetIntArraySum(const TArray<int32>& Values);
```

Returns the sum of an integer array as a 64-bit integer, so large arrays cannot overflow int32.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to sum. |

**Returns:** The sum of the values, or 0 if the array is empty.

## Get Int Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayAverage(const TArray<int32>& Values);
```

Returns the arithmetic mean of an integer array.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to average. |

**Returns:** The average of the values, or 0 if the array is empty.

## Get Int Array Median
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayMedian(const TArray<int32>& Values);
```

Returns the median of an integer array (computed on a sorted copy; the input is not modified). For an even count, returns the average of the two middle values.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to take the median of. |

**Returns:** The median of the values, or 0 if the array is empty.

## Get Int Array Standard Deviation
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetIntArrayStandardDeviation(const TArray<int32>& Values);
```

Returns the population standard deviation of an integer array (divides by N, not N-1).

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<int32>&` | The values to measure. |

**Returns:** The population standard deviation, or 0 if the array is empty.

## Get Float Array Sum
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArraySum(const TArray<float>& Values);
```

Returns the sum of a float array. Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to sum. |

**Returns:** The sum of the values, or 0 if the array is empty.

## Get Float Array Average
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayAverage(const TArray<float>& Values);
```

Returns the arithmetic mean of a float array. Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to average. |

**Returns:** The average of the values, or 0 if the array is empty.

## Get Float Array Median
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayMedian(const TArray<float>& Values);
```

Returns the median of a float array (computed on a sorted copy; the input is not modified). For an even count, returns the average of the two middle values.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to take the median of. |

**Returns:** The median of the values, or 0 if the array is empty.

## Get Float Array Standard Deviation
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Array`

```cpp
static float GetFloatArrayStandardDeviation(const TArray<float>& Values);
```

Returns the population standard deviation of a float array (divides by N, not N-1). Accumulates in double internally for precision.

| Parameter | Type | Description |
|-----------|------|-------------|
| Values | `const TArray<float>&` | The values to measure. |

**Returns:** The population standard deviation, or 0 if the array is empty.

## Get Random Index From Weights
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static int32 GetRandomIndexFromWeights(const TArray<float>& Weights);
```

Returns a random index into the Weights array, where each index's probability is proportional to its weight. Useful for loot tables and weighted spawning. Negative weights are treated as zero.

| Parameter | Type | Description |
|-----------|------|-------------|
| Weights | `const TArray<float>&` | The per-index weights. |

**Returns:** The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.

## Get Random Index From Weights (Stream)
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Math|Random`

```cpp
static int32 GetRandomIndexFromWeightsFromStream(UPARAM(ref) FRandomStream& Stream, const TArray<float>& Weights);
```

Deterministic version of Get Random Index From Weights that draws from (and advances) the provided random stream.

| Parameter | Type | Description |
|-----------|------|-------------|
| Stream | `FRandomStream&` | The random stream to draw from. |
| Weights | `const TArray<float>&` | The per-index weights. Negative weights are treated as zero. |

**Returns:** The selected index, or INDEX_NONE (-1) if the array is empty or all weights are zero.

---

## EDirectiveUtilEaseType

Easing curves not provided by the engine's built-in Ease node (EEasingFunc): the classic Penner Back, Elastic and Bounce curves.

| Value | Display Name | Description |
|-------|--------------|-------------|
| BackIn | Back In | Overshoots slightly at the start before easing in. |
| BackOut | Back Out | Overshoots slightly past the end before settling. |
| BackInOut | Back In Out | Overshoots at both the start and the end. |
| ElasticIn | Elastic In | Oscillates with increasing amplitude before easing in. |
| ElasticOut | Elastic Out | Oscillates with decreasing amplitude after the end. |
| ElasticInOut | Elastic In Out | Oscillates at both the start and the end. |
| BounceIn | Bounce In | Bounces with increasing energy before easing in. |
| BounceOut | Bounce Out | Bounces with decreasing energy after the end. |
| BounceInOut | Bounce In Out | Bounces at both the start and the end. |
