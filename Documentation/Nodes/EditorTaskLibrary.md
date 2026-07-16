# Editor Task Library

> Progress and notification nodes for editor scripts.

**Module:** `DirectiveUtilitiesEditor (Editor)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesEditor/Public/Libraries/DirectiveUtilEditorTaskLibrary.h`

---

## Start Editor Slow Task
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Task`

```cpp
static UDirectiveUtilEditorSlowTask* StartEditorSlowTask(
    float TotalWork,
    const FText& Description,
    bool bCanCancel = false);
```

Opens an editor progress dialog and returns its task object. A total of zero or less returns null.

| Parameter | Type | Description |
|-----------|------|-------------|
| TotalWork | `float` | The total amount of work represented by the task. |
| Description | `FText` | The text shown when the task starts. |
| bCanCancel | `bool` | Shows a cancel button when enabled. |

Store the returned object until the work finishes. Destroying an active task closes its dialog.

## Advance
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Task`

```cpp
bool Advance(float Work, const FText& Message);
```

Advances the task and updates its message. Returns false after the task is finished or when `Work` is negative.

## Is Cancel Requested
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Task`

```cpp
bool IsCancelRequested() const;
```

Returns true after the user presses the task's cancel button. The caller decides when to stop its work.

## Is Active
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Task`

```cpp
bool IsActive() const;
```

Returns true until `Finish` runs or the task object is destroyed.

## Finish
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Task`

```cpp
void Finish();
```

Closes the progress task. Calling it more than once is safe.

## Show Editor Notification
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Notification`

```cpp
static bool ShowEditorNotification(
    const FText& Message,
    EDirectiveUtilEditorNotificationState State,
    float ExpireDuration = 3.0f);
```

Displays a neutral, success, warning, or failure notification. Returns false when Slate is unavailable or the notification could not be created. Negative lifetimes are treated as zero.
