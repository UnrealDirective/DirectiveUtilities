# Editor Actor Subsystem

> Blueprint helpers for querying and filtering actors in the editor world.

**Module:** `DirectiveUtilitiesEditor (Editor)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesEditor/Public/Subsystems/DirectiveUtilEditorActorSubsystem.h`

---

## Utilities

## Focus Actors In Viewport
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor`

```cpp
static void FocusActorsInViewport(const TArray<AActor*> Actors, bool bInstant = false);
```

Focus the provided actors in the editor viewport.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The actors to focus. |
| bInstant | `bool` | Enable to focus the actors instantly instead of smoothly animating. |

## Get All Level Classes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor`

```cpp
TArray<UClass*> GetAllLevelClasses();
```

Get all unique classes used in the level.

**Returns:** The list of unique classes found in the level.

## Align Actors
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Actor Layout`

```cpp
static FDirectiveUtilActorOperationResult AlignActors(
    const TArray<AActor*>& Actors,
    EDirectiveUtilActorLayoutAxis Axis,
    EDirectiveUtilActorAlignment Alignment);
```

Aligns actor bounds on one world axis. Minimum and maximum alignment use the outer edge of the combined bounds. Center alignment uses the center of the combined bounds. Other location axes are unchanged.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The actors to align. Editor selection is not read. |
| Axis | `EDirectiveUtilActorLayoutAxis` | X, Y, or Z. |
| Alignment | `EDirectiveUtilActorAlignment` | Minimum, center, or maximum bounds alignment. |

**Returns:** Actors whose locations changed and actors skipped because they were invalid, transient, or pending destruction. Duplicate inputs are processed once. The operation uses one undo transaction and preserves attachments.

## Distribute Actors
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Actor Layout`

```cpp
static FDirectiveUtilActorOperationResult DistributeActors(
    const TArray<AActor*>& Actors,
    EDirectiveUtilActorLayoutAxis Axis,
    EDirectiveUtilActorDistribution Distribution);
```

Sorts actors by bounds center and distributes the actors between the two outer actors. Center mode uses equal center-to-center spacing. Bounds Gaps mode uses equal empty space between adjacent bounds. The outer actors remain in place.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The actors to distribute. At least three valid actors are required. |
| Axis | `EDirectiveUtilActorLayoutAxis` | X, Y, or Z. |
| Distribution | `EDirectiveUtilActorDistribution` | Equal center spacing or equal bounds gaps. |

**Returns:** Actors whose locations changed and actors skipped during validation. The operation uses one undo transaction and preserves attachments.

## Snap Actors To Surface
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Editor|Actor Layout`

```cpp
static FDirectiveUtilActorOperationResult SnapActorsToSurface(
    const TArray<AActor*>& Actors,
    FVector TraceDirection,
    float MaximumDistance,
    ECollisionChannel TraceChannel,
    EDirectiveUtilSurfacePlacement Placement,
    bool bAlignToNormal = false);
