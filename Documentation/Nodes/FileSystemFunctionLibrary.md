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

Each CRLF, LF, or CR sequence ends one line. The text after the last line ending becomes the final line only when it is not empty. An empty file returns no lines, `a\n` returns `a`, and a file that holds only `\n` returns one empty line. With `bIncludeEmptyLines` set to false, lines with no characters are dropped. Lines that hold only spaces or tabs are kept.

## Write Text File / Write Binary File

```cpp
static bool WriteTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
static bool WriteBinaryFile(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
```

Writes the complete value to the destination. Text uses UTF-8 without a byte-order mark. Missing parent directories are created when requested. Setting `bAllowOverwrite` false refuses an existing destination and uses the atomic write path below.

## Write Text File Atomic / Write Binary File Atomic

```cpp
static bool WriteTextFileAtomic(const FString& Path, const FString& Contents, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
static bool WriteBinaryFileAtomic(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories = true, const bool bAllowOverwrite = true);
```

Writes a temporary sibling named `<file>.tmp-<32 hex digits>`, then moves it over the destination. A failed write or move leaves the previous destination unchanged and removes the temporary file. Setting `bAllowOverwrite` false performs a race-safe create that fails if another writer creates the destination first. The final move is atomic on supported desktop platforms because the temporary file stays on the destination volume.

This protects readers from partial content. It does not guarantee recovery from power loss after the function returns.

Platform behavior:

- Windows accepts destination paths of 260 characters or more, including UNC paths, using the same `\\?\` and `\\?\UNC\` prefixes as the engine's own file writes. When antivirus or search indexing holds the new file open, the move is retried for about 100 ms before the call fails.
- Mac and Linux create a no-overwrite destination with a hard link. On filesystems without hard links, such as FAT, exFAT, and some network shares, the call instead creates an empty destination with exclusive create and then moves the temporary file over it. Readers can see that empty file until the move completes, and a writer that replaces the destination during that window is overwritten.

Limits:

- The destination is replaced, not edited. A symbolic link at the destination is replaced by a regular file rather than followed, and the new file has default permissions instead of the old file's permissions.
- A process that exits during the write can leave the temporary sibling behind. Later calls do not remove it.
- Atomic writes need a filesystem-backed destination, so pak and other virtual files are not supported.

## Append Text File

```cpp
static bool AppendTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories = true);
```

Appends to the end of the file without inserting a separator and without rewriting the existing bytes. A missing file is created with UTF-8 text and no byte-order mark. When the file starts with a UTF-16 byte-order mark, the text is appended as UTF-16 in the same byte order. Every other file receives UTF-8 text, so a Latin-1 file keeps its original bytes followed by UTF-8 bytes. An unreadable existing file, a file that starts with a UTF-32 byte-order mark, or a UTF-16 file with an odd byte count fails without changing the file. The append is not atomic. A failure part way through can leave part of the text in the file.

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

Reads the complete file on Unreal's worker pool, then fires `Completed` or `Failed` on the game thread. The result and an error string are exposed on both branches. A missing world context, an empty path, or an unreadable file fires `Failed`. Canceling suppresses the callback. See [Async tasks](AsyncTasks.md#async-read-text-file) for the pins.

## Async Write Text File / Async Write Binary File

```cpp
static UDirectiveUtilTask_WriteTextFile* WriteTextFileAsync(UObject* WorldContextObject, const FString& Path, const FString& Contents, bool bCreateDirectories = true, bool bAllowOverwrite = true, bool bAtomic = true);
static UDirectiveUtilTask_WriteBinaryFile* WriteBinaryFileAsync(UObject* WorldContextObject, const FString& Path, const TArray<uint8>& Bytes, bool bCreateDirectories = true, bool bAllowOverwrite = true, bool bAtomic = true);
```

Writes on Unreal's worker pool and returns to the game thread. Atomic replacement is enabled by default. Setting `bAtomic` false uses `WriteTextFile` or `WriteBinaryFile`. Canceling suppresses the callback, but a write that has already started still finishes on disk. See [Async tasks](AsyncTasks.md#async-write-text-file) for the pins.

## Watch File

```cpp
static UDirectiveUtilTask_WatchFile* WatchFile(UObject* WorldContextObject, const FString& Path, float PollInterval = 0.25f);
```

Watches one file for `Created`, `Modified`, and `Deleted` changes until canceled or its world is cleaned up. It works in packaged Win64, Mac, and Linux games without a developer module. `Changed` returns the path exactly as it was passed in, including a `Saved`-relative path.

Each poll compares existence, size, and modification time. Mac and Linux report modification times in whole seconds, so the watcher also compares a CRC-32 of the contents while the file's modification time is within 2 seconds of the current time. This catches a same-size rewrite inside the same second. The contents check only covers files of 1 MiB or less. A larger file rewritten with the same size inside the same second is not reported. Several writes between two polls are reported as one change.

Polling runs on the engine's core ticker with real time. Game pause and time dilation do not slow or stop it. Each poll reads the file state and contents on Unreal's worker pool and reports changes on the game thread; a poll that is still running when the next one is due is not repeated. The first snapshot, taken when the node activates, runs on the game thread. An interval shorter than one frame polls once per frame. A non-positive or non-finite interval, a missing world context, or an empty path fires `Failed`. See [Async tasks](AsyncTasks.md#watch-file) for the pins.

## Worlds without a game instance

The async nodes normally stay alive by registering with the world's game instance. Editor worlds, such as those used by Editor Utility Blueprints, have no game instance. In those worlds each node keeps itself alive until it finishes, is canceled, or the world is cleaned up, and its delegates still fire. A node created without a world context fires `Failed` from `Activate`.
