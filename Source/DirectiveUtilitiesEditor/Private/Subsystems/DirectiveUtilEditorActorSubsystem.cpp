// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Subsystems/DirectiveUtilEditorActorSubsystem.h"

#include "DirectiveUtilLogChannels.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "EditorViewportClient.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "ScopedTransaction.h"

namespace
{
	// Adds an actor to OutActors when whether it matches the predicate (evaluated once across all of the
	// actor's components) equals whether matches are being included. Centralizes the include/exclude
	// aggregation so a multi-component actor is judged per-actor rather than per-component.
	template <typename TSourceActor, typename TPredicate>
	void FilterActorsByPredicate(const TArray<TSourceActor*>& Actors, TArray<AActor*>& OutActors,
		const EDirectiveUtilInclusivity Inclusivity, TPredicate&& ActorMatches)
	{
		const bool bIncludeMatches = Inclusivity == Include;

		// Filtering an array into itself: snapshot the input and rebuild in place.
		TArray<TSourceActor*> AliasedCopy;
		const TArray<TSourceActor*>* Source = &Actors;
		if (static_cast<const void*>(&Actors) == static_cast<const void*>(&OutActors))
		{
			AliasedCopy = Actors;
			Source = &AliasedCopy;
			OutActors.Reset();
		}

		TSet<AActor*> Seen;
		Seen.Reserve(OutActors.Num() + Source->Num());
		Seen.Append(OutActors);

		for (TSourceActor* Actor : *Source)
		{
			if (!Actor || Seen.Contains(Actor)) { continue; }
			if (ActorMatches(Actor) == bIncludeMatches)
			{
				Seen.Add(Actor);
				OutActors.Add(Actor);
			}
		}
	}
}

void UDirectiveUtilEditorActorSubsystem::FocusActorsInViewport(const TArray<AActor*> Actors, const bool bInstant)
{
	if (Actors.Num() == 0) { return; }
	if (!GEditor) { return; }

	FViewport* ActiveViewport = GEditor->GetActiveViewport();
	if (!ActiveViewport) { return; }

	FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(ActiveViewport->GetClient());
	if (!ViewportClient) { return; }

	FBox BoundingBox = FBox(ForceInit);
	for (const auto Actor : Actors)
	{
		if (!Actor) { continue; }
		BoundingBox += Actor->GetComponentsBoundingBox(true, true);
	}

	ViewportClient->FocusViewportOnBox(BoundingBox, bInstant);
}

TArray<UClass*> UDirectiveUtilEditorActorSubsystem::GetAllLevelClasses()
{
	TArray<UClass*> ActorClasses;
	TArray<AActor*> Actors = GetAllLevelActors();

	for (const AActor* Actor : Actors)
	{
		if (!Actor) { continue; }
		ActorClasses.AddUnique(Actor->GetClass());
	}

	return ActorClasses;
}

