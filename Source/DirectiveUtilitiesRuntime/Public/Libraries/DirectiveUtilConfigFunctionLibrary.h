// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilConfigFunctionLibrary.generated.h"

/**
 * Configuration file (.ini) utilities for Blueprints: reading values with defaults, writing
 * and removing keys, clearing sections, and listing sections and keys. The engine keeps
 * GConfig behind C++.
 *
 * All path parameters accept absolute paths, or paths relative to the project Saved directory.
 * Writes atomically replace the file before returning, so a write followed by an external
 * read sees the new value. Every read loads the file fresh from disk, so external edits are
 * visible immediately and each call pays a full parse.
 *
 * Reads return stored text as written. Engine macros such as %GAME% are not expanded. When a
 * key holds several entries, scalar reads return the last one. Reads treat a malformed or
 * unreadable file as missing, so they return their default or `false`.
 *
 * A write regenerates the whole file from the parsed model: comments and blank lines in a
 * hand-authored .ini are not preserved, but empty sections are. Values that the .ini parser
 * would change, such as text with leading or trailing tabs, braces, or line breaks, are written
 * in quotes so they read back unchanged. Clearing the last section leaves an empty file rather
 * than deleting it. Writes always produce UTF-8 without a byte-order mark. Writes refuse
 * malformed or unreadable existing files, including lines that the engine parser would join
 * with the next line or change by dropping unquoted braces. Section and key names must not
 * contain configuration syntax such as brackets, braces, or double quotes, line breaks,
 * surrounding whitespace, or names too long for Unreal's FName representation. Key names also
 * cannot start with an engine config command character: `+`, `-`, `.`, `!`, `@`, `*`, or `^`.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilConfigFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Reads a string value from a configuration file.
	 *
	 * @param FilePath The configuration file to read. Created on write operations, never on reads.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param DefaultValue Returned when the file, section, or key is missing.
	 * @return The stored value, or `DefaultValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static FString ReadConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& DefaultValue);

	/**
	 * Reads an int32 value from a configuration file.
	 *
	 * @param FilePath The configuration file to read.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param DefaultValue Returned when the file, section, or key is missing.
	 * @return The stored value, or `DefaultValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static int32 ReadConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 DefaultValue);

	/**
	 * Reads an int64 value from a configuration file.
	 *
	 * @param FilePath The configuration file to read.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param DefaultValue Returned when the file, section, or key is missing.
	 * @return The stored value, or `DefaultValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static int64 ReadConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 DefaultValue);

	/**
	 * Reads a float value from a configuration file.
	 *
	 * @param FilePath The configuration file to read.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param DefaultValue Returned when the file, section, or key is missing.
	 * @return The stored value, or `DefaultValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static float ReadConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float DefaultValue);

	/**
	 * Reads a Boolean value from a configuration file.
	 *
	 * @param FilePath The configuration file to read.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param DefaultValue Returned when the file, section, or key is missing.
	 * @return The stored value, or `DefaultValue`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool ReadConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool DefaultValue);

	/**
	 * Reads a string array value from a configuration file.
	 *
	 * @param FilePath The configuration file to read.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to read.
	 * @param OutValues Receives the stored entries, or an empty array when the file, section, or key is missing.
	 * @return `true` when the key was found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool ReadConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, TArray<FString>& OutValues);

	/**
	 * Writes a string value to a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write. Missing parent directories are created.
	 * @param SectionName The section that receives the key. Created when missing.
	 * @param KeyName The key to write.
	 * @param Value The value to store.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigString(const FString& FilePath, const FString& SectionName, const FString& KeyName, const FString& Value);

	/**
	 * Writes an int32 value to a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write.
	 * @param SectionName The section that receives the key.
	 * @param KeyName The key to write.
	 * @param Value The value to store.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigInt(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int32 Value);

	/**
	 * Writes an int64 value to a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write.
	 * @param SectionName The section that receives the key.
	 * @param KeyName The key to write.
	 * @param Value The value to store.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigInt64(const FString& FilePath, const FString& SectionName, const FString& KeyName, const int64 Value);

	/**
	 * Writes a float value to a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write.
	 * @param SectionName The section that receives the key.
	 * @param KeyName The key to write.
	 * @param Value The value to store.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigFloat(const FString& FilePath, const FString& SectionName, const FString& KeyName, const float Value);

	/**
	 * Writes a Boolean value to a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write.
	 * @param SectionName The section that receives the key.
	 * @param KeyName The key to write.
	 * @param Value The value to store.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigBool(const FString& FilePath, const FString& SectionName, const FString& KeyName, const bool Value);

	/**
	 * Replaces a string array value in a configuration file and flushes it to disk.
	 *
	 * @param FilePath The configuration file to write.
	 * @param SectionName The section that receives the key.
	 * @param KeyName The key to write.
	 * @param Values The entries to store. An empty array removes every entry for the key and
	 *        leaves the section in place, creating it empty when it was missing.
	 * @return `true` when the file was written to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigStringArray(const FString& FilePath, const FString& SectionName, const FString& KeyName, const TArray<FString>& Values);

	/**
	 * Removes one key from a configuration file and flushes the change to disk. Removing the
	 * last key of a section leaves the empty section in the file.
	 *
	 * @param FilePath The configuration file to edit.
	 * @param SectionName The section that holds the key.
	 * @param KeyName The key to remove.
	 * @return `true` when the key existed and was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool RemoveConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName);

	/**
	 * Removes every key of one section from a configuration file and flushes the change to
	 * disk. The empty section itself is also dropped.
	 *
	 * @param FilePath The configuration file to edit.
	 * @param SectionName The section to clear.
	 * @return `true` when the section existed and was cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool ClearConfigSection(const FString& FilePath, const FString& SectionName);

	/**
	 * Checks whether a section exists in a configuration file.
	 *
	 * @param FilePath The configuration file to inspect.
	 * @param SectionName The section to look for.
	 * @return `true` when the section exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool HasConfigSection(const FString& FilePath, const FString& SectionName);

	/**
	 * Checks whether a key exists in a configuration section.
	 *
	 * @param FilePath The configuration file to inspect.
	 * @param SectionName The section to inspect.
	 * @param KeyName The key to find.
	 * @return `true` when the file, section, and key exist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool HasConfigKey(const FString& FilePath, const FString& SectionName, const FString& KeyName);

	/** Reads a Vector2D value, returning DefaultValue when it is missing or malformed. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static FVector2D ReadConfigVector2D(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FVector2D& DefaultValue);

	/** Reads a Vector value, returning DefaultValue when it is missing or malformed. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static FVector ReadConfigVector(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FVector& DefaultValue);

	/** Reads a Rotator value, returning DefaultValue when it is missing or malformed. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static FRotator ReadConfigRotator(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FRotator& DefaultValue);

	/** Reads a Color value, returning DefaultValue when it is missing or malformed. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static FColor ReadConfigColor(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FColor& DefaultValue);

	/** Writes a finite Vector2D value and flushes it to disk. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigVector2D(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FVector2D& Value);

	/** Writes a finite Vector value and flushes it to disk. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigVector(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FVector& Value);

	/** Writes a finite Rotator value and flushes it to disk. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigRotator(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FRotator& Value);

	/** Writes a Color value and flushes it to disk. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool WriteConfigColor(const FString& FilePath, const FString& SectionName,
		const FString& KeyName, const FColor& Value);

	/**
	 * Returns every section name in a configuration file, sorted case-insensitively.
	 *
	 * @param FilePath The configuration file to inspect.
	 * @param OutSectionNames Receives the section names without brackets, or an empty array when the file does not exist.
	 * @return `true` when the file exists and parses.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool GetConfigSectionNames(const FString& FilePath, TArray<FString>& OutSectionNames);

	/**
	 * Returns every unique key name inside one section, sorted case-insensitively.
	 *
	 * @param FilePath The configuration file to inspect.
	 * @param SectionName The section to list.
	 * @param OutKeyNames Receives the key names, or an empty array when the file or section does not exist.
	 * @return `true` when the file parses and the section exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Config")
	static bool GetConfigKeysInSection(const FString& FilePath, const FString& SectionName, TArray<FString>& OutKeyNames);
};
