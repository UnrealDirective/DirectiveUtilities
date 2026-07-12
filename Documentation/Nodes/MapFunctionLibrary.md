# Map Function Library

> A collection of map (TMap) utility functions that improve the usability of maps in Blueprints.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilMapFunctionLibrary.h`

---

## Find Or Add
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static void Map_FindOrAdd(const TMap<int32, int32>& TargetMap, const int32& Key, int32& Value);
```

Finds the value associated with Key, adding a new default-constructed entry if the key is not present. This is a wildcard map node; the key and value types resolve to those of the connected map.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map to search or add to. |
| Key | `const int32&` | The key to look up. |
| Value | `int32&` | [out] A copy of the existing or newly added value. |

**Returns:** `Value` (out): a copy of the existing or newly added value.

## Clear Values
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static void Map_ClearValues(const TMap<int32, int32>& TargetMap);
```

Resets every value in the map to its default while preserving all keys. This is a wildcard map node; the key and value types resolve to those of the connected map.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map whose values will be reset. |

## Get Keys By Value
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static void Map_GetKeysByValue(const TMap<int32, int32>& TargetMap, const int32& Value, TArray<int32>& Keys);
```

Gathers every key whose value is identical to Value, in map order. This is a wildcard map node; the key and value types resolve to those of the connected map.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map to search. |
| Value | `const int32&` | The value to look for. |
| Keys | `TArray<int32>&` | [out] Every key associated with Value. |

**Returns:** `Keys` (out): every key associated with Value, in map order.

## Has Value
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static bool Map_HasValue(const TMap<int32, int32>& TargetMap, const int32& Value);
```

Checks whether any value in the map is identical to Value. This is a wildcard map node; the key and value types resolve to those of the connected map.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map to search. |
| Value | `const int32&` | The value to look for. |

**Returns:** `bool`: true if at least one entry holds Value.

## Remove Keys
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static int32 Map_RemoveKeys(const TMap<int32, int32>& TargetMap, const TArray<int32>& Keys);
```

Removes every key in Keys from the map. Keys that are not present are ignored. This is a wildcard map node; the key and value types resolve to those of the connected map.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map to remove from. |
| Keys | `const TArray<int32>&` | The keys to remove. |

**Returns:** `int32`: the number of entries that were actually removed.

## Append
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Map`

```cpp
static void Map_Append(const TMap<int32, int32>& TargetMap, const TMap<int32, int32>& SourceMap, bool bOverwriteExisting = true);
```

Copies every pair from SourceMap into TargetMap. Both maps must share the same key and value types. This is a wildcard map node; the key and value types resolve to those of the connected maps.

| Parameter | Type | Description |
|-----------|------|-------------|
| TargetMap | `const TMap<int32, int32>&` | The map to copy into. |
| SourceMap | `const TMap<int32, int32>&` | The map to copy from. |
| bOverwriteExisting | `bool` | If true, keys already present in TargetMap are overwritten with SourceMap's values. Defaults to true. |