```

Traces from each actor location and moves the actor to the first blocking hit. Pivot mode places the pivot on the hit. Bounds mode offsets the actor so its directional bounds touch the hit. Input actors are ignored by every trace.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The actors to snap. Editor selection is not read. |
| TraceDirection | `FVector` | The trace direction. It is normalized before use. |
| MaximumDistance | `float` | The maximum trace distance. Must be greater than zero. |
| TraceChannel | `ECollisionChannel` | The collision channel used by the trace. |
| Placement | `EDirectiveUtilSurfacePlacement` | Places the pivot or bounds on the hit. |
| bAlignToNormal | `bool` | Rotates the actor so its up vector matches the hit normal. |

**Returns:** Actors whose transforms changed and actors skipped because validation failed or no surface was hit. The operation uses one undo transaction and preserves attachments.

---

## Filters

Output arrays are appended to and deduplicated (existing entries are preserved). Passing the same array as both input and output filters it in place.

## Filter Actors By Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& ActorName, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filters the provided actors based on the provided name.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| ActorName | `FString` | The name to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided name. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByClass(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSubclassOf<AActor> ActorClass, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided class.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| ActorClass | `TSubclassOf<AActor>` | The class to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided class. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Tag
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByTag(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FName Tag, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided tag.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Tag | `FName` | The tag to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided tags. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Material Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMaterialName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& MaterialName, EDirectiveUtilSearchLocation MaterialSource, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided material name.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MaterialName | `FString` | The material name to filter by. |
| MaterialSource | `EDirectiveUtilSearchLocation` | The location to check for the material. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided material name. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Material
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMaterial(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UMaterialInterface>& Material, EDirectiveUtilSearchLocation MaterialSource, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided material reference.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Material | `TSoftObjectPtr<UMaterialInterface>` | The material reference to filter by. |
| MaterialSource | `EDirectiveUtilSearchLocation` | The location to check for the material. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided material reference. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Static Mesh Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByStaticMeshName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& StaticMeshName, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided static mesh name. Uses a case-insensitive substring match against the mesh asset name.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| StaticMeshName | `FString` | The static mesh name to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided static mesh name. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Static Mesh
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByStaticMesh(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UStaticMesh>& StaticMesh, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided static mesh reference.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| StaticMesh | `TSoftObjectPtr<UStaticMesh>` | The static mesh reference to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided static mesh reference. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Vert Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByVertCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinVertCount, int32 MaxVertCount, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided vert count range. Includes static mesh components from child actors; meshes without built LOD0 render data are skipped.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinVertCount | `int32` | The minimum vert count to filter by. |
| MaxVertCount | `int32` | The maximum vert count to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided vert count range. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Tri Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByTriCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinTriCount, int32 MaxTriCount, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided triangle count range. Includes static mesh components from child actors; meshes without built LOD0 render data are skipped.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinTriCount | `int32` | The minimum triangle count to filter by. |
| MaxTriCount | `int32` | The maximum triangle count to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided triangle count range. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Bounds
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided actor bounds.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinBounds | `FVector` | The minimum bounds to filter by. |
| MaxBounds | `FVector` | The maximum bounds to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided bounds. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Static Mesh Bounds
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByStaticMeshBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided static mesh bounds.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinBounds | `FVector` | The minimum bounds to filter by. |
| MaxBounds | `FVector` | The maximum bounds to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided bounds. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By World Location
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByWorldLocation(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& WorldLocation, float Radius, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided world location and radius.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| WorldLocation | `FVector` | The world location to filter by. |
| Radius | `float` | The radius to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided world location and radius. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By LOD Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByLODCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLODs, int32 MaxLODs, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided LOD (Level of Detail) count.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinLODs | `int32` | The minimum LOD count to filter by. |
| MaxLODs | `int32` | The maximum LOD count to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided LOD count. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Nanite State
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByNaniteState(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, bool bNaniteEnabled, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided Nanite state.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| bNaniteEnabled | `bool` | Whether to filter by Nanite enabled or disabled. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided Nanite state. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Lightmap Resolution
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByLightmapResolution(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLightmapResolution, int32 MaxLightmapResolution, EDirectiveUtilSearchLocation SearchLocation, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided Lightmap Resolution. The actor-override check only applies to components with `bOverrideLightMapRes` enabled.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| MinLightmapResolution | `int32` | The minimum lightmap resolution to filter by. |
| MaxLightmapResolution | `int32` | The maximum lightmap resolution to filter by. |
| SearchLocation | `EDirectiveUtilSearchLocation` | The location to search from (Actor Override and/or Static Mesh). |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided lightmap resolution. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Mobility
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMobility(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EComponentMobility::Type Mobility, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided mobility. Tests the root component's mobility, matching the actor mobility shown in the editor UI.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Mobility | `EComponentMobility::Type` | The mobility to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided mobility. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Collision Channel
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByCollisionChannel(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided collision channel.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| CollisionChannel | `ECollisionChannel` | The collision type to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided collision type. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Collision Response
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByCollisionResponse(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided collision response.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| CollisionChannel | `ECollisionChannel` | The collision channel to filter by. |
| CollisionResponse | `ECollisionResponse` | The collision response to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided collision response. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Collision Enabled
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByCollisionEnabled(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionEnabled::Type CollisionEnabled, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided collision-enabled state.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| CollisionEnabled | `ECollisionEnabled::Type` | The collision state to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided collision-enabled state. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Collision Profile
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByCollisionProfile(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FName CollisionProfile, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided collision profile.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| CollisionProfile | `FName` | The collision profile to filter by. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided collision profile. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Texture Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByTextureName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FString TextureName, EDirectiveUtilSearchLocation Source, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided texture name.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| TextureName | `FString` | The texture name to filter by. |
| Source | `EDirectiveUtilSearchLocation` | Choose between searching through material overrides or the base material. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided texture name. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Texture
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByTexture(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSoftObjectPtr<UTexture2D> TextureReference, EDirectiveUtilSearchLocation Source, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on the provided texture reference.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| TextureReference | `TSoftObjectPtr<UTexture2D>` | The texture reference to filter by. |
| Source | `EDirectiveUtilSearchLocation` | Choose between searching through material overrides or the base material. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with the provided texture reference. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Empty Actors
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterEmptyActors(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filters the provided actors based on whether the actor is empty or not. An actor is empty when it has no components, or its only component is a plain scene component with no children; Include and Exclude return exact complements.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude empty actors. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Missing Materials
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMissingMaterials(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilSearchLocation Location, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on missing materials.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Location | `EDirectiveUtilSearchLocation` | The location to search for missing materials. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with missing materials. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Missing Static Meshes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMissingStaticMeshes(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on missing Static Meshes.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with missing Static Meshes. |

**Returns:** FilteredActors: the list of actors that have been filtered.

## Filter Actors By Missing Textures
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Filters|Actor`

