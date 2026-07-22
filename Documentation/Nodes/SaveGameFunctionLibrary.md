# Save Game Function Library

> Save-slot utilities that fill the gaps left by UGameplayStatics: enumerating slots, reading slot timestamps, and serializing a save object to/from an in-memory byte array.

Slot operations accept flat file names. Path separators and relative paths are rejected for consistent behavior across platform save backends.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilSaveGameFunctionLibrary.h`

---

## Get All Save Slot Names
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static TArray<FString> GetAllSaveSlotNames();
```

Returns the names of all existing save slots in the project's default save directory. This enumerates the engine's default file-based save directory (Saved/SaveGames) and does not cover platform-specific save systems such as console storage.

**Returns:** The save slot names (without extension).

## Get Save Slot Timestamp
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static bool GetSaveSlotTimestamp(const FString& SlotName, FDateTime& OutTimestamp);
```

Returns the last-modified timestamp of a save slot, if it exists.

| Parameter | Type | Description |
|-----------|------|-------------|
| SlotName | `const FString&` | The save slot name. |
| OutTimestamp | `FDateTime&` | [out] The slot's last-modified time (local), or a default time if it does not exist. |

**Returns:** True if the slot exists.

## Save Game To Bytes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static bool SaveGameToBytes(USaveGame* SaveGameObject, TArray<uint8>& OutBytes);
```

Serializes a save game object to an in-memory byte array instead of a slot file. Useful for custom storage, networking, or cloud saves.

| Parameter | Type | Description |
|-----------|------|-------------|
| SaveGameObject | `USaveGame*` | The save game object to serialize. |
| OutBytes | `TArray<uint8>&` | [out] The serialized bytes. |

**Returns:** True if serialization succeeded.

## Load Game From Bytes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static USaveGame* LoadGameFromBytes(const TArray<uint8>& SaveData);
```

Deserializes a save game object from an in-memory byte array produced by Save Game To Bytes.

| Parameter | Type | Description |
|-----------|------|-------------|
| SaveData | `const TArray<uint8>&` | The serialized bytes. |

**Returns:** The deserialized save game object, or null on failure.

## Does Save Slot Exist
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static bool DoesSaveSlotExist(const FString& SlotName, int32 UserIndex = 0);
```

Checks whether a save slot exists. Goes through the engine's save game system, so unlike enumeration it also works on platform save backends.

| Parameter | Type | Description |
|-----------|------|-------------|
| SlotName | `const FString&` | The save slot name. |
| UserIndex | `int32` | The platform user index the save belongs to. |

**Returns:** True if the slot exists.

## Delete Save Slot
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static bool DeleteSaveSlot(const FString& SlotName, int32 UserIndex = 0);
```

Deletes a save slot. Goes through the engine's save game system, so unlike enumeration it also works on platform save backends.

| Parameter | Type | Description |
|-----------|------|-------------|
| SlotName | `const FString&` | The save slot name. |
| UserIndex | `int32` | The platform user index the save belongs to. |

**Returns:** True if a save was actually deleted.

## Rename Save Slot
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|SaveGame`

```cpp
static bool RenameSaveSlot(const FString& OldSlotName, const FString& NewSlotName, int32 UserIndex = 0);
```

Renames a save slot by copying its data to the new name and then deleting the original. Fails without mutating anything unless both names are valid, the names differ, the old slot exists, and the new slot does not. On failure the original slot is never lost. Goes through the engine's save game system, so unlike enumeration it also works on platform save backends.

| Parameter | Type | Description |
|-----------|------|-------------|
| OldSlotName | `const FString&` | The existing save slot name. |
| NewSlotName | `const FString&` | The new save slot name. |
| UserIndex | `int32` | The platform user index the save belongs to. |

**Returns:** True if the slot was renamed.