void UDirectiveUtilEditorActorSubsystem::FilterStaticMeshActors(
	TArray<AStaticMeshActor*>& OutStaticMeshActors,
	TArray<AActor*> ActorsToFilter) const
{
	TSet<AActor*> Seen;
	Seen.Reserve(OutStaticMeshActors.Num() + ActorsToFilter.Num());
	for (AStaticMeshActor* ExistingActor : OutStaticMeshActors)
	{
		Seen.Add(ExistingActor);
	}

	for (AActor* Actor : ActorsToFilter)
	{
		if (!Actor || Seen.Contains(Actor))
		{
			continue;
		}
		if (Actor->IsA(AStaticMeshActor::StaticClass()))
		{
			Seen.Add(Actor);
			OutStaticMeshActors.Add(Cast<AStaticMeshActor>(Actor));
		}
	}

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Filtered %d static mesh actors"), OutStaticMeshActors.Num());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& ActorName,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		return Actor->GetActorLabel().Contains(ActorName);
	});

	UE_LOG(
		LogDirectiveUtilEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the name [%s]"),
		FilteredActors.Num(),
		Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"),
		*ActorName);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByClass(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSubclassOf<AActor> ActorClass,
	const EDirectiveUtilInclusivity Inclusivity)
{
	if (!ActorClass) { return; }

	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		return Actor->IsA(ActorClass);
	});

	UE_LOG(
		LogDirectiveUtilEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the class %s"),
		FilteredActors.Num(),
		Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"),
		*ActorClass->GetName());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByTag(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FName Tag,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		return Actor->ActorHasTag(Tag);
	});

	UE_LOG(
		LogDirectiveUtilEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the tag %s"),
		FilteredActors.Num(),
		Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"),
		*Tag.ToString());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterialName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& MaterialName,
	const EDirectiveUtilSearchLocation MaterialSource,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }

			if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
			{
				for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
				{
					const UMaterialInterface* Mat = StaticMeshComponent->GetMaterial(i);
					if (Mat && Mat->GetName().Contains(MaterialName)) { return true; }
				}
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
			{
				if (const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh())
				{
					for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
					{
						const UMaterialInterface* Mat = Mesh->GetMaterial(i);
						if (Mat && Mat->GetName().Contains(MaterialName)) { return true; }
					}
				}
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the material %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"), *MaterialName);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterial(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSoftObjectPtr<UMaterialInterface>& Material,
	const EDirectiveUtilSearchLocation MaterialSource,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const UMaterialInterface* TargetMaterial = Material.LoadSynchronous();
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }

			if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
			{
				for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
				{
					if (StaticMeshComponent->GetMaterial(i) == TargetMaterial) { return true; }
				}
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
			{
				if (const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh())
				{
					for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
					{
						if (Mesh->GetMaterial(i) == TargetMaterial) { return true; }
					}
				}
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the material %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"), *Material.ToString());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMeshName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& StaticMeshName,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const UActorComponent* Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }

			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh)
			{
				// A null static mesh only counts as a match when filtering for an empty name.
				if (StaticMeshName.IsEmpty()) { return true; }
				continue;
			}
			if (Mesh->GetName().Contains(StaticMeshName)) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the static mesh %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"), *StaticMeshName);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMesh(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSoftObjectPtr<UStaticMesh>& StaticMesh,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const UStaticMesh* TargetMesh = StaticMesh.LoadSynchronous();
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const UActorComponent* Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }

			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh)
			{
				if (StaticMesh.IsNull()) { return true; }
				continue;
			}
			if (Mesh == TargetMesh) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the static mesh %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("") : TEXT("not"),
	       *StaticMesh.ToString());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByVertCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinVertCount,
	const int32 MaxVertCount,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh || !Mesh->HasValidRenderData(true, 0)) { continue; }

			const int32 VertCount = Mesh->GetNumVertices(0);
			if (VertCount >= MinVertCount && VertCount <= MaxVertCount) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i vertices"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinVertCount, MaxVertCount);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByTriCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinTriCount,
	const int32 MaxTriCount,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh || !Mesh->HasValidRenderData(true, 0)) { continue; }

			const int32 TriCount = Mesh->GetNumTriangles(0);
			if (TriCount >= MinTriCount && TriCount <= MaxTriCount) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i triangles"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinTriCount, MaxTriCount);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByBounds(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& MinBounds,
	const FVector& MaxBounds,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		FVector Origin, Extent;
		Actor->GetActorBounds(false, Origin, Extent);
		const FVector ActorSize = Extent * 2;
		return MinBounds.X <= ActorSize.X && ActorSize.X <= MaxBounds.X &&
			MinBounds.Y <= ActorSize.Y && ActorSize.Y <= MaxBounds.Y &&
			MinBounds.Z <= ActorSize.Z && ActorSize.Z <= MaxBounds.Z;
	});

	UE_LOG(LogDirectiveUtilEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the bounds (%f, %f, %f) and (%f, %f, %f)"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("is") : TEXT("is not"), MinBounds.X,
	       MinBounds.Y, MinBounds.Z,
	       MaxBounds.X, MaxBounds.Y, MaxBounds.Z);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMeshBounds(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& MinBounds,
	const FVector& MaxBounds,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (UActorComponent* Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh()) { continue; }

			const FVector StaticMeshSize = StaticMeshComponent->GetStaticMesh()->GetBounds().BoxExtent * 2;
			if (MinBounds.X <= StaticMeshSize.X && StaticMeshSize.X <= MaxBounds.X &&
				MinBounds.Y <= StaticMeshSize.Y && StaticMeshSize.Y <= MaxBounds.Y &&
				MinBounds.Z <= StaticMeshSize.Z && StaticMeshSize.Z <= MaxBounds.Z) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the static mesh bounds (%f, %f, %f) and (%f, %f, %f)"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("is") : TEXT("is not"), MinBounds.X,
	       MinBounds.Y, MinBounds.Z,
	       MaxBounds.X, MaxBounds.Y, MaxBounds.Z);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByWorldLocation(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& WorldLocation,
	const float Radius,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		return FVector::Dist(Actor->GetActorLocation(), WorldLocation) <= Radius;
	});

	UE_LOG(LogDirectiveUtilEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the world location (%f, %f, %f) with the radius of %f"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("is") : TEXT("is not"), WorldLocation.X,
	       WorldLocation.Y, WorldLocation.Z, Radius);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByLODCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinLODs,
	const int32 MaxLODs,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh) { continue; }
			if (StaticMesh->GetNumLODs() >= MinLODs && StaticMesh->GetNumLODs() <= MaxLODs) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i LODs"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"), MinLODs,
	       MaxLODs);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByNaniteState(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const bool bNaniteEnabled,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh) { continue; }
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
			const bool bMeshNaniteEnabled = Mesh->GetNaniteSettings().bEnabled;
