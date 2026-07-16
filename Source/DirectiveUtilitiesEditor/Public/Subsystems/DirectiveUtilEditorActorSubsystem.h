// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/EngineTypes.h"
#include "Types/DirectiveUtilEditorTypes.h"
#include "DirectiveUtilEditorActorSubsystem.generated.h"

class UCapsuleComponent;
class UBoxComponent;
class USphereComponent;
class AStaticMeshActor;

/**
 * DirectiveUtilEditorActorSubsystem
 *
 * Blueprint helpers for querying and filtering actors in the editor world.
 */
UCLASS()
class DIRECTIVEUTILITIESEDITOR_API UDirectiveUtilEditorActorSubsystem : public UEditorActorSubsystem
{
	GENERATED_BODY()

public:

	//-----------------------------
	// Utilities
	//-----------------------------

	/**
	 * Focus actors in viewport.
	 * @param Actors The actors to focus.
	 * @param bInstant Enable to focus the actors instantly instead of smoothly animating.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor")
	static void FocusActorsInViewport(const TArray<AActor*> Actors, bool bInstant = false);

	/**
	 * Get all unique classes used in the level.
	 * @result Classes The list of classes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor")
	TArray<UClass*> GetAllLevelClasses();

	/** Aligns actor bounds to the combined minimum, center, or maximum on one axis. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Actor Layout")
	static FDirectiveUtilActorOperationResult AlignActors(
		const TArray<AActor*>& Actors,
		EDirectiveUtilActorLayoutAxis Axis,
		EDirectiveUtilActorAlignment Alignment);

	/** Distributes actors between the outer actors using equal center spacing or equal bounds gaps. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Actor Layout")
	static FDirectiveUtilActorOperationResult DistributeActors(
		const TArray<AActor*>& Actors,
		EDirectiveUtilActorLayoutAxis Axis,
		EDirectiveUtilActorDistribution Distribution);

	/** Traces from each actor and places its pivot or directional bounds on the hit surface. */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Editor|Actor Layout", meta = (AdvancedDisplay = "bAlignToNormal"))
	static FDirectiveUtilActorOperationResult SnapActorsToSurface(
		const TArray<AActor*>& Actors,
		FVector TraceDirection,
		float MaximumDistance,
		TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility,
		EDirectiveUtilSurfacePlacement Placement = EDirectiveUtilSurfacePlacement::Bounds,
		bool bAlignToNormal = false);

	//-----------------------------
	// Filters
	//-----------------------------
	// Output arrays are appended to and deduplicated (existing entries are preserved).
	// Passing the same array as input and output filters it in place.

	/**
	 * Returns only the Static Mesh Actors from the provided Actor List.
	 * @param ActorsToFilter The list of Actors to filter.
	 * @param OutStaticMeshActors The list of Actors that are Static Mesh Actors.
	 */
	virtual void FilterStaticMeshActors(TArray<AStaticMeshActor*>& OutStaticMeshActors, TArray<AActor*> ActorsToFilter) const;

