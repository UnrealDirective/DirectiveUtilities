# Array Function Library

> A collection of array utility functions that improve the usability of arrays in Blueprints.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilArrayFunctionLibrary.h`

---

## Next Index
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static int32 Array_NextIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);
```

Returns the next index in the array. If the next index is greater than the last array index and `bLoop` is enabled, the index loops back to the start; otherwise the last index is returned. Returns `INDEX_NONE` for an empty array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to get the next index for. |
| Index | `int32` | The current index. |
| bLoop | `bool` | If true, the index loops back to the beginning of the array when the next index is greater than the last array index. Otherwise, the last index is returned. |

**Returns:** The next index in the array.

## Previous Index
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static int32 Array_PreviousIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);
```

Returns the previous index in the array. If the previous index is less than 0 and `bLoop` is enabled, the index loops back to the end; otherwise 0 is returned. Returns `INDEX_NONE` for an empty array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to get the previous index for. |
| Index | `int32` | The current index. |
| bLoop | `bool` | If the next index is greater than the last array index and bLoop is enabled, the index loops back to the start of the array. Otherwise, the last index is returned. |

**Returns:** The previous index in the array.

## Remove Duplicates
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_RemoveDuplicates(const TArray<int32>& TargetArray);
```

Removes duplicate elements from the array in-place.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to remove duplicates from. |

## Get Valid First Array Item (Copy)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetValidFirstItemCopy(const TArray<int32>& TargetArray, int32& OutItem);
```

Returns a copy of the first element of the array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutItem | `Wildcard` | [out] A copy of the first element, or the default value if the array is empty. |

**Returns:** True if the array contained a valid first element.

## Get Valid Last Array Item (Copy)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetValidLastItemCopy(const TArray<int32>& TargetArray, int32& OutItem);
```

Returns a copy of the last element of the array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutItem | `Wildcard` | [out] A copy of the last element, or the default value if the array is empty. |

**Returns:** True if the array contained a valid last element.

## Get Valid Array Item From Index (Copy)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetValidItemFromIndexCopy(const TArray<int32>& TargetArray, const int32 Index, int32& OutItem);
```

Returns a copy of the element at the given index, if the index is valid.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| Index | `int32` | The index to read. |
| OutItem | `Wildcard` | [out] A copy of the element, or the default value if the index is invalid. |

**Returns:** True if the index was valid.

## Get Random Valid Array Item
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetRandomItem(const TArray<int32>& TargetArray, int32& OutItem, int32& OutIndex);
```

Returns a copy of a random element from the array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutItem | `Wildcard` | [out] A copy of the randomly selected element, or the default value if the array is empty. |
| OutIndex | `int32` | [out] The index of the selected element, or INDEX_NONE if the array is empty. |

**Returns:** True if a valid element was selected.

## Last Value
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_LastValue(const TArray<int32>& TargetArray, int32& OutItem);
```

Returns a copy of the last element of the array without removing it.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutItem | `Wildcard` | [out] A copy of the last element, or the default value if the array is empty. |

**Returns:** OutItem: a copy of the last element, or the default value if the array is empty.

## Pop
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_Pop(const TArray<int32>& TargetArray, int32& OutItem);
```

Removes the last element of the array and returns a copy of it.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to pop from. |
| OutItem | `Wildcard` | [out] A copy of the removed element, or the default value if the array is empty. |

**Returns:** True if an element was removed.

## Pop First
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_PopFirst(const TArray<int32>& TargetArray, int32& OutItem);
```

Removes the first element of the array and returns a copy of it.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to pop from. |
| OutItem | `Wildcard` | [out] A copy of the removed element, or the default value if the array is empty. |

**Returns:** True if an element was removed.

## Remove At Swap
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_RemoveAtSwap(const TArray<int32>& TargetArray, const int32 Index);
```

Removes the element at the given index by swapping it with the last element (does not preserve order). This is O(1) but changes the position of the previously-last element.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to remove from. |
| Index | `int32` | The index to remove. |

**Returns:** True if the index was valid and an element was removed.

