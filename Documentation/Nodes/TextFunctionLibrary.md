# Text Function Library

> A collection of helpful text utility functions that improve the usability of text in Blueprints.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilTextFunctionLibrary.h`

---

## Is Not Empty
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Text`

```cpp
static bool IsNotEmpty(const FText& Text);
```

Returns true if the provided text is not empty.

| Parameter | Type | Description |
|-----------|------|-------------|
| Text | `const FText&` | The text to check. |

**Returns:** `bool`: true if the text is not empty.
