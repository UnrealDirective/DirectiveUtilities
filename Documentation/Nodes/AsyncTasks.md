# Async Tasks

> Latent Blueprint async-action nodes for cancellable delays, soft asset/class loading, off-thread collision traces, and path-following movement to a location or actor.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_Delay.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_AsyncLoadAsset.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_AsyncTrace.h`, `Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_MoveToLocation.h`

---

## Cancellable Delay
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`

```cpp
static UDirectiveUtilTask_Delay* CancellableDelay(UObject* WorldContextObject, float Duration);
```

Starts a cancellable delay. When the delay finishes the `Completed` delegate fires. Call `EndTask` on the async proxy to cancel the delay before it completes.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldContextObject | `UObject*` | The world context object. |
| Duration | `float` | The duration of the delay in seconds. |

**Output exec pins:**
- `Completed` (`FOnDelayCompleted`, no params): fired when the delay has completed.

## End Task
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|FlowControl`, `Directive Utilities|Navigation`

```cpp
void UDirectiveUtilTask_Delay::EndTask();
void UDirectiveUtilTask_MoveToLocation::EndTask();
void UDirectiveUtilTask_MoveToActor::EndTask();
```

Ends the async proxy early. On a cancellable delay, this clears the timer and does not fire `Completed`. On movement tasks, this stops the task and fires `Completed` exactly once with `bSuccess` set to false.

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

Moves the actor to the specified location; when movement succeeds or fails the `Completed` delegate fires with the result, exactly once. The task automatically ends if the controller or pawn is destroyed while moving, if path-following stops for any reason (success still requires ending within `AcceptanceRadius`), or if `bCheckStuckMovement` is enabled and the controller gets stuck. Call `EndTask` on the async proxy to manually end the task when it is no longer needed.

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

Moves the controller's pawn to the goal actor; when movement succeeds or fails the `Completed` delegate fires with the result, exactly once. The goal's location is re-read every poll, so a moving goal is tracked. The task automatically ends if the controller, pawn, or goal actor is destroyed while moving, if path-following stops for any reason (success still requires ending within `AcceptanceRadius` of the goal), or if `bCheckStuckMovement` is enabled and the controller gets stuck. Call `EndTask` on the async proxy to manually end the task when it is no longer needed.

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