```cpp
static void FilterActorsByMissingTextures(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilSearchLocation Location, EDirectiveUtilInclusivity Inclusivity = Include);
```

Filter the provided actors based on missing textures.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actors | `TArray<AActor*>` | The list of actors to filter. |
| Location | `EDirectiveUtilSearchLocation` | The location to search for missing textures. |
| Inclusivity | `EDirectiveUtilInclusivity` | Whether to include or exclude actors with missing textures. |

**Returns:** FilteredActors: the list of actors that have been filtered.

---

## Bounds Calculation

## Is Actor Within Box Bounds
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities`

```cpp
static bool IsActorWithinBoxBounds(AActor* Actor, UBoxComponent* BoxComponent);
```

Check if an actor is within the bounds of a box.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actor | `AActor*` | The actor to check. |
| BoxComponent | `UBoxComponent*` | The box component to check. |

**Returns:** True if the actor is within the box bounds.

## Is Actor Within Sphere Bounds
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities`

```cpp
static bool IsActorWithinSphereBounds(AActor* Actor, USphereComponent* SphereComponent);
```

Check if an actor is within the bounds of a sphere.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actor | `AActor*` | The actor to check. |
| SphereComponent | `USphereComponent*` | The sphere component to check. |

**Returns:** True if the actor is within the sphere bounds.

