// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilFileSystemFunctionLibrary.generated.h"

/**
 * File utilities for Blueprints: reading and writing text or binary files, atomic replacement,
 * and querying size and timestamps. Use the engine's Paths and Blueprint File Utils nodes for
 * path parsing and file management.
 *
 * All path parameters accept absolute paths, or paths relative to the project Saved directory.
 * Reads and ordinary writes go through IFileManager. Atomic writes resolve the active platform
 * layer's physical write path before replacement and therefore require a filesystem-backed path.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilFileSystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Reads an entire text file into a single string. Loads the whole file into memory;
	 * avoid very large files on the game thread. Unreal's automatic encoding detection accepts
	 * supported UTF-8 and UTF-16 forms. Files of 2 GB or more fail rather than truncate.
	 *
	 * @param Path The file to read.
	 * @param OutContents Receives the file contents, or an empty string on failure.
	 * @return `true` when the file was read.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool ReadTextFile(const FString& Path, FString& OutContents);

	/**
	 * Reads a text file line by line. Splits on CRLF, LF, and CR sequences.
	 *
	 * @param Path The file to read.
	 * @param OutLines Receives one entry per line, without line terminators, or an empty array on failure. A trailing newline does not produce a trailing empty entry.
	 * @param bIncludeEmptyLines Whether blank lines appear in the output.
	 * @return `true` when the file was read.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool ReadTextFileLines(const FString& Path, TArray<FString>& OutLines, const bool bIncludeEmptyLines = true);

	/**
	 * Writes text to a file in UTF-8 without a byte-order mark, replacing any existing content
	 * unless overwriting is refused.
	 *
	 * @param Path The file to write. Missing parent directories are created only when requested.
	 * @param Contents The text to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the call fails if the file already exists.
	 * @return `true` when the file was written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/**
	 * Appends text to a file, creating the file when missing. No separator is inserted between
	 * the existing contents and the appended text. Existing supported text encodings are read
	 * first and the complete result is atomically rewritten as UTF-8 without a byte-order mark.
	 *
	 * @param Path The file to append to.
	 * @param Contents The text to append.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @return `true` when the text was appended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool AppendTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true);

	/**
	 * Reads an entire binary file into memory. Avoid very large files on the game thread.
	 *
	 * @param Path The file to read.
	 * @param OutBytes Receives the file bytes, or an empty array on failure.
	 * @return `true` when the file was read.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool ReadBinaryFile(const FString& Path, TArray<uint8>& OutBytes);

	/**
	 * Writes bytes to a file, replacing any existing content unless overwriting is refused.
	 *
	 * @param Path The file to write. Missing parent directories are created only when requested.
	 * @param Bytes The bytes to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the call fails if the file already exists.
	 * @return `true` when the file was written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteBinaryFile(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/** Writes a temporary text file and atomically installs it after the write succeeds. No-overwrite mode is race-safe. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteTextFileAtomic(const FString& Path, const FString& Contents,
		const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/** Writes a temporary binary file and atomically installs it after the write succeeds. No-overwrite mode is race-safe. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteBinaryFileAtomic(const FString& Path, const TArray<uint8>& Bytes,
		const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/**
	 * Returns the size of a file in bytes.
	 *
	 * @param Path The file to measure.
	 * @return The size in bytes, or -1 when the file does not exist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static int64 GetFileSize(const FString& Path);

	/**
	 * Returns the last-modified timestamp of a file or directory as local time.
	 *
	 * @param Path The path to query.
	 * @param OutTimestamp Receives the last-modified time converted from UTC using the timezone rules for that instant, or a default value on failure.
	 * @return `true` when the path exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool GetFileTimeStamp(const FString& Path, FDateTime& OutTimestamp);
};