	/**
	 * Filters the provided actors based on the provided name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param ActorName The name to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& ActorName, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided class.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param ActorClass The class to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided class.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByClass(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSubclassOf<AActor> ActorClass, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided tags.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Tag The tag to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByTag(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FName Tag, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided material name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MaterialName The material name to filter by.
	 * @param MaterialSource The location to check for the material.
	 * @param Inclusivity Whether to include or exclude actors with the provided material name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMaterialName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& MaterialName, EDirectiveUtilSearchLocation MaterialSource, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided material reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Material The material reference to filter by.
	 * @param MaterialSource The location to check for the material.
	 * @param Inclusivity Whether to include or exclude actors with the provided material reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMaterial(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UMaterialInterface>& Material, EDirectiveUtilSearchLocation MaterialSource, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh name.
	 * Uses a case-insensitive substring match against the mesh asset name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param StaticMeshName The static mesh name to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided static mesh name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByStaticMeshName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& StaticMeshName, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param StaticMesh The static mesh reference to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided static mesh reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByStaticMesh(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UStaticMesh>& StaticMesh, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided vert count range.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinVertCount The minimum vert count to filter by.
	 * @param MaxVertCount The maximum vert count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided vert count range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByVertCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinVertCount, int32 MaxVertCount, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided triangle count range.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinTriCount The minimum triangle count to filter by.
	 * @param MaxTriCount The maximum triangle count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided triangle count range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByTriCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinTriCount, int32 MaxTriCount, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided actor bounds.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinBounds The minimum bounds to filter by.
	 * @param MaxBounds The maximum bounds to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided bounds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh bounds.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinBounds The minimum bounds to filter by.
	 * @param MaxBounds The maximum bounds to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided bounds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByStaticMeshBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided world location and radius.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param WorldLocation The world location to filter by.
	 * @param Radius The radius to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided world location and radius.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByWorldLocation(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& WorldLocation, float Radius, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided LOD (Level of Detail) count.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinLODs The minimum LOD count to filter by.
	 * @param MaxLODs The maximum LOD count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided LOD count.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByLODCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLODs, int32 MaxLODs, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Nanite state.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param bNaniteEnabled Whether to filter by Nanite enabled or disabled.
	 * @param Inclusivity Whether to include or exclude actors with the provided Nanite state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByNaniteState(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, bool bNaniteEnabled, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Lightmap Resolution.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinLightmapResolution The minimum lightmap resolution to filter by.
	 * @param MaxLightmapResolution The maximum lightmap resolution to filter by.
	 * @param SearchLocation The location to search from (Actor Override and/or Static Mesh).
	 * @param Inclusivity Whether to include or exclude actors with the provided lightmap resolution.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByLightmapResolution(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLightmapResolution, int32 MaxLightmapResolution, EDirectiveUtilSearchLocation SearchLocation, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided mobility.
	 * Tests the root component's mobility, matching the actor mobility shown in the editor UI.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Mobility The mobility to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided mobility.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMobility(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EComponentMobility::Type Mobility, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision channel.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionChannel The collision type to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision type.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByCollisionChannel(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision response.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionChannel The collision channel to filter by.
	 * @param CollisionResponse The collision response to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByCollisionResponse(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision-enabled state.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionEnabled The collision state to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision-enabled state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByCollisionEnabled(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionEnabled::Type CollisionEnabled, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision profile.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionProfile The collision profile to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision profile.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByCollisionProfile(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FName CollisionProfile, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Texture Name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param TextureName The texture name to filter by.
	 * @param Source Chose between searching through material overrides or the base material.
	 * @param Inclusivity Whether to include or exclude actors with the provided texture name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByTextureName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FString TextureName, EDirectiveUtilSearchLocation Source, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Texture Reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param TextureReference The texture reference to filter by.
	 * @param Source Chose between searching through material overrides or the base material.
	 * @param Inclusivity Whether to include or exclude actors with the provided texture reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByTexture(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSoftObjectPtr<UTexture2D> TextureReference, EDirectiveUtilSearchLocation Source, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filters the provided actors based on if the actor is empty or not.
	 * An actor is empty when it has no components, or its only component is a
	 * plain scene component with no child components.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Inclusivity Whether to include or exclude empty actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterEmptyActors(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing materials.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Location The location to search for missing materials.
	 * @param Inclusivity Whether to include or exclude actors with missing materials.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMissingMaterials(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilSearchLocation Location, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing Static Meshes.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Inclusivity Whether to include or exclude actors with missing Static Meshes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMissingStaticMeshes(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing textures.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Location The location to search for missing textures.
	 * @param Inclusivity Whether to include or exclude actors with missing textures.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Filters|Actor")
	static void FilterActorsByMissingTextures(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EDirectiveUtilSearchLocation Location, EDirectiveUtilInclusivity Inclusivity = Include);


	//-----------------------------
	// Bounds Calculation
	//-----------------------------

	/**
	 * Check if an actor is within the bounds of a box.
	 * @param Actor The actor to check.
	 * @param BoxComponent The box component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities")
	static bool IsActorWithinBoxBounds(AActor* Actor, UBoxComponent* BoxComponent);

	/**
	 * Check if an actor is within the bounds of a Sphere.
	 * @param Actor The actor to check.
	 * @param SphereComponent The sphere component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities")
	static bool IsActorWithinSphereBounds(AActor* Actor, USphereComponent* SphereComponent);

	/**
	 * Check if an actor is within the bounds of a capsule.
	 * @param Actor The actor to check.
	 * @param CapsuleComponent The capsule component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities")
	static bool IsActorWithinCapsuleBounds(AActor* Actor, UCapsuleComponent* CapsuleComponent);

	//-----------------------------
	// Getters
	//-----------------------------

	/**
	 * Returns a list of actors based on the provided class and options.
	 * Get actors within the current level by their class.
	 * @param FoundActors The list of actors that were found.
	 * @param ActorClass The class of the actors to select.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByClass(
		TArray<AActor*>& FoundActors,
		TSubclassOf<AActor> ActorClass,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided asset name and options.
	 * @param FoundActors The list of actors that were found.
	 * @param ActorName The name of the actors to select.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByName(
		TArray<AActor*>& FoundActors,
		FString ActorName,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided material reference and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param Material The reference of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterial(
		TArray<AActor*>& FoundActors,
		const UMaterialInterface* Material,
		EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided material reference and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param Material The reference of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterialSoftReference(
		TArray<AActor*>& FoundActors,
		const TSoftObjectPtr<UMaterialInterface> Material,
		EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided material name and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param MaterialName The name of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterialName(
		TArray<AActor*>& FoundActors,
		FString MaterialName,
		EDirectiveUtilSearchLocation MaterialSource = BaseAndOverride,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	* Returns a list of actors based on the provided vert count and options.
	* Note: This will only return actors that have a Static Mesh Component.
	* @param FoundActors The list of actors that were found.
	* @param From The minimum number of vertices to search for.
	* @param To	The maximum number of vertices to search for.
	* @param SelectionMethod The selection method to use.
	* @param Inclusivity Should the search be inclusive or exclusive?
	*/
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByVertexCount(
		TArray<AActor*>& FoundActors,
		int32 From,
		int32 To,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	* Returns a list of actors based on the provided triangle count and options.
	* Note: This will only return actors that have a Static Mesh Component.
	* @param FoundActors The list of actors that were found.
	* @param From The minimum number of vertices to search for.
	* @param To	The maximum number of vertices to search for.
	* @param SelectionMethod The selection method to use.
	* @param Inclusivity Should the search be inclusive or exclusive?
	*/
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByTriCount(
		TArray<AActor*>& FoundActors,
		int32 From,
		int32 To,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided bounding box and options.
	 * Matches actors whose world-space bounding box lies entirely within the box defined by Min/Max (inclusive).
	 * @param FoundActors The list of actors that were found.
	 * @param Min The minimum point of the bounding box.
	 * @param Max The maximum point of the bounding box.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByBoundingBox(
		TArray<AActor*>& FoundActors,
		FVector Min,
		FVector Max,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided mesh size and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum size of the mesh to search for.
	 * @param To The maximum size of the mesh to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByMeshSize(
		TArray<AActor*>& FoundActors,
		float From,
		float To,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided world location, radius, and options.
	 * @param FoundActors The list of actors that were found.
	 * @param WorldLocation The world location to search by.
	 * @param Radius The radius around the provided world location to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByWorldLocation(
		TArray<AActor*>& FoundActors,
		FVector WorldLocation,
		float Radius = 1000.f,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided LOD count and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum number of LODs to search for.
	 * @param To The maximum number of LODs to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByLODCount(
		TArray<AActor*>& FoundActors,
		int32 From = 0,
		int32 To = 7,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on if they have Nanite enabled or not.
	 * Note: This will only return actors that have a Static Mesh Component.
	 * @param FoundActors The list of actors that were found.
	 * @param bNaniteEnabled Enable to find Actors with Static Mesh Components that have Nanite enabled. Disable to find Actors with Static Mesh Components that do not have Nanite enabled.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByNaniteEnabled(
		TArray<AActor*>& FoundActors,
		bool bNaniteEnabled,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Lightmap Resolution and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum lightmap resolution to search for.
	 * @param To The maximum lightmap resolution to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=3))
	void GetActorsByLightmapResolution(
		TArray<AActor*>& FoundActors,
		int32 From = 4,
		int32 To = 4096,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided mobility and options.
	 * Tests the root component's mobility, matching the actor mobility shown in the editor UI.
	 * @param FoundActors The list of actors that were found.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 * @param Mobility The mobility to search for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByMobility(
		TArray<AActor*>& FoundActors,
		EComponentMobility::Type Mobility,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMesh The Static Mesh to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMesh(
		TArray<AActor*>& FoundActors,
		UStaticMesh* StaticMesh,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh soft reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMesh The Static Mesh Soft Reference to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMeshSoftReference(
		TArray<AActor*>& FoundActors,
		TSoftObjectPtr<UStaticMesh> StaticMesh,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh name and options.
	 * Uses a case-insensitive substring match against the mesh asset name.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMeshName The Static Mesh name to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMeshName(
		TArray<AActor*>& FoundActors,
		FString StaticMeshName,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param Texture The texture to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTexture(
		TArray<AActor*>& FoundActors,
		UTexture2D* Texture,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture soft reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param Texture The texture soft reference to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTextureSoftReference(
		TArray<AActor*>& FoundActors,
		TSoftObjectPtr<UTexture2D> Texture,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture name and options.
	 * @param FoundActors The list of actors that were found.
	 * @param TextureName The texture name to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTextureName(
		TArray<AActor*>& FoundActors,
		FString TextureName,
		EDirectiveUtilSelectionMethod SelectionMethod = World,
		EDirectiveUtilInclusivity Inclusivity = Include);

	/**
	 * Returns a list of invalid actors.
	 * @param FoundActors The list of actors that were found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Select", meta=(AdvancedDisplay=1))
	void GetInvalidActors(TArray<AActor*>& FoundActors);

	//-----------------------------
	// Static Mesh
	//-----------------------------

	/**
	 * Pushes the overriden materials on the provided Static Mesh Component to the source Static Mesh.
	 * @param StaticMeshComponent The Static Mesh Component to push the materials from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Static Mesh")
	static void PushOverrideMaterialsToSource(UStaticMeshComponent* StaticMeshComponent);
};
