// Copyright Unreal Directive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/EngineTypes.h"
#include "UDCoreEditorTypes.h"
#include "UDCoreEditorActorSubsystem.generated.h"

class UCapsuleComponent;
class UBoxComponent;
class USphereComponent;
class UStaticMeshActor;

/**
 * UDCoreEditorActorSubsystem
 *
 * An enhanced version of the Editor Actor Subsystem, provided by UDCore.
 */
UCLASS()
class UDCOREEDITOR_API UUDCoreEditorActorSubsystem : public UEditorActorSubsystem
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
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Editor")
	static void FocusActorsInViewport(const TArray<AActor*> Actors, bool bInstant = false);

	/**
	 * Get all unique classes used in the level.
	 * @result Classes The list of classes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Editor")
	TArray<UClass*> GetAllLevelClasses();
	
	//-----------------------------
	// Filters
	//-----------------------------

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
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& ActorName, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided class.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param ActorClass The class to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided class.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByClass(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSubclassOf<AActor> ActorClass, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided tags.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Tag The tag to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByTag(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FName Tag, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided material name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MaterialName The material name to filter by.
	 * @param MaterialSource The location to check for the material.
	 * @param Inclusivity Whether to include or exclude actors with the provided material name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMaterialName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& MaterialName, EUDSearchLocation MaterialSource, EUDInclusivity Inclusivity = Include);
	
	/**
	 * Filter the provided actors based on the provided material reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Material The material reference to filter by.
	 * @param MaterialSource The location to check for the material.
	 * @param Inclusivity Whether to include or exclude actors with the provided material reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMaterial(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UMaterialInterface>& Material, EUDSearchLocation MaterialSource, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param StaticMeshName The static mesh name to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided static mesh name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByStaticMeshName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FString& StaticMeshName, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param StaticMesh The static mesh reference to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided static mesh reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByStaticMesh(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const TSoftObjectPtr<UStaticMesh>& StaticMesh, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided vert count range.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinVertCount The minimum vert count to filter by.
	 * @param MaxVertCount The maximum vert count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided vert count range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByVertCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinVertCount, int32 MaxVertCount, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided triangle count range.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinTriCount The minimum triangle count to filter by.
	 * @param MaxTriCount The maximum triangle count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided triangle count range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByTriCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinTriCount, int32 MaxTriCount, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided actor bounds.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinBounds The minimum bounds to filter by.
	 * @param MaxBounds The maximum bounds to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided bounds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided static mesh bounds.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinBounds The minimum bounds to filter by.
	 * @param MaxBounds The maximum bounds to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided bounds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByStaticMeshBounds(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& MinBounds, const FVector& MaxBounds, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided world location and radius.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param WorldLocation The world location to filter by.
	 * @param Radius The radius to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided world location and radius.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByWorldLocation(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, const FVector& WorldLocation, float Radius, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided LOD (Level of Detail) count.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinLODs The minimum LOD count to filter by.
	 * @param MaxLODs The maximum LOD count to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided LOD count.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByLODCount(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLODs, int32 MaxLODs, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Nanite state.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param bNaniteEnabled Whether to filter by Nanite enabled or disabled.
	 * @param Inclusivity Whether to include or exclude actors with the provided Nanite state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByNaniteState(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, bool bNaniteEnabled, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Lightmap Resolution.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param MinLightmapResolution The minimum lightmap resolution to filter by.
	 * @param MaxLightmapResolution The maximum lightmap resolution to filter by.
	 * @param SearchLocation The location to search from (Actor Override and/or Static Mesh).
	 * @param Inclusivity Whether to include or exclude actors with the provided lightmap resolution.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByLightmapResolution(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, int32 MinLightmapResolution, int32 MaxLightmapResolution, EUDSearchLocation SearchLocation, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided mobility.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Mobility The mobility to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided mobility.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMobility(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EComponentMobility::Type Mobility, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision channel.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionChannel The collision type to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision type.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByCollisionChannel(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision response.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionChannel The collision channel to filter by.
	 * @param CollisionResponse The collision response to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision response.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByCollisionResponse(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision-enabled state.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionEnabled The collision state to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision-enabled state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByCollisionEnabled(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, ECollisionEnabled::Type CollisionEnabled, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided collision profile.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param CollisionProfile The collision profile to filter by.
	 * @param Inclusivity Whether to include or exclude actors with the provided collision profile.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByCollisionProfile(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FName CollisionProfile, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Texture Name.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param TextureName The texture name to filter by.
	 * @param Source Chose between searching through material overrides or the base material.
	 * @param Inclusivity Whether to include or exclude actors with the provided texture name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByTextureName(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, FString TextureName, EUDSearchLocation Source, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on the provided Texture Reference.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param TextureReference The texture reference to filter by.
	 * @param Source Chose between searching through material overrides or the base material.
	 * @param Inclusivity Whether to include or exclude actors with the provided texture reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByTexture(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, TSoftObjectPtr<UTexture2D> TextureReference, EUDSearchLocation Source, EUDInclusivity Inclusivity = Include);

	/**
	 * Filters the provided actors based on if the actor is empty or not.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Inclusivity Whether to include or exclude empty actors.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterEmptyActors(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing materials.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Location The location to search for missing materials.
	 * @param Inclusivity Whether to include or exclude actors with missing materials.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMissingMaterials(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EUDSearchLocation Location, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing Static Meshes.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Inclusivity Whether to include or exclude actors with missing Static Meshes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMissingStaticMeshes(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EUDInclusivity Inclusivity = Include);

	/**
	 * Filter the provided actors based on missing textures.
	 * @param Actors The list of actors to filter.
	 * @param FilteredActors The list of actors that have been filtered.
	 * @param Location The location to search for missing textures.
	 * @param Inclusivity Whether to include or exclude actors with missing textures.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Filters|Actor")
	static void FilterActorsByMissingTextures(const TArray<AActor*>& Actors, TArray<AActor*>& FilteredActors, EUDSearchLocation Location, EUDInclusivity Inclusivity = Include);
	
	
	//-----------------------------
	// Bounds Calculation
	//-----------------------------

	/**
	 * Check if an actor is within the bounds of a box.
	 * @param Actor The actor to check.
	 * @param BoxComponent The box component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit")
	static bool IsActorWithinBoxBounds(AActor* Actor, UBoxComponent* BoxComponent);

	/**
	 * Check if an actor is within the bounds of a Sphere.
	 * @param Actor The actor to check.
	 * @param SphereComponent The sphere component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit")
	static bool IsActorWithinSphereBounds(AActor* Actor, USphereComponent* SphereComponent);

	/**
	 * Check if an actor is within the bounds of a capsule.
	 * @param Actor The actor to check.
	 * @param CapsuleComponent The capsule component to check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit")
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
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByClass(
		TArray<AActor*>& FoundActors,
		TSubclassOf<AActor> ActorClass,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided asset name and options.
	 * @param FoundActors The list of actors that were found.
	 * @param ActorName The name of the actors to select.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByName(
		TArray<AActor*>& FoundActors,
		FString ActorName,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided material reference and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param Material The reference of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterial(
		TArray<AActor*>& FoundActors,
		const UMaterialInterface* Material,
		EUDSearchLocation MaterialSource = BaseAndOverride,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided material reference and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param Material The reference of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterialSoftReference(
		TArray<AActor*>& FoundActors,
		const TSoftObjectPtr<UMaterialInterface> Material,
		EUDSearchLocation MaterialSource = BaseAndOverride,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);
	
	/**
	 * Returns a list of actors based on the provided material name and options.
	 * Note: This will only return actors that have a static mesh component.
	 * @param MaterialName The name of the material to search by.
	 * @param FoundActors The list of actors that were found.
	 * @param MaterialSource The source of the material to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=4))
	void GetActorsByMaterialName(
		TArray<AActor*>& FoundActors,
		FString MaterialName,
		EUDSearchLocation MaterialSource = BaseAndOverride,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	* Returns a list of actors based on the provided vert count and options.
	* Note: This will only return actors that have a Static Mesh Component.
	* @param FoundActors The list of actors that were found.
	* @param From The minimum number of vertices to search for.
	* @param To	The maximum number of vertices to search for.
	* @param SelectionMethod The selection method to use.
	* @param Inclusivity Should the search be inclusive or exclusive?
	*/
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByVertexCount(
		TArray<AActor*>& FoundActors,
		int32 From,
		int32 To,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	* Returns a list of actors based on the provided triangle count and options.
	* Note: This will only return actors that have a Static Mesh Component.
	* @param FoundActors The list of actors that were found.
	* @param From The minimum number of vertices to search for.
	* @param To	The maximum number of vertices to search for.
	* @param SelectionMethod The selection method to use.
	* @param Inclusivity Should the search be inclusive or exclusive?
	*/
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByTriCount(
		TArray<AActor*>& FoundActors,
		int32 From,
		int32 To,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided bounding box and options.
	 * @param FoundActors The list of actors that were found.
	 * @param Min The minimum point of the bounding box.
	 * @param Max The maximum point of the bounding box.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByBoundingBox(
		TArray<AActor*>& FoundActors,
		FVector Min,
		FVector Max,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided mesh size and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum size of the mesh to search for.
	 * @param To The maximum size of the mesh to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByMeshSize(
		TArray<AActor*>& FoundActors,
		float From,
		float To,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided world location, radius, and options.
	 * @param FoundActors The list of actors that were found.
	 * @param WorldLocation The world location to search by.
	 * @param Radius The radius around the provided world location to search by.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByWorldLocation(
		TArray<AActor*>& FoundActors,
		FVector WorldLocation,
		float Radius = 1000.f,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided LOD count and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum number of LODs to search for.
	 * @param To The maximum number of LODs to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByLODCount(
		TArray<AActor*>& FoundActors,
		int32 From = 0,
		int32 To = 7,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on if they have Nanite enabled or not.
	 * Note: This will only return actors that have a Static Mesh Component.
	 * @param FoundActors The list of actors that were found.
	 * @param bNaniteEnabled Enable to find Actors with Static Mesh Components that have Nanite enabled. Disable to find Actors with Static Mesh Components that do not have Nanite enabled.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByNaniteEnabled(
		TArray<AActor*>& FoundActors,
		bool bNaniteEnabled,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Lightmap Resolution and options.
	 * @param FoundActors The list of actors that were found.
	 * @param From The minimum lightmap resolution to search for.
	 * @param To The maximum lightmap resolution to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=3))
	void GetActorsByLightmapResolution(
		TArray<AActor*>& FoundActors,
		int32 From = 4,
		int32 To = 4096,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided mobility and options.
	 * @param FoundActors The list of actors that were found.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 * @param Mobility The mobility to search for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByMobility(
		TArray<AActor*>& FoundActors,
		EComponentMobility::Type Mobility,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMesh The Static Mesh to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMesh(
		TArray<AActor*>& FoundActors,
		UStaticMesh* StaticMesh,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh soft reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMesh The Static Mesh Soft Reference to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMeshSoftReference(
		TArray<AActor*>& FoundActors,
		TSoftObjectPtr<UStaticMesh> StaticMesh,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided Static Mesh name and options.
	 * @param FoundActors The list of actors that were found.
	 * @param StaticMeshName The Static Mesh name to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByStaticMeshName(
		TArray<AActor*>& FoundActors,
		FString StaticMeshName,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param Texture The texture to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTexture(
		TArray<AActor*>& FoundActors,
		UTexture2D* Texture,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture soft reference and options.
	 * @param FoundActors The list of actors that were found.
	 * @param Texture The texture soft reference to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTextureSoftReference(
		TArray<AActor*>& FoundActors,
		TSoftObjectPtr<UTexture2D> Texture,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of actors based on the provided texture name and options.
	 * @param FoundActors The list of actors that were found.
	 * @param TextureName The texture name to search for.
	 * @param SelectionMethod The selection method to use.
	 * @param Inclusivity Should the search be inclusive or exclusive?
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=2))
	void GetActorsByTextureName(
		TArray<AActor*>& FoundActors,
		FString TextureName,
		EUDSelectionMethod SelectionMethod = World,
		EUDInclusivity Inclusivity = Include);

	/**
	 * Returns a list of invalid actors.
	 * @param FoundActors The list of actors that were found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Select", meta=(AdvancedDisplay=1))
	void GetInvalidActors(TArray<AActor*>& FoundActors);

	//-----------------------------
	// Static Mesh
	//-----------------------------

	/**
	 * Pushes the overriden materials on the provided Static Mesh Component to the source Static Mesh.
	 * @param StaticMeshComponent The Static Mesh Component to push the materials from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Directive Toolkit|Static Mesh")
	static void PushOverrideMaterialsToSource(UStaticMeshComponent* StaticMeshComponent);

	
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 5
	// These functions were added for Unreal Engine 5.5 to solve a code regression.
	// For more information, see https://github.com/UnrealDirective/UDCore/issues/6
	// TODO: Remove these once UE 5.5 is no longer supported.
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;


protected:
	/** To fire before an Actor is Dropped */
	void BroadcastEditNewActorsDropped(const TArray<UObject*>& DroppedObjects, const TArray<AActor*>& DroppedActors) const;

	/** To fire before an Actor is Placed */
	void BroadcastEditNewActorsPlaced(UObject* ObjToUse, const TArray<AActor*>& PlacedActors) const;

	/** To fire before an Actor is Cut */
	void BroadcastEditCutActorsBegin() const;

	/** To fire after an Actor is Cut */
	void BroadcastEditCutActorsEnd() const;

	/** To fire before an Actor is Copied */
	void BroadcastEditCopyActorsBegin() const;

	/** To fire after an Actor is Copied */
	void BroadcastEditCopyActorsEnd() const;

	/** To fire before an Actor is Pasted */
	void BroadcastEditPasteActorsBegin() const;

	/** To fire after an Actor is Pasted */
	void BroadcastEditPasteActorsEnd() const;

	/** To fire before an Actor is duplicated */
	void BroadcastDuplicateActorsBegin() const;

	/** To fire after an Actor is duplicated */
	void BroadcastDuplicateActorsEnd() const;

	/** To fire before an Actor is Deleted */
	void BroadcastDeleteActorsBegin() const;

	/** To fire after an Actor is Deleted */
	void BroadcastDeleteActorsEnd() const;

#endif
};