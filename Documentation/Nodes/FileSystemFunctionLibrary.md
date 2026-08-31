# File System Function Library

> Runtime text and binary file I/O, atomic replacement, size, and timestamps.

All path parameters accept absolute paths, or paths relative to the project `Saved` directory. Relative writes therefore remain valid in packaged builds. Empty paths and paths containing embedded null characters are rejected. Use Unreal's `Paths` nodes and Blueprint File Utils plugin for path parsing, existence checks, directory operations, copy, move, delete, and enumeration.

The ordinary functions load or write a complete file on the calling thread. Use the async nodes for frame-sensitive Blueprint code.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilFileSystemFunctionLibrary.h`

---

## Read Text File

```cpp
static bool ReadTextFile(const FString& Path, FString& OutContents);
```

Reads a complete text file with Unreal's automatic encoding detection, including UTF-8 and UTF-16 byte-order marks. Files of 2 GB or more fail rather than truncate. `OutContents` is empty on failure.

## Read Text File Lines

```cpp
static bool ReadTextFileLines(const FString& Path, TArray<FString>& OutLines, const bool bIncludeEmptyLines = true);
```

Splits CRLF, LF, and CR line endings. A final line ending does not add an empty entry.

## Write Text File / Write Binary File

```cpp
static bool WriteTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
static bool WriteBinaryFile(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
```

Writes the complete value to the destination. Text uses UTF-8 without a byte-order mark. Missing parent directories are created when requested. Setting `bAllowOverwrite` false refuses an existing destination.

## Write Text File Atomic / Write Binary File Atomic

```cpp
static bool WriteTextFileAtomic(const FString& Path, const FString& Contents, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
static bool WriteBinaryFileAtomic(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
```

Writes a temporary sibling file, then replaces the destination. A failed temporary write leaves the previous destination unchanged and removes the temporary file. Setting `bAllowOverwrite` false performs a race-safe create that fails if another writer creates the destination first. The final replacement is atomic on supported desktop platforms because the temporary file stays on the destination volume.

This protects readers from partial content. It does not guarantee recovery from power loss after the function returns.

## Append Text File

```cpp
static bool AppendTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true);
```

Reads any existing supported text encoding, appends without inserting a separator, then atomically rewrites the complete file as UTF-8 without a byte-order mark. A missing file is created.

## Read Binary File

```cpp
static bool ReadBinaryFile(const FString& Path, TArray<uint8>& OutBytes);
```

Reads a complete binary file. `OutBytes` is empty on failure.

## Get File Size

```cpp
static int64 GetFileSize(const FString& Path);
```

Returns the size in bytes, or -1 when the file does not exist.

## Get File Time Stamp

```cpp
static bool GetFileTimeStamp(const FString& Path, FDateTime& OutTimestamp);
```

Returns the last-modified timestamp as local time. The output is reset on failure.

## Async Read Text File / Async Read Binary File

```cpp
static UDirectiveUtilTask_ReadTextFile* ReadTextFileAsync(UObject* WorldContextObject, const FString& Path);
static UDirectiveUtilTask_ReadBinaryFile* ReadBinaryFileAsync(UObject* WorldContextObject, const FString& Path);
```

Reads the complete file on Unreal's worker pool, then fires `Completed` or `Failed` on the game thread. The result and an error string are exposed on both branches. Canceling suppresses the callback.

## Async Write Text File / Async Write Binary File

```cpp
static UDirectiveUtilTask_WriteTextFile* WriteTextFileAsync(UObject* WorldContextObject, const FString& Path, const FString& Contents, bool bCreateDirectories = true, bool bAllowOverwrite = true, bool bAtomic = true);
static UDirectiveUtilTask_WriteBinaryFile* WriteBinaryFileAsync(UObject* WorldContextObject, const FString& Path, const TArray<uint8>& Bytes, bool bCreateDirectories = true, bool bAllowOverwrite = true, bool bAtomic = true);
```

Writes on Unreal's worker pool and returns to the game thread. Atomic replacement is enabled by default. Canceling suppresses the callback, but a write already handed to the operating system can still finish.

## Watch File

```cpp
static UDirectiveUtilTask_WatchFile* WatchFile(UObject* WorldContextObject, const FString& Path, float PollInterval = 0.25f);
```

Watches one file for `Created`, `Modified`, and `Deleted` changes until canceled or its world closes. It works in packaged Win64, Mac, and Linux games without a developer module. The watcher compares existence, size, and modification time at the requested interval, so several writes inside one interval are coalesced and a same-size rewrite can be missed on a filesystem with coarse timestamp precision.
