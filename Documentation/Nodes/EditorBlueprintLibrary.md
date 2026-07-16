# Editor Blueprint Library

> Read-only Blueprint inspection for editor tools.

**Module:** `DirectiveUtilitiesEditor (Editor)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesEditor/Public/Libraries/DirectiveUtilEditorBlueprintLibrary.h`

Searches use the Asset Registry to find Blueprint assets in the requested package paths before loading the assets needed for inspection. The default search path is `/Game`. These nodes do not compile, reparent, create graphs, or change variables.

---

## Get Blueprint Compile Status
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static EDirectiveUtilBlueprintCompileStatus GetBlueprintCompileStatus(
    const UBlueprint* Blueprint);
```

Returns unknown, dirty, error, up to date, being created, or up to date with warnings. A null Blueprint returns unknown.

## Find Blueprints By Compile Status
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static TArray<FAssetData> FindBlueprintsByCompileStatus(
    EDirectiveUtilBlueprintCompileStatus CompileStatus,
    const FDirectiveUtilBlueprintSearchOptions& Options);
```

Returns Blueprint assets whose current compile status matches the requested value.

## Find Blueprints By Parent Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static TArray<FAssetData> FindBlueprintsByParentClass(
    UClass* ParentClass,
    const FDirectiveUtilBlueprintSearchOptions& Options,
    bool bIncludeDescendants = true);
```

Returns Blueprints with the requested native or Blueprint parent. Disable descendant matching to require that exact immediate parent.

## Find Blueprints Implementing Interface
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static TArray<FAssetData> FindBlueprintsImplementingInterface(
    UClass* InterfaceClass,
    const FDirectiveUtilBlueprintSearchOptions& Options);
```

Returns Blueprints that implement the interface directly or inherit an implementation from a parent class.

## Find Blueprints Containing Component Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static TArray<FAssetData> FindBlueprintsContainingComponentClass(
    UClass* ComponentClass,
    const FDirectiveUtilBlueprintSearchOptions& Options,
    bool bIncludeDerivedComponents = true);
```

Searches construction-script components declared by the Blueprint or an inherited Blueprint. Derived component classes match by default.

## Get Unused Blueprint Variables
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Blueprint Inspection`

```cpp
static TArray<FName> GetUnusedBlueprintVariables(UBlueprint* Blueprint);
```

Returns member-variable names that Unreal's Blueprint analysis marks unused. The result is sorted by name. A null Blueprint returns an empty array.
