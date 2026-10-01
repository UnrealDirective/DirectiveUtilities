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
 * Reads, appends, and overwriting writes go through IFileManager. Atomic writes and
 * no-overwrite writes resolve the active platform layer's physical write path before
 * replacement and therefore require a filesystem-backed path.
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
	 * Reads a text file line by line. Each CRLF, LF, or CR sequence ends one line. The text after
	 * the last terminator becomes the final line only when it is not empty, so an empty file has no
	 * lines, "a\n" has one line, and a file that holds only "\n" has one empty line.
	 *
	 * @param Path The file to read.
	 * @param OutLines Receives one entry per line, without line terminators, or an empty array on failure.
	 * @param bIncludeEmptyLines When `false`, lines with no characters are dropped. Lines that hold only spaces or tabs are kept.
	 * @return `true` when the file was read.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool ReadTextFileLines(const FString& Path, TArray<FString>& OutLines, const bool bIncludeEmptyLines = true);

	/**
	 * Writes text to a file in UTF-8 without a byte-order mark, replacing any existing content
	 * unless overwriting is refused. When `bAllowOverwrite` is `false`, the write uses the
	 * temporary-file path of WriteTextFileAtomic so it cannot replace a file that another writer
	 * creates first.
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
	 * Appends text to the end of a file, creating the file when missing. No separator is inserted
	 * and the existing bytes are not rewritten. The text is encoded as UTF-16 when the file starts
	 * with a UTF-16 byte-order mark, and as UTF-8 without a byte-order mark otherwise. A file in
	 * another encoding, such as Latin-1, keeps its bytes but receives UTF-8 text. The append is not
	 * atomic, so a failed call can leave part of the text in the file.
	 *
	 * @param Path The file to append to.
	 * @param Contents The text to append.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @return `true` when the text was appended. A UTF-32 file, a UTF-16 file with an odd byte count, or a file whose first bytes cannot be read fails without changing the file.
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
	 * Writes bytes to a file, replacing any existing content unless overwriting is refused. When
	 * `bAllowOverwrite` is `false`, the write uses the temporary-file path of WriteBinaryFileAtomic.
	 *
	 * @param Path The file to write. Missing parent directories are created only when requested.
	 * @param Bytes The bytes to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the call fails if the file already exists.
	 * @return `true` when the file was written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteBinaryFile(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/**
	 * Writes text in UTF-8 without a byte-order mark to a temporary sibling file, then moves it over
	 * the destination so readers see either the old file or the complete new file. When
	 * `bAllowOverwrite` is `false`, the call fails if the destination exists or another writer
	 * creates it first. On Mac and Linux filesystems without hard links, such as FAT, exFAT, and
	 * some network shares, that mode can show an empty destination until the move completes,
	 * and a writer that replaces the destination during that window is overwritten.
	 *
	 * The replacement is a new file. A symbolic link at the destination is replaced rather than
	 * followed, and the file takes default permissions instead of the old file's permissions. A
	 * process that exits during the write can leave a `<file>.tmp-<32 hex digits>` sibling behind.
	 * The destination must be on a filesystem-backed path.
	 *
	 * @param Path The file to write.
	 * @param Contents The text to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the call fails if the file already exists.
	 * @return `true` when the file was installed. On failure the destination is unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|FileSystem")
	static bool WriteTextFileAtomic(const FString& Path, const FString& Contents,
		const bool bCreateDirectories = true, const bool bAllowOverwrite = true);

	/**
	 * Writes bytes to a temporary sibling file, then moves it over the destination so readers see
	 * either the old file or the complete new file. Overwrite, symbolic link, permission, and
	 * temporary file behavior match `WriteTextFileAtomic`.
	 *
	 * @param Path The file to write.
	 * @param Bytes The bytes to write.
	 * @param bCreateDirectories When `true`, missing parent directories are created.
	 * @param bAllowOverwrite When `false`, the call fails if the file already exists.
	 * @return `true` when the file was installed. On failure the destination is unchanged.
	 */
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