## Is Actor Within Capsule Bounds
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities`

```cpp
static bool IsActorWithinCapsuleBounds(AActor* Actor, UCapsuleComponent* CapsuleComponent);
```

Check if an actor is within the bounds of a capsule.

| Parameter | Type | Description |
|-----------|------|-------------|
| Actor | `AActor*` | The actor to check. |
| CapsuleComponent | `UCapsuleComponent*` | The capsule component to check. |

**Returns:** True if the actor is within the capsule bounds.

---

## Getters

## Get Actors By Class
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByClass(TArray<AActor*>& FoundActors, TSubclassOf<AActor> ActorClass, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors within the current level based on the provided class and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| ActorClass | `TSubclassOf<AActor>` | The class of the actors to select. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByName(TArray<AActor*>& FoundActors, FString ActorName, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided asset name and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| ActorName | `FString` | The name of the actors to select. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Material
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByMaterial(TArray<AActor*>& FoundActors, const UMaterialInterface* Material, EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided material reference and options. Only returns actors that have a static mesh component.

| Parameter | Type | Description |
|-----------|------|-------------|
| Material | `const UMaterialInterface*` | The reference of the material to search by. |
| MaterialSource | `EDirectiveUtilSearchLocation` | The source of the material to search by. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Material Soft Reference
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByMaterialSoftReference(TArray<AActor*>& FoundActors, const TSoftObjectPtr<UMaterialInterface> Material, EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided material soft reference and options. Only returns actors that have a static mesh component.

| Parameter | Type | Description |
|-----------|------|-------------|
| Material | `TSoftObjectPtr<UMaterialInterface>` | The reference of the material to search by. |
| MaterialSource | `EDirectiveUtilSearchLocation` | The source of the material to search by. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Material Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByMaterialName(TArray<AActor*>& FoundActors, FString MaterialName, EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided material name and options. Only returns actors that have a static mesh component.

| Parameter | Type | Description |
|-----------|------|-------------|
| MaterialName | `FString` | The name of the material to search by. |
| MaterialSource | `EDirectiveUtilSearchLocation` | The source of the material to search by. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Vertex Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByVertexCount(TArray<AActor*>& FoundActors, int32 From, int32 To, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided vert count and options. Only returns actors that have a Static Mesh Component (including from child actors); meshes without built LOD0 render data are skipped.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `int32` | The minimum number of vertices to search for. |
| To | `int32` | The maximum number of vertices to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Tri Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByTriCount(TArray<AActor*>& FoundActors, int32 From, int32 To, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided triangle count and options. Only returns actors that have a Static Mesh Component (including from child actors); meshes without built LOD0 render data are skipped.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `int32` | The minimum number of vertices to search for. |
| To | `int32` | The maximum number of vertices to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Bounding Box
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByBoundingBox(TArray<AActor*>& FoundActors, FVector Min, FVector Max, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided bounding box and options. Matches actors whose world-space bounding box lies entirely within the box defined by Min/Max (inclusive).

| Parameter | Type | Description |
|-----------|------|-------------|
| Min | `FVector` | The minimum point of the bounding box. |
| Max | `FVector` | The maximum point of the bounding box. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Mesh Size
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByMeshSize(TArray<AActor*>& FoundActors, float From, float To, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided mesh size and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `float` | The minimum size of the mesh to search for. |
| To | `float` | The maximum size of the mesh to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By World Location
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByWorldLocation(TArray<AActor*>& FoundActors, FVector WorldLocation, float Radius = 1000.f, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided world location, radius, and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| WorldLocation | `FVector` | The world location to search by. |
| Radius | `float` | The radius around the provided world location to search by. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By LOD Count
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByLODCount(TArray<AActor*>& FoundActors, int32 From = 0, int32 To = 7, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided LOD count and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `int32` | The minimum number of LODs to search for. |
| To | `int32` | The maximum number of LODs to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Nanite Enabled
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByNaniteEnabled(TArray<AActor*>& FoundActors, bool bNaniteEnabled, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on whether they have Nanite enabled or not. Only returns actors that have a Static Mesh Component.

| Parameter | Type | Description |
|-----------|------|-------------|
| bNaniteEnabled | `bool` | Enable to find Actors with Static Mesh Components that have Nanite enabled. Disable to find Actors with Static Mesh Components that do not have Nanite enabled. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Lightmap Resolution
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByLightmapResolution(TArray<AActor*>& FoundActors, int32 From = 4, int32 To = 4096, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided Lightmap Resolution and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| From | `int32` | The minimum lightmap resolution to search for. |
| To | `int32` | The maximum lightmap resolution to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Mobility
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByMobility(TArray<AActor*>& FoundActors, EComponentMobility::Type Mobility, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided mobility and options. Tests the root component's mobility, matching the actor mobility shown in the editor UI.

| Parameter | Type | Description |
|-----------|------|-------------|
| Mobility | `EComponentMobility::Type` | The mobility to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Static Mesh
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByStaticMesh(TArray<AActor*>& FoundActors, UStaticMesh* StaticMesh, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided Static Mesh reference and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| StaticMesh | `UStaticMesh*` | The Static Mesh to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Static Mesh Soft Reference
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByStaticMeshSoftReference(TArray<AActor*>& FoundActors, TSoftObjectPtr<UStaticMesh> StaticMesh, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided Static Mesh soft reference and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| StaticMesh | `TSoftObjectPtr<UStaticMesh>` | The Static Mesh Soft Reference to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Static Mesh Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByStaticMeshName(TArray<AActor*>& FoundActors, FString StaticMeshName, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided Static Mesh name and options. Uses a case-insensitive substring match against the mesh asset name.

| Parameter | Type | Description |
|-----------|------|-------------|
| StaticMeshName | `FString` | The Static Mesh name to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Texture
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByTexture(TArray<AActor*>& FoundActors, UTexture2D* Texture, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided texture reference and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| Texture | `UTexture2D*` | The texture to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Texture Soft Reference
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByTextureSoftReference(TArray<AActor*>& FoundActors, TSoftObjectPtr<UTexture2D> Texture, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided texture soft reference and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| Texture | `TSoftObjectPtr<UTexture2D>` | The texture soft reference to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Actors By Texture Name
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

```cpp
void GetActorsByTextureName(TArray<AActor*>& FoundActors, FString TextureName, EDirectiveUtilSelectionMethod SelectionMethod = World, EDirectiveUtilInclusivity Inclusivity = Include);
```

Returns a list of actors based on the provided texture name and options.

| Parameter | Type | Description |
|-----------|------|-------------|
| TextureName | `FString` | The texture name to search for. |
| SelectionMethod | `EDirectiveUtilSelectionMethod` | The selection method to use. |
| Inclusivity | `EDirectiveUtilInclusivity` | Should the search be inclusive or exclusive? |

**Returns:** FoundActors: the list of actors that were found.

## Get Invalid Actors
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Select`

Deprecated. Unreal's editor actor enumeration excludes invalid and pending-kill actors, so this node always returns an empty array.

```cpp
void GetInvalidActors(TArray<AActor*>& FoundActors);
```

**Returns:** FoundActors: the list of actors that were found.

---

## Static Mesh

## Push Override Materials To Source
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Static Mesh`

```cpp
static void PushOverrideMaterialsToSource(UStaticMeshComponent* StaticMeshComponent);
```

Pushes the overriden materials on the provided Static Mesh Component to the source Static Mesh.

| Parameter | Type | Description |
|-----------|------|-------------|
| StaticMeshComponent | `UStaticMeshComponent*` | The Static Mesh Component to push the materials from. |