#else
			const bool bMeshNaniteEnabled = Mesh->NaniteSettings.bEnabled;
#endif
			if (bMeshNaniteEnabled == bNaniteEnabled) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s Nanite enabled"),
	       FilteredActors.Num(), bNaniteEnabled ? TEXT("has") : TEXT("does not have"));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByLightmapResolution(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinLightmapResolution,
	const int32 MaxLightmapResolution,
	const EDirectiveUtilSearchLocation SearchLocation,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh) { continue; }

			if ((SearchLocation == BaseAndOverride || SearchLocation == OverrideOnly) && StaticMeshComponent->bOverrideLightMapRes)
			{
				const int32 LightmapRes = StaticMeshComponent->OverriddenLightMapRes;
				if (LightmapRes >= MinLightmapResolution && LightmapRes <= MaxLightmapResolution) { return true; }
			}
			if (SearchLocation == BaseAndOverride || SearchLocation == BaseOnly)
			{
				const int32 LightmapRes = Mesh->GetLightMapResolution();
				if (LightmapRes >= MinLightmapResolution && LightmapRes <= MaxLightmapResolution) { return true; }
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s between %i and %i lightmap resolution"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinLightmapResolution, MaxLightmapResolution);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMobility(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EComponentMobility::Type Mobility,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		const USceneComponent* Root = Actor->GetRootComponent();
		return Root && Root->Mobility == Mobility;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s mobility of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(Mobility));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionChannel(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionChannel CollisionChannel,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			if (StaticMeshComponent->GetCollisionObjectType() == CollisionChannel) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision channel of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionChannel));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionResponse(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionChannel CollisionChannel,
	const ECollisionResponse CollisionResponse,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			if (StaticMeshComponent->GetCollisionResponseToChannel(CollisionChannel) == CollisionResponse) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision response of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionResponse));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionEnabled(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionEnabled::Type CollisionEnabled,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			if (StaticMeshComponent->GetCollisionEnabled() == CollisionEnabled) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision enabled of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionEnabled));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionProfile(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FName CollisionProfile,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			if (StaticMeshComponent->GetCollisionProfileName() == CollisionProfile) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision profile of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *CollisionProfile.ToString());
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByTextureName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString TextureName,
	const EDirectiveUtilSearchLocation Source,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }

			if (Source == BaseAndOverride || Source == OverrideOnly)
			{
				for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
				{
					if (!Material || !Material->GetMaterial()) { continue; }
					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
						{
							if (TextureSample->Texture.GetName().Contains(TextureName)) { return true; }
						}
						if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
						{
							if (TextureObject->Texture.GetName().Contains(TextureName)) { return true; }
						}
					}
				}
			}

			if (Source == BaseAndOverride || Source == BaseOnly)
			{
				const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
				if (!Mesh) { continue; }
				for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
				{
					UMaterialInterface* Material = Mesh->GetMaterial(i);
					if (!Material || !Material->GetMaterial()) { continue; }
					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
						{
							if (TextureSample->Texture.GetName().Contains(TextureName)) { return true; }
						}
						if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
						{
							if (TextureObject->Texture.GetName().Contains(TextureName)) { return true; }
						}
					}
				}
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s texture name of %s"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *TextureName);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByTexture(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	TSoftObjectPtr<UTexture2D> TextureReference,
	const EDirectiveUtilSearchLocation Source,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }

			if (Source == BaseAndOverride || Source == OverrideOnly)
			{
				for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
				{
					if (!Material || !Material->GetMaterial()) { continue; }
					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
						{
							if (TextureSample->Texture == TextureReference) { return true; }
						}
						if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
						{
							if (TextureObject->Texture == TextureReference) { return true; }
						}
					}
				}
			}

			if (Source == BaseAndOverride || Source == BaseOnly)
			{
				const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
				if (!Mesh) { continue; }
				for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
				{
					UMaterialInterface* Material = Mesh->GetMaterial(i);
					if (!Material || !Material->GetMaterial()) { continue; }
					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
						{
							if (TextureSample->Texture == TextureReference) { return true; }
						}
						if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
						{
							if (TextureObject->Texture == TextureReference) { return true; }
						}
					}
				}
			}
		}
		return false;
	});
}

