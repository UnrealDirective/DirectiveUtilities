# Async Tasks

> Latent Blueprint async-action nodes for timed flow, soft asset/class loading, off-thread collision traces, path-following movement to a location or actor, and file reads, writes, and watching.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_Delay.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_Flow.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_AsyncLoadAsset.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_AsyncTrace.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_MoveToLocation.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_FileSystem.h`

---

## Cancellable Delay
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`

```cpp
static UDirectiveUtilTask_Delay* CancellableDelay(UObject* WorldContextObject, float Duration);
```

Starts a cancellable delay. When the delay finishes the `Completed` delegate fires. Non-finite and non-positive durations complete on the next timer tick. Call `Cancel` on the async proxy to stop it.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Duration | `float` | The duration of the delay in seconds. |

**Output exec pins:**

- `Completed` (`FOnDelayCompleted`, no params): fired when the delay has completed.

`EndTask` remains available as a deprecated alias for existing Blueprints.

## Update for Duration
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`

```cpp
static UDirectiveUtilTask_UpdateForDuration* UpdateForDuration(
    UObject* WorldContextObject,
    float Duration,
    float UpdateInterval = 0.0f);
```

Reports elapsed time and normalized progress until the duration ends. An update interval of zero runs once per world tick. The final update has an alpha of `1.0` and fires before `Completed`. Game pause stops the timer, and time dilation scales it.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Duration | `float` | The duration in seconds. Non-positive and non-finite values finish on the next tick. |
| UpdateInterval | `float` | The minimum time between updates. Zero updates each tick. |

**Output exec pins:**

- `Updated` (`FOnDurationUpdated`, `float ElapsedTime, float DeltaTime, float Alpha`): fired as game time advances.
- `Completed` (`FOnDurationCompleted`, no params): fired after the final update.

Call `Cancel` on the async proxy to stop without firing either delegate again.

## Repeat with Interval
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`

```cpp
static UDirectiveUtilTask_RepeatWithInterval* RepeatWithInterval(
    UObject* WorldContextObject,
    int32 Count,
    float Interval,
    float InitialDelay = 0.0f);
```

Runs a fixed number of iterations, or forever when Count is `-1`. Zero intervals run once per world tick, and longer intervals fire at most once per frame. The first index is zero. An infinite repeat wraps the index to zero after `MAX_int32`. For a finite count, `Remaining` is the iterations left after the current one; for `-1` it stays `-1`. Game pause stops the timer, and time dilation scales it.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Count | `int32` | Iteration count. `-1` repeats until Cancel. Zero and other negative values complete on the next tick with no iterations. |
| Interval | `float` | The delay between iterations. Zero runs on consecutive ticks. |
| InitialDelay | `float` | The delay before the first iteration. Zero starts on the next tick. |

**Output exec pins:**

- `Iteration` (`FOnRepeatIteration`, `int32 Index, int32 Remaining`): fired once per iteration.
- `Completed` (`FOnRepeatCompleted`, no params): fired after the final finite iteration. Not fired for an infinite count unless you stop with Cancel, which does not fire Completed.

Call `Cancel` on the async proxy to stop without firing either delegate again.

## End Task
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`, `Directive Utilities|Navigation`

```cpp
void UDirectiveUtilTask_Delay::EndTask();
void UDirectiveUtilTask_MoveToLocation::EndTask();
void UDirectiveUtilTask_MoveToActor::EndTask();
```

Ends the async proxy early. `EndTask` is deprecated for cancellable delay; use `Cancel`. On movement tasks, it stops the task and fires `Completed` exactly once with `bSuccess` set to false.

## Async Load Asset
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|AssetManagement`

```cpp
static UDirectiveUtilTask_AsyncLoadAsset* AsyncLoadAsset(UObject* WorldContextObject, const TSoftObjectPtr<UObject> Asset);
```

Asynchronously loads the asset referenced by a soft object pointer. `Completed` fires with the loaded asset on success; `Failed` fires with null on failure. Call `Cancel` on the async proxy to abort the load (a manual cancel does not broadcast `Failed`).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Asset | `const TSoftObjectPtr<UObject>` | The soft object reference to load. |