## Slice
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_Slice(const TArray<int32>& TargetArray, const int32 StartIndex, const int32 Count, TArray<int32>& OutArray);
```

Returns a copy of a contiguous range of the array. The range is clamped to the array bounds.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to slice. |
| StartIndex | `int32` | The index to start copying from (clamped to [0, Length]). |
| Count | `int32` | The number of elements to copy. Values <= 0 produce an empty array. |
| OutArray | `Wildcard Array` | [out] The sliced copy. |

**Returns:** OutArray: the sliced copy.

## Rotate
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_Rotate(const TArray<int32>& TargetArray, const int32 Shift);
```

Cyclically rotates the elements of the array in place.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to rotate. |
| Shift | `int32` | The number of positions to rotate. Positive rotates toward the end; negative toward the start. |

## Get Distinct (Copy)
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_GetDistinct(const TArray<int32>& TargetArray, TArray<int32>& OutArray);
```

Returns a copy of the array with duplicates removed, keeping the first occurrence and preserving order. Unlike Remove Duplicates, this does not modify the input array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutArray | `Wildcard Array` | [out] The de-duplicated copy. |

**Returns:** OutArray: the de-duplicated copy.

## Count Occurrences
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static int32 Array_CountOccurrences(const TArray<int32>& TargetArray, const int32& ItemToCount);
```

Counts how many times an item appears in the array.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to search. |
| ItemToCount | `Wildcard` | The item to count. |

**Returns:** The number of occurrences.

## Get Most Common
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetMostCommon(const TArray<int32>& TargetArray, int32& OutItem, int32& OutCount);
```

Returns the most frequently occurring element of the array (ties resolve to the earliest such element).

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read from. |
| OutItem | `Wildcard` | [out] A copy of the most common element, or the default value if the array is empty. |
| OutCount | `int32` | [out] The number of times the most common element occurs. |

**Returns:** True if the array was non-empty.

## Sample Array
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_Sample(
    const TArray<int32>& TargetArray,
    int32 Count,
    bool bWithReplacement,
    TArray<int32>& OutArray);
```

Returns random elements from a wildcard array. Without replacement, `Count` is limited to the source length and each source index can be selected once. With replacement, the output contains `Count` elements and values may repeat. An empty source or a non-positive count returns an empty output.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to sample. |
| Count | `int32` | The requested output size. |
| bWithReplacement | `bool` | Allows the same source index to be selected more than once. |
| OutArray | `Wildcard Array` | The sampled values. |

## Sample Array From Stream
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void Array_SampleFromStream(
    const TArray<int32>& TargetArray,
    int32 Count,
    bool bWithReplacement,
    FRandomStream& RandomStream,
    TArray<int32>& OutArray);
```

Uses the same rules as `Sample Array`, but reads from a random stream. Streams created with the same seed and state produce the same sample.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to sample. |
| Count | `int32` | The requested output size. |
| bWithReplacement | `bool` | Allows the same source index to be selected more than once. |
| RandomStream | `FRandomStream` | The stream used for selection. Its state advances. |
| OutArray | `Wildcard Array` | The sampled values. |

## Get Array Page
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static bool Array_GetPage(
    const TArray<int32>& TargetArray,
    int32 PageIndex,
    int32 PageSize,
    TArray<int32>& OutArray,
    int32& OutPageCount);
```

Returns one zero-based page from a wildcard array. A negative page index, non-positive page size, or page index outside the available pages returns false and an empty output. An empty source has zero pages.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `Wildcard Array` | The array to read. |
| PageIndex | `int32` | The zero-based page index. |
| PageSize | `int32` | The maximum elements per page. |
| OutArray | `Wildcard Array` | The requested page. |
| OutPageCount | `int32` | The total page count. |

**Returns:** True when the requested page exists.

## Natural Sort String Array
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void NaturalSortStringArray(TArray<FString>& TargetArray, bool bDescending = false);
```

Sorts strings in place and compares embedded numbers by value. For example, `Item2` sorts before `Item10`. Equal values keep their original order.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `TArray<FString>` | The array to sort. |
| bDescending | `bool` | Reverses the sort direction. |

## Natural Sort Name Array
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Array`

```cpp
static void NaturalSortNameArray(TArray<FName>& TargetArray, bool bDescending = false);
```

Sorts names in place using the same ordering as `Natural Sort String Array`.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetArray | `TArray<FName>` | The array to sort. |
| bDescending | `bool` | Reverses the sort direction. |