void UDirectiveUtilEditorActorSubsystem::FilterEmptyActors(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByPredicate(Actors, FilteredActors, Inclusivity, [](AActor* Actor) -> bool
	{
		TArray<UActorComponent*> ActorComponents;
		Actor->GetComponents(ActorComponents);
		if (ActorComponents.IsEmpty()) { return true; }
		if (ActorComponents.Num() == 1)
		{
			// ExactCast: a subclass component (mesh, light, camera...) means the actor does something.
			const USceneComponent* SceneComponent = ExactCast<USceneComponent>(ActorComponents[0]);
			return SceneComponent && SceneComponent->GetNumChildrenComponents() == 0;
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Actor Filter: Found %i actors that %s empty"),
	       FilteredActors.Num(), Inclusivity == EDirectiveUtilInclusivity::Include ? TEXT("are") : TEXT("are not"));
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingMaterials(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EDirectiveUtilSearchLocation Location,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByMaterial(Actors, FilteredActors, nullptr, Location, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingStaticMeshes(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByStaticMesh(Actors, FilteredActors, nullptr, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingTextures(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EDirectiveUtilSearchLocation Location,
	const EDirectiveUtilInclusivity Inclusivity)
{
	FilterActorsByTexture(Actors, FilteredActors, nullptr, Location, Inclusivity);
}

bool UDirectiveUtilEditorActorSubsystem::IsActorWithinBoxBounds(AActor* Actor, UBoxComponent* BoxComponent)
{
	if (!Actor || !BoxComponent)
	{
		return false;
	}

	// Transform the actor's location into the box's local space so the box's rotation and its own
	// scale are respected, then test against the unscaled extent (a proper oriented-box check).
	const FVector LocalLocation = BoxComponent->GetComponentTransform().InverseTransformPosition(Actor->GetActorLocation());
	const FVector Extent = BoxComponent->GetUnscaledBoxExtent();
	return FMath::Abs(LocalLocation.X) <= Extent.X
		&& FMath::Abs(LocalLocation.Y) <= Extent.Y
		&& FMath::Abs(LocalLocation.Z) <= Extent.Z;
}

bool UDirectiveUtilEditorActorSubsystem::IsActorWithinSphereBounds(AActor* Actor, USphereComponent* SphereComponent)
{
	if (!Actor || !SphereComponent)
	{
		return false;
	}
	const float DeltaLoc = FVector::Dist(Actor->GetActorLocation(), SphereComponent->GetComponentLocation());
	return DeltaLoc <= SphereComponent->GetScaledSphereRadius();
}

bool UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(AActor* Actor, UCapsuleComponent* CapsuleComponent)
{
	if (!Actor || !CapsuleComponent)
	{
		return false;
	}
	// Proper capsule containment: distance from the actor to the capsule's central segment (its local
	// Z axis, between the cap centres) must be within the radius. Local space respects orientation.
	const FVector LocalLocation = CapsuleComponent->GetComponentTransform().InverseTransformPosition(Actor->GetActorLocation());
	const float Radius = CapsuleComponent->GetUnscaledCapsuleRadius();
	const float SegmentHalfHeight = FMath::Max(0.0f, CapsuleComponent->GetUnscaledCapsuleHalfHeight() - Radius);
	const FVector ClosestOnSegment(0.0f, 0.0f, FMath::Clamp(LocalLocation.Z, -SegmentHalfHeight, SegmentHalfHeight));
	return FVector::Dist(LocalLocation, ClosestOnSegment) <= Radius;
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByClass(
	TArray<AActor*>& FoundActors,
	const TSubclassOf<AActor> ActorClass,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> ActorsToFilter = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByClass(ActorsToFilter, FoundActors, ActorClass, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByName(
	TArray<AActor*>& FoundActors,
	const FString ActorName,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> ActorsToFilter = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByName(ActorsToFilter, FoundActors, ActorName, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByMaterial(
	TArray<AActor*>& FoundActors,
	const UMaterialInterface* Material,
	const EDirectiveUtilSearchLocation MaterialSource,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TSoftObjectPtr<UMaterialInterface> MaterialSoftReference{FSoftObjectPath(Material)};
	GetActorsByMaterialSoftReference(FoundActors, MaterialSoftReference, MaterialSource, SelectionMethod, Inclusivity);
}


void UDirectiveUtilEditorActorSubsystem::GetActorsByMaterialSoftReference(
	TArray<AActor*>& FoundActors,
	const TSoftObjectPtr<UMaterialInterface> Material,
	const EDirectiveUtilSearchLocation MaterialSource,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	TArray<AStaticMeshActor*> StaticMeshActors;
	FilterStaticMeshActors(StaticMeshActors, SourceActors);

	FilterActorsByPredicate(StaticMeshActors, FoundActors, Inclusivity, [&](AStaticMeshActor* StaticMeshActor) -> bool
	{
		const UStaticMeshComponent* StaticMeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!StaticMeshComp) { return false; }
		const UStaticMesh* Mesh = StaticMeshComp->GetStaticMesh();
		if (!Mesh) { return false; }

		if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetNumMaterials(); i++)
			{
				if (StaticMeshComp->GetMaterial(i) == Material) { return true; }
			}
		}
		if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
		{
			for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
			{
				if (Mesh->GetMaterial(i) == Material) { return true; }
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with the material reference were found."),
	       FoundActors.Num());
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByMaterialName(
	TArray<AActor*>& FoundActors,
	const FString MaterialName,
	const EDirectiveUtilSearchLocation MaterialSource,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	TArray<AStaticMeshActor*> StaticMeshActors;
	FilterStaticMeshActors(StaticMeshActors, SourceActors);

	FilterActorsByPredicate(StaticMeshActors, FoundActors, Inclusivity, [&](AStaticMeshActor* StaticMeshActor) -> bool
	{
		const UStaticMeshComponent* StaticMeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!StaticMeshComp) { return false; }
		const UStaticMesh* Mesh = StaticMeshComp->GetStaticMesh();
		if (!Mesh) { return false; }

		if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetNumMaterials(); i++)
			{
				const UMaterialInterface* Mat = StaticMeshComp->GetMaterial(i);
				if (Mat && Mat->GetName().Contains(MaterialName)) { return true; }
			}
		}
		if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
		{
			for (int32 i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
			{
				const UMaterialInterface* Mat = Mesh->GetMaterial(i);
				if (Mat && Mat->GetName().Contains(MaterialName)) { return true; }
			}
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with material %s were found."), FoundActors.Num(),
	       *MaterialName);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByVertexCount(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh || !StaticMesh->HasValidRenderData(true, 0)) { continue; }
			const int32 VertexCount = StaticMesh->GetNumVertices(0);
			if (VertexCount >= From && VertexCount <= To) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with vertex count between %i and %i were found."),
	       FoundActors.Num(), From, To);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByTriCount(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh || !StaticMesh->HasValidRenderData(true, 0)) { continue; }
			const int32 TriCount = StaticMesh->GetNumTriangles(0);
			if (TriCount >= From && TriCount <= To) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor,
		Display,
		TEXT("%i actors with triangle count between %i and %i were found."),
		FoundActors.Num(),
		From,
		To);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByBoundingBox(
	TArray<AActor*>& FoundActors,
	const FVector Min,
	const FVector Max,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		const FBox QueryBox(Min, Max);
		const FBox Box = Actor->GetComponentsBoundingBox();
		return Box.IsValid && QueryBox.IsInsideOrOn(Box.Min) && QueryBox.IsInsideOrOn(Box.Max);
	});


	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with bounding box between %s and %s were found."),
	       FoundActors.Num(), *Min.ToString(), *Max.ToString());
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByMeshSize(
	TArray<AActor*>& FoundActors,
	const float From,
	const float To,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh()) { continue; }
			const double BoundBoxSize = StaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetSize().Size();
			if (BoundBoxSize >= From && BoundBoxSize <= To) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with mesh size between %f and %f were found."),
	       FoundActors.Num(), From, To);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByWorldLocation(
	TArray<AActor*>& FoundActors,
	const FVector WorldLocation,
	const float Radius,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		return FVector::Dist(Actor->GetActorLocation(), WorldLocation) <= Radius;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with world location %s and radius %f were found."),
	       FoundActors.Num(), *WorldLocation.ToString(), Radius);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByLODCount(
	TArray<AActor*>& FoundActors,
	const int32 LODCountFrom,
	const int32 LODCountTo,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh) { continue; }
			if (StaticMesh->GetNumLODs() >= LODCountFrom && StaticMesh->GetNumLODs() <= LODCountTo) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with LOD count between %i and %i were found."),
	       FoundActors.Num(), LODCountFrom, LODCountTo);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByNaniteEnabled(
	TArray<AActor*>& FoundActors,
	const bool bNaniteEnabled,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh) { continue; }
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
			const bool bMeshNaniteEnabled = Mesh->GetNaniteSettings().bEnabled;
#else
			const bool bMeshNaniteEnabled = Mesh->NaniteSettings.bEnabled;
#endif
			if (bMeshNaniteEnabled == bNaniteEnabled) { return true; }
		}
		return false;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with nanite enabled %s were found."),
	       FoundActors.Num(), bNaniteEnabled ? TEXT("true") : TEXT("false"));
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByLightmapResolution(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);
		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh()) { continue; }
			const int32 LightmapRes = StaticMeshComponent->GetStaticMesh()->GetLightMapResolution();
			if (LightmapRes >= From && LightmapRes <= To) { return true; }
		}
		return false;
	});

	UE_LOG(
		LogDirectiveUtilEditor,
		Display,
		TEXT("%i actors with lightmap resolution between %i and %i were found."),
		FoundActors.Num(),
		From,
		To);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByMobility(
	TArray<AActor*>& FoundActors,
	const EComponentMobility::Type Mobility,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		const USceneComponent* Root = Actor->GetRootComponent();
		return Root && Root->Mobility == Mobility;
	});

	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with mobility %s were found."),
	       FoundActors.Num(), *UEnum::GetValueAsName(Mobility).ToString());
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByStaticMesh(
	TArray<AActor*>& FoundActors,
	UStaticMesh* StaticMesh,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	GetActorsByStaticMeshSoftReference(FoundActors, StaticMesh, SelectionMethod, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByStaticMeshSoftReference(
	TArray<AActor*>& FoundActors,
	TSoftObjectPtr<UStaticMesh> StaticMesh,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const auto Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }
			if (StaticMeshComponent->GetStaticMesh() == StaticMesh) { return true; }
		}
		return false;
	});
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with static mesh %s were found."),
	       FoundActors.Num(), *StaticMesh.ToString());
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByStaticMeshName(
	TArray<AActor*>& FoundActors,
	const FString StaticMeshName,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const auto Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }
			const UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh();
			if (!Mesh)
			{
				// A null static mesh only counts as a match when filtering for an empty name.
				if (StaticMeshName.IsEmpty()) { return true; }
				continue;
			}
			if (Mesh->GetName().Contains(StaticMeshName)) { return true; }
		}
		return false;
	});
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with static mesh %s were found."),
	       FoundActors.Num(), *StaticMeshName);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByTexture(
	TArray<AActor*>& FoundActors,
	UTexture2D* Texture,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	GetActorsByTextureSoftReference(FoundActors, Texture, SelectionMethod, Inclusivity);
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByTextureSoftReference(
	TArray<AActor*>& FoundActors,
	const TSoftObjectPtr<UTexture2D> Texture,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const auto Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }
			for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
			{
				if (!Material || !Material->GetMaterial()) { continue; }
				for (const auto& Expression : Material->GetMaterial()->GetExpressions())
				{
					if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
					{
						if (TextureSample->Texture == Texture) { return true; }
					}
					if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
					{
						if (TextureObject->Texture == Texture) { return true; }
					}
				}
			}
		}
		return false;
	});
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with texture %s were found."),
	       FoundActors.Num(), *Texture.ToString());
}