**Output exec pins:**
- `Completed` (`FOnAsyncLoadAssetCompleted`, `UObject* LoadedAsset`): fired when the asset has finished loading; `LoadedAsset` is valid.
- `Failed` (`FOnAsyncLoadAssetCompleted`, `UObject* LoadedAsset`): fired when the load failed; `LoadedAsset` is null.

## Async Load Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|AssetManagement`

```cpp
static UDirectiveUtilTask_AsyncLoadClass* AsyncLoadClass(UObject* WorldContextObject, const TSoftClassPtr<UObject> AssetClass);
```

Asynchronously loads the class referenced by a soft class pointer. `Completed` fires with the loaded class on success; `Failed` fires with null on failure. Call `Cancel` on the async proxy to abort the load (a manual cancel does not broadcast `Failed`).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| AssetClass | `const TSoftClassPtr<UObject>` | The soft class reference to load. |

**Output exec pins:**
- `Completed` (`FOnAsyncLoadClassCompleted`, `UClass* LoadedClass`): fired when the class has finished loading; `LoadedClass` is valid.
- `Failed` (`FOnAsyncLoadClassCompleted`, `UClass* LoadedClass`): fired when the load failed; `LoadedClass` is null.

## Async Load Assets
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|AssetManagement`

```cpp
static UDirectiveUtilTask_AsyncLoadAssets* AsyncLoadAssets(UObject* WorldContextObject, const TArray<TSoftObjectPtr<UObject>>& Assets);
```

Asynchronously loads the assets referenced by an array of soft object pointers in a single request. `Completed` fires exactly once with the loaded assets in input order; entries that were unset or failed to resolve are null, and the assets are only guaranteed alive during the broadcast. An empty input array completes immediately with an empty array. `Progress` fires as assets arrive (it is not called when the request finishes before the first update, e.g. all assets were already in memory). Call `Cancel` on the async proxy to abort the load (a manual cancel does not broadcast `Completed`).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Assets | `const TArray<TSoftObjectPtr<UObject>>&` | The soft object references to load. |

**Output exec pins:**
- `Completed` (`FOnAsyncLoadAssetsCompleted`, `const TArray<UObject*>& LoadedAssets`): fired exactly once when the batch has finished loading; `LoadedAssets` is in input order with null entries for references that were unset or failed to resolve.
- `Progress` (`FOnAsyncLoadAssetsProgress`, `int32 LoadedCount, int32 TotalCount`): fired as assets arrive, with the number loaded so far and the total requested.

## Cancel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|AssetManagement`

```cpp
void UDirectiveUtilTask_AsyncLoadAsset::Cancel();
void UDirectiveUtilTask_AsyncLoadClass::Cancel();
void UDirectiveUtilTask_AsyncLoadAssets::Cancel();
```

Cancels an in-progress async load proxy and marks it ready to destroy. Manual cancellation does not fire `Failed` for single asset/class loads and does not fire `Completed` for batch asset loads.

## Async Line Trace By Channel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Collision`

```cpp
static UDirectiveUtilTask_AsyncTrace* AsyncLineTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);
```

Queues a line trace against the given trace channel and reports the result on the next tick. The trace cannot be cancelled.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Start | `const FVector` | The start of the trace. |
| End | `const FVector` | The end of the trace. |
| TraceChannel | `const ETraceTypeQuery` | The trace channel to test against. |
| bMultiTrace | `const bool` | If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit. |

**Output exec pins:**
- `Completed` (`FOnAsyncTraceCompleted`, `const TArray<FHitResult>& Hits`): fired when the trace has completed; `Hits` is empty if nothing was hit.

## Async Sphere Trace By Channel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Collision`

```cpp
static UDirectiveUtilTask_AsyncTrace* AsyncSphereTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);
```

Queues a sphere sweep against the given trace channel and reports the result on the next tick. The sweep cannot be cancelled.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Start | `const FVector` | The start of the sweep. |
| End | `const FVector` | The end of the sweep. |
| Radius | `const float` | The radius of the sphere. |
| TraceChannel | `const ETraceTypeQuery` | The trace channel to test against. |
| bMultiTrace | `const bool` | If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit. |

