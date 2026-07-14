// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilSaveGameFunctionLibrary.generated.h"

class USaveGame;

/**
 * UDirectiveUtilSaveGameFunctionLibrary
 * Save-slot utilities that fill the gaps left by UGameplayStatics: enumerating slots, reading slot
 * timestamps, and serializing a save object to/from an in-memory byte array. This is slot/IO QoL only,
 * not a save framework: use the engine's SaveGameToSlot/LoadGameFromSlot for the actual slot I/O.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilSaveGameFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns the names of all existing save slots in the project's default save directory.
	 * @note This enumerates the engine's default file-based save directory (Saved/SaveGames); it does not
	 * cover platform-specific save systems (e.g. console storage).
	 * @returns The save slot names (without extension).
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static TArray<FString> GetAllSaveSlotNames();

	/**
	 * Returns the last-modified timestamp of a save slot, if it exists.
	 * @param SlotName - The save slot name.
	 * @param OutTimestamp - [out] The slot's last-modified time (local), or a default time if it does not exist.
	 * Converted from the file system's UTC timestamp to local time.
	 * @returns True if the slot exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static bool GetSaveSlotTimestamp(const FString& SlotName, FDateTime& OutTimestamp);

	/**
	 * Serializes a save game object to an in-memory byte array (instead of a slot file).
	 * Useful for custom storage, networking, or cloud saves.
	 * @param SaveGameObject - The save game object to serialize.
	 * @param OutBytes - [out] The serialized bytes.
	 * @returns True if serialization succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static bool SaveGameToBytes(USaveGame* SaveGameObject, TArray<uint8>& OutBytes);

	/**
	 * Deserializes a save game object from an in-memory byte array produced by Save Game To Bytes.
	 * @param SaveData - The serialized bytes.
	 * @returns The deserialized save game object, or null on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static USaveGame* LoadGameFromBytes(const TArray<uint8>& SaveData);

	/**
	 * Checks whether a save slot exists. Goes through the engine's save game system, so unlike
	 * enumeration it also works on platform save backends.
	 * @param SlotName - The save slot name.
	 * @param UserIndex - The platform user index the save belongs to.
	 * @returns True if the slot exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static bool DoesSaveSlotExist(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Deletes a save slot. Goes through the engine's save game system, so unlike enumeration it
	 * also works on platform save backends.
	 * @param SlotName - The save slot name.
	 * @param UserIndex - The platform user index the save belongs to.
	 * @returns True if a save was actually deleted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static bool DeleteSaveSlot(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Renames a save slot by copying its data to the new name and then deleting the original.
	 * Fails without mutating anything unless both names are valid, the names differ, the old slot
	 * exists, and the new slot does not. On failure the original slot is never lost. Goes through
	 * the engine's save game system, so unlike enumeration it also works on platform save backends.
	 * @param OldSlotName - The existing save slot name.
	 * @param NewSlotName - The new save slot name.
	 * @param UserIndex - The platform user index the save belongs to.
	 * @returns True if the slot was renamed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|SaveGame")
	static bool RenameSaveSlot(const FString& OldSlotName, const FString& NewSlotName, int32 UserIndex = 0);
};