void UDirectiveUtilEditorActorSubsystem::GetActorsByTextureName(
	TArray<AActor*>& FoundActors,
	const FString TextureName,
	const EDirectiveUtilSelectionMethod SelectionMethod,
	const EDirectiveUtilInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	FilterActorsByPredicate(SourceActors, FoundActors, Inclusivity, [&](AActor* Actor) -> bool
	{
		for (const auto Component : Actor->GetComponents())
		{
			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			if (!StaticMeshComponent) { continue; }
			for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
			{
				if (!Material || !Material->GetMaterial()) { continue; }
				for (const auto& Expression : Material->GetMaterial()->GetExpressions())
				{
					if (const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression))
					{
						if (TextureSample->Texture.GetName().Contains(TextureName)) { return true; }
					}
					if (const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression))
					{
						if (TextureObject->Texture.GetName().Contains(TextureName)) { return true; }
					}
				}
			}
		}
		return false;
	});
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i actors with texture %s were found."),
	       FoundActors.Num(), *TextureName);
}

void UDirectiveUtilEditorActorSubsystem::GetInvalidActors(TArray<AActor*>& FoundActors)
{
	for (AActor* Actor : GetAllLevelActors()) { if (!IsValid(Actor)) { FoundActors.AddUnique(Actor); } }
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("%i invalid actors were found."), FoundActors.Num());
}

void UDirectiveUtilEditorActorSubsystem::PushOverrideMaterialsToSource(UStaticMeshComponent* StaticMeshComponent)
{
	if (!IsValid(StaticMeshComponent))
	{
		UE_LOG(LogDirectiveUtilEditor, Error, TEXT("Static Mesh Component is invalid."));
		return;
	}

	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	if (!IsValid(StaticMesh))
	{
		UE_LOG(LogDirectiveUtilEditor, Error, TEXT("Static Mesh Component has no valid static mesh."));
		return;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("DirectiveUtilities", "PushOverrideMaterialsToSource", "Push Override Materials To Source"));
	for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
	{
		if (UMaterialInterface* Material = StaticMeshComponent->GetMaterial(i))
		{
			StaticMesh->SetMaterial(i, Material);
		}
	}
	UE_LOG(LogDirectiveUtilEditor, Display, TEXT("Materials were pushed to source for %s."), *StaticMeshComponent->GetName());
}