**Output exec pins:**
- `Completed` (`FOnAsyncTraceCompleted`, `const TArray<FHitResult>& Hits`): fired when the sweep has completed; `Hits` is empty if nothing was hit.

## Async Box Trace By Channel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Collision`

```cpp
static UDirectiveUtilTask_AsyncTrace* AsyncBoxTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const FVector HalfSize, const FRotator Orientation, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);
```

Queues a box sweep against the given trace channel and reports the result on the next tick. The sweep cannot be cancelled.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Start | `const FVector` | The start of the sweep. |
| End | `const FVector` | The end of the sweep. |
| HalfSize | `const FVector` | The half-extents of the box. |
| Orientation | `const FRotator` | The orientation of the box. |
| TraceChannel | `const ETraceTypeQuery` | The trace channel to test against. |
| bMultiTrace | `const bool` | If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit. |

**Output exec pins:**
- `Completed` (`FOnAsyncTraceCompleted`, `const TArray<FHitResult>& Hits`): fired when the sweep has completed; `Hits` is empty if nothing was hit.

## Async Capsule Trace By Channel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Collision`

```cpp
static UDirectiveUtilTask_AsyncTrace* AsyncCapsuleTraceByChannel(UObject* WorldContextObject, const FVector Start, const FVector End, const float Radius, const float HalfHeight, const ETraceTypeQuery TraceChannel, const bool bMultiTrace = false);
```

Queues a capsule sweep against the given trace channel and reports the result on the next tick. The sweep cannot be cancelled.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Start | `const FVector` | The start of the sweep. |
| End | `const FVector` | The end of the sweep. |
| Radius | `const float` | The radius of the capsule. |
| HalfHeight | `const float` | The half-height of the capsule (including the radius). |
| TraceChannel | `const ETraceTypeQuery` | The trace channel to test against. |
| bMultiTrace | `const bool` | If true, returns all hits up to and including the first blocking hit; otherwise returns only the first blocking hit. |

**Output exec pins:**
- `Completed` (`FOnAsyncTraceCompleted`, `const TArray<FHitResult>& Hits`): fired when the sweep has completed; `Hits` is empty if nothing was hit.

## Async Move To Location
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Navigation`

```cpp
static UDirectiveUtilTask_MoveToLocation* MoveToLocation(
    UObject* WorldContextObject,
    AController* Controller,
    FVector Destination,
    float AcceptanceRadius = 100.0f,
    bool bCheckStuckMovement = true,
    float StuckThreshold = 1.0f,
    bool bDebugLineTrace = false);
```

Moves the actor to the specified location; when movement succeeds or fails the `Completed` delegate fires with the result, exactly once. The task automatically ends if the controller, pawn, or world is unavailable, if path-following stops for any reason (success still requires ending within `AcceptanceRadius`), or if `bCheckStuckMovement` is enabled and the controller gets stuck. Negative and non-finite radius and threshold values are clamped to zero. Call `EndTask` on the async proxy to manually end the task when it is no longer needed.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Controller | `AController*` | The controller to move. |
| Destination | `FVector` | The vector location to move to. |
| AcceptanceRadius | `float` | The radius around the destination considered acceptable. Set this to a reasonable value as the controller may never reach the exact destination. |
| bCheckStuckMovement | `bool` | Check if the controller gets stuck while moving. |
| StuckThreshold | `float` | The distance threshold to consider the controller stuck. |
| bDebugLineTrace | `bool` | Display a line trace to the destination location for a short duration. |

**Output exec pins:**
- `Completed` (`FOnAsyncMoveToLocation`, `bool bSuccess`): fired when the movement has completed regardless of success; `bSuccess` indicates whether the destination was reached.

## Async Move To Actor
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Navigation`

```cpp
static UDirectiveUtilTask_MoveToActor* MoveToActor(
    UObject* WorldContextObject,
    AController* Controller,
    AActor* Goal,
    float AcceptanceRadius = 100.0f,
    bool bCheckStuckMovement = true,
    float StuckThreshold = 1.0f);
```

