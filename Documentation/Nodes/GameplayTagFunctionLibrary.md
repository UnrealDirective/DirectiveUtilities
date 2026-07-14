# Gameplay Tag Function Library

> Hierarchy navigation helpers for Gameplay Tags that the engine does not expose to Blueprints.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilGameplayTagFunctionLibrary.h`

---

## Get Tag Direct Parent
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTag GetTagDirectParent(const FGameplayTag& Tag);
```

Returns the direct (immediate) parent of a tag, e.g. "A.B.C" returns "A.B".

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** The direct parent tag, or an invalid tag if the tag is a root or invalid.

## Get Tag Parents
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTagContainer GetTagParents(const FGameplayTag& Tag);
```

Returns all ancestor tags of a tag (its parents, grandparents, etc.), excluding the tag itself, e.g. "A.B.C" returns { "A.B", "A" }.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** A container of the tag's ancestors.

## Get Tag Depth
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static int32 GetTagDepth(const FGameplayTag& Tag);
```

Returns the number of segments (hierarchy depth) of a tag, e.g. "A.B.C" returns 3.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** The depth, or 0 for an invalid tag.

## Get Tag Leaf Name
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FString GetTagLeafName(const FGameplayTag& Tag);
```

Returns the last (leaf) segment of a tag's name, e.g. "A.B.C" returns "C".

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** The leaf segment, or an empty string for an invalid tag.

## Get Tag Segments
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static TArray<FString> GetTagSegments(const FGameplayTag& Tag);
```

Splits a tag's name into its individual segments, e.g. "A.B.C" returns [ "A", "B", "C" ].

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** The segments in order, or an empty array for an invalid tag.

## Get Tag Children
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTagContainer GetTagChildren(const FGameplayTag& Tag);
```

Returns all registered descendant tags of a tag (children, grandchildren, etc.), excluding the tag itself, e.g. "A" returns { "A.B", "A.B.C" } when those tags are registered.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** A container of the tag's descendants, or an empty container for an invalid tag.

## Get Tag Direct Children
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTagContainer GetTagDirectChildren(const FGameplayTag& Tag);
```

Returns only the direct (immediate) registered children of a tag, e.g. "A" returns "A.B" but not "A.B.C".

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** A container of the tag's direct children, or an empty container for an invalid tag.

## Get Tag Common Ancestor
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTag GetTagCommonAncestor(const FGameplayTag& TagA, const FGameplayTag& TagB);
```

Returns the deepest tag that both tags share as an ancestor, e.g. "A.B.C" and "A.B.D" return "A.B". The result may be one of the inputs when one tag is an ancestor of the other.

| Parameter | Type | Description |
|-----------|------|-------------|
| TagA | `const FGameplayTag&` | The first tag. |
| TagB | `const FGameplayTag&` | The second tag. |

**Returns:** The deepest common ancestor tag, or an invalid tag if the tags share none.

## Get Tag At Depth
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTag GetTagAtDepth(const FGameplayTag& Tag, int32 Depth);
```

Truncates a tag to its first Depth segments, e.g. "A.B.C" at depth 2 returns "A.B". An ancestor of a registered tag is always registered itself.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to truncate. |
| Depth | `int32` | The number of leading segments to keep. |

**Returns:** The truncated tag, the tag itself when Depth >= its depth, or an invalid tag when Depth < 1 or the tag is invalid.

## Get Tag Siblings
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static FGameplayTagContainer GetTagSiblings(const FGameplayTag& Tag);
```

Returns the registered tags that share a tag's direct parent, excluding the tag itself, e.g. "A.B" returns "A.C" when both are registered. Enumerating the siblings of a root tag (which has no parent) is not supported and returns an empty container.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to read. |

**Returns:** A container of the tag's siblings, or an empty container for an invalid or root tag.

## Is Leaf Tag
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static bool IsLeafTag(const FGameplayTag& Tag);
```

Checks whether a tag has no registered children.

| Parameter | Type | Description |
|-----------|------|-------------|
| Tag | `const FGameplayTag&` | The tag to test. |

**Returns:** True if the tag is valid and has no registered descendants; false for an invalid tag.

## Find Registered Tags
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|GameplayTags`

```cpp
static TArray<FGameplayTag> FindRegisteredTags(const FString& Substring);
```

Finds every registered gameplay tag whose name contains Substring (case-insensitive), in registry order. An empty substring returns an empty array. Cost scales with the size of the tag registry; intended for tooling and debug use, not per-frame calls.

| Parameter | Type | Description |
|-----------|------|-------------|
| Substring | `const FString&` | The text to search for. An empty string returns an empty array. |

**Returns:** The matching tags.