Moves the controller's pawn to the goal actor; when movement succeeds or fails the `Completed` delegate fires with the result, exactly once. The goal's location is re-read every poll, so a moving goal is tracked. The task automatically ends if the controller, pawn, goal actor, or world is unavailable, if path-following stops for any reason (success still requires ending within `AcceptanceRadius` of the goal), or if `bCheckStuckMovement` is enabled and the controller gets stuck. Negative and non-finite radius and threshold values are clamped to zero. Call `EndTask` on the async proxy to manually end the task when it is no longer needed.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Controller | `AController*` | The controller to move. |
| Goal | `AActor*` | The actor to move to. |
| AcceptanceRadius | `float` | The radius around the goal actor considered acceptable. Set this to a reasonable value as the controller may never reach the goal's exact location. |
| bCheckStuckMovement | `bool` | Check if the controller gets stuck while moving. |
| StuckThreshold | `float` | The distance threshold to consider the controller stuck. |

**Output exec pins:**
- `Completed` (`FOnAsyncMoveToActor`, `bool bSuccess`): fired when the movement has completed regardless of success; `bSuccess` indicates whether the goal was reached.

## Async Read Text File
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FileSystem`

```cpp
static UDirectiveUtilTask_ReadTextFile* ReadTextFileAsync(UObject* WorldContextObject, const FString& Path);
```

Reads a complete text file on Unreal's worker pool with the same rules as [Read Text File](FileSystemFunctionLibrary.md#read-text-file), then fires one delegate on the game thread. Call `Cancel` on the async proxy to suppress the callback. In a world without a game instance the node keeps itself alive until it finishes and still fires its delegates.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. Without one, `Failed` fires from `Activate`. |
| Path | `const FString&` | The file to read, absolute or relative to the project `Saved` directory. |

**Output exec pins:**
- `Completed` (`FDirectiveUtilAsyncTextFileResult`, `const FString& Contents, const FString& Error`): fired with the file contents and an empty `Error`.
- `Failed` (`FDirectiveUtilAsyncTextFileResult`, `const FString& Contents, const FString& Error`): fired with empty `Contents` when the world context or path is invalid or the file cannot be read.

## Async Read Binary File
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FileSystem`

```cpp
static UDirectiveUtilTask_ReadBinaryFile* ReadBinaryFileAsync(UObject* WorldContextObject, const FString& Path);
```

Reads a complete binary file on Unreal's worker pool, then fires one delegate on the game thread. Cancellation and worlds without a game instance behave as in [Async Read Text File](#async-read-text-file).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. Without one, `Failed` fires from `Activate`. |
| Path | `const FString&` | The file to read, absolute or relative to the project `Saved` directory. |

**Output exec pins:**
- `Completed` (`FDirectiveUtilAsyncBinaryFileResult`, `const TArray<uint8>& Bytes, const FString& Error`): fired with the file bytes and an empty `Error`.
- `Failed` (`FDirectiveUtilAsyncBinaryFileResult`, `const TArray<uint8>& Bytes, const FString& Error`): fired with empty `Bytes` when the world context or path is invalid or the file cannot be read.

## Async Write Text File
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FileSystem`

```cpp
static UDirectiveUtilTask_WriteTextFile* WriteTextFileAsync(
    UObject* WorldContextObject,
    const FString& Path,
    const FString& Contents,
    bool bCreateDirectories = true,
    bool bAllowOverwrite = true,
    bool bAtomic = true);
```

Writes text as UTF-8 without a byte-order mark on Unreal's worker pool, then fires one delegate on the game thread. `bAtomic` selects [Write Text File Atomic](FileSystemFunctionLibrary.md#write-text-file-atomic--write-binary-file-atomic) or [Write Text File](FileSystemFunctionLibrary.md#write-text-file--write-binary-file). `Cancel` suppresses the callback, but a write that has already started still finishes on disk. Worlds without a game instance behave as in [Async Read Text File](#async-read-text-file).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. Without one, `Failed` fires from `Activate`. |
| Path | `const FString&` | The file to write, absolute or relative to the project `Saved` directory. |
| Contents | `const FString&` | The text to write. |
| bCreateDirectories | `bool` | Creates missing parent directories when true. |
| bAllowOverwrite | `bool` | When false, the write fails if the file already exists. |
| bAtomic | `bool` | When true, writes a temporary sibling and moves it over the destination. |

**Output exec pins:**
- `Completed` (`FDirectiveUtilAsyncFileWriteResult`, `const FString& Error`): fired after the file is written; `Error` is empty.
- `Failed` (`FDirectiveUtilAsyncFileWriteResult`, `const FString& Error`): fired when the world context or path is invalid, overwriting was refused, or the write failed.

## Async Write Binary File
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FileSystem`

```cpp
static UDirectiveUtilTask_WriteBinaryFile* WriteBinaryFileAsync(
    UObject* WorldContextObject,
    const FString& Path,
    const TArray<uint8>& Bytes,
    bool bCreateDirectories = true,
    bool bAllowOverwrite = true,
    bool bAtomic = true);
```

Writes bytes on Unreal's worker pool, then fires one delegate on the game thread. Options, cancellation, and failures match [Async Write Text File](#async-write-text-file).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. Without one, `Failed` fires from `Activate`. |
| Path | `const FString&` | The file to write, absolute or relative to the project `Saved` directory. |
| Bytes | `const TArray<uint8>&` | The bytes to write. |
| bCreateDirectories | `bool` | Creates missing parent directories when true. |
| bAllowOverwrite | `bool` | When false, the write fails if the file already exists. |
| bAtomic | `bool` | When true, writes a temporary sibling and moves it over the destination. |

**Output exec pins:**
- `Completed` (`FDirectiveUtilAsyncFileWriteResult`, `const FString& Error`): fired after the file is written; `Error` is empty.
- `Failed` (`FDirectiveUtilAsyncFileWriteResult`, `const FString& Error`): fired when the world context or path is invalid, overwriting was refused, or the write failed.

## Watch File
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FileSystem`

```cpp
static UDirectiveUtilTask_WatchFile* WatchFile(UObject* WorldContextObject, const FString& Path, float PollInterval = 0.25f);
```

Polls one file until `Cancel` is called or the world is cleaned up. Polling uses real time on the engine's core ticker, so game pause and time dilation do not affect it. Each poll compares existence, size, and modification time, plus a CRC-32 of files up to 1 MiB that changed in the last 2 seconds. Polls read the file on Unreal's worker pool and fire `Changed` on the game thread. Several writes between two polls are reported as one change. [Watch File](FileSystemFunctionLibrary.md#watch-file) lists the detection limits. Worlds without a game instance behave as in [Async Read Text File](#async-read-text-file).

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. Without one, `Failed` fires from `Activate`. |
| Path | `const FString&` | The file to watch, absolute or relative to the project `Saved` directory. |
| PollInterval | `float` | Real seconds between polls. Values shorter than one frame poll once per frame. Non-positive and non-finite values fire `Failed`. |

**Output exec pins:**
- `Changed` (`FDirectiveUtilFileChanged`, `EDirectiveUtilFileChangeType ChangeType, const FString& Path`): fired once per detected change with `Created`, `Modified`, or `Deleted` and the path as it was passed in.
- `Failed` (`FDirectiveUtilFileWatchFailed`, `const FString& Error`): fired from `Activate` when the world context, path, or poll interval is invalid. No further polls run.

## Worlds without a game instance

Async nodes normally stay alive by registering with the world's game instance. Editor worlds, such as those used by Editor Utility Blueprints, have no game instance. In those worlds the delay, flow, load, trace, movement, and file nodes keep themselves alive until they finish, are canceled, or the world is cleaned up, and their delegates still fire. Timers run on the world's timer manager, so they follow that world's time.

World cleanup stops a pending node and releases it without firing its delegates. A node never fires a delegate after it has finished or been canceled. A node created without a world context cannot start timers, traces, or movement. The load nodes do not need a world and keep themselves alive until the load finishes or is canceled.
