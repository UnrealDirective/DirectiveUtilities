// Copyright Unreal Directive. All Rights Reserved.

#include "Subsystems/UDCoreEditorActorSubsystem.h"

#include "UDCoreLogChannels.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "EditorViewportClient.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Subsystems/SubsystemCollection.h"

void UUDCoreEditorActorSubsystem::FocusActorsInViewport(const TArray<AActor*> Actors, const bool bInstant)
{
	if (Actors.Num() == 0) { return; }

	FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	if (!ViewportClient) { return; }

	// Get bounding box around all actors
	FBox BoundingBox = FBox(ForceInit);
	for (const auto Actor : Actors)
	{
		if (!Actor) { continue; }
		BoundingBox += Actor->GetComponentsBoundingBox(true, true);
	}

	ViewportClient->FocusViewportOnBox(BoundingBox, bInstant);
}

TArray<UClass*> UUDCoreEditorActorSubsystem::GetAllLevelClasses()
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

void UUDCoreEditorActorSubsystem::FilterStaticMeshActors(
	TArray<AStaticMeshActor*>& OutStaticMeshActors,
	TArray<AActor*> ActorsToFilter) const
{
	for (AActor* Actor : ActorsToFilter)
	{
		if (!Actor)
		{
			continue;
		}
		if (Actor->IsA(AStaticMeshActor::StaticClass()))
		{
			OutStaticMeshActors.AddUnique(Cast<AStaticMeshActor>(Actor));
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Filtered %d static mesh actors"), OutStaticMeshActors.Num());
}

void UUDCoreEditorActorSubsystem::FilterActorsByName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& ActorName,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		if (Actor->GetActorLabel().Contains(ActorName) == (Inclusivity == Include))
		{
			FilteredActors.AddUnique(Actor);
		}
	}

	UE_LOG(
		LogUDCoreEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the name [%s]"),
		FilteredActors.Num(),
		Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"),
		*ActorName);
}

void UUDCoreEditorActorSubsystem::FilterActorsByClass(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSubclassOf<AActor> ActorClass,
	const EUDInclusivity Inclusivity)
{
	if (!ActorClass) { return; }

	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		if (Actor->IsA(ActorClass) == (Inclusivity == Include))
		{
			FilteredActors.AddUnique(Actor);
		}
	}

	UE_LOG(
		LogUDCoreEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the class %s"),
		FilteredActors.Num(),
		Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"),
		*ActorClass->GetName());
}

void UUDCoreEditorActorSubsystem::FilterActorsByTag(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FName Tag,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		if (Actor->ActorHasTag(Tag) == (Inclusivity == Include))
		{
			FilteredActors.AddUnique(Actor);
		}
	}

	UE_LOG(
		LogUDCoreEditor,
		Display,
		TEXT("Actor Filter: Found %i actors that does %s contain the tag %s"),
		FilteredActors.Num(),
		Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"),
		*Tag.ToString());
}

void UUDCoreEditorActorSubsystem::FilterActorsByMaterialName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& MaterialName,
	const EUDSearchLocation MaterialSource,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		// Check for Static Mesh Components
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
			{
				for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
				{
					if (StaticMeshComponent->GetMaterial(i) == nullptr)
					{
						continue;
					}
					if (StaticMeshComponent->GetMaterial(i)->GetName().Contains(MaterialName) == (Inclusivity ==
						Include))
					{
						FilteredActors.AddUnique(Actor);
						break;
					}
				}
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
			{
				if (!StaticMeshComponent->GetStaticMesh()) { return; }
				for (int32 i = 0; i < StaticMeshComponent->GetStaticMesh()->GetStaticMaterials().Num(); i++)
				{
					if (StaticMeshComponent->GetStaticMesh()->GetMaterial(i) == nullptr)
					{
						continue;
					}
					if (StaticMeshComponent->GetStaticMesh()->GetMaterial(i)->GetName().Contains(MaterialName) == (
						Inclusivity == Include))
					{
						FilteredActors.AddUnique(Actor);
						break;
					}
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the material %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"), *MaterialName);
}

void UUDCoreEditorActorSubsystem::FilterActorsByMaterial(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSoftObjectPtr<UMaterialInterface>& Material,
	const EUDSearchLocation MaterialSource,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		// Check for Static Mesh Components
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
			{
				for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
				{
					if (StaticMeshComponent->GetMaterial(i) == Material.LoadSynchronous() == (Inclusivity == Include))
					{
						FilteredActors.AddUnique(Actor);
						break;
					}
				}
			}

			if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
			{
				if (!StaticMeshComponent->GetStaticMesh())
				{
					continue;
				}

				for (int32 i = 0; i < StaticMeshComponent->GetStaticMesh()->GetStaticMaterials().Num(); i++)
				{
					if (StaticMeshComponent->GetStaticMesh()->GetMaterial(i) == Material.LoadSynchronous() == (
						Inclusivity == Include))
					{
						FilteredActors.AddUnique(Actor);
						break;
					}
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the material %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"), *Material.ToString());
}

void UUDCoreEditorActorSubsystem::FilterActorsByStaticMeshName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString& StaticMeshName,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				// In the off chance that the static mesh is null and the name is empty, we'll add the actor.
				// Otherwise, we'll skip it.
				if (StaticMeshName.IsEmpty()) { FilteredActors.AddUnique(Actor); }
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh()->GetName().Contains(StaticMeshName) == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the static mesh %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"), *StaticMeshName);
}

void UUDCoreEditorActorSubsystem::FilterActorsByStaticMesh(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const TSoftObjectPtr<UStaticMesh>& StaticMesh,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				if (StaticMesh.IsNull()) { FilteredActors.AddUnique(Actor); }
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh() == StaticMesh.LoadSynchronous() == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that does %s contain the static mesh %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("") : TEXT("not"),
	       *StaticMesh.ToString());
}

void UUDCoreEditorActorSubsystem::FilterActorsByVertCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinVertCount,
	const int32 MaxVertCount,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			const int32 VertCount = StaticMeshComponent->GetStaticMesh()->GetNumVertices(0);

			if (VertCount >= MinVertCount && VertCount <= MaxVertCount == (Inclusivity == Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i vertices"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinVertCount, MaxVertCount);
}

void UUDCoreEditorActorSubsystem::FilterActorsByTriCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinTriCount,
	const int32 MaxTriCount,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			const int32 TriCount = StaticMeshComponent->GetStaticMesh()->GetNumTriangles(0);

			if (TriCount >= MinTriCount && TriCount <= MaxTriCount == (Inclusivity == Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i triangles"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinTriCount, MaxTriCount);
}

void UUDCoreEditorActorSubsystem::FilterActorsByBounds(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& MinBounds,
	const FVector& MaxBounds,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		FVector Origin, Extent;
		Actor->GetActorBounds(false, Origin, Extent);
		const FVector ActorSize = Extent * 2;

		if (MinBounds.X <= ActorSize.X && ActorSize.X <= MaxBounds.X &&
			MinBounds.Y <= ActorSize.Y && ActorSize.Y <= MaxBounds.Y &&
			MinBounds.Z <= ActorSize.Z && ActorSize.Z <= MaxBounds.Z == (Inclusivity == Include))
		{
			FilteredActors.AddUnique(Actor);
		}
	}

	UE_LOG(LogUDCoreEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the bounds (%f, %f, %f) and (%f, %f, %f)"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("is") : TEXT("is not"), MinBounds.X,
	       MinBounds.Y, MinBounds.Z,
	       MaxBounds.X, MaxBounds.Y, MaxBounds.Z);
}

void UUDCoreEditorActorSubsystem::FilterActorsByStaticMeshBounds(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& MinBounds,
	const FVector& MaxBounds,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			FBoxSphereBounds StaticMeshBounds = StaticMeshComponent->GetStaticMesh()->GetBounds();
			const FVector StaticMeshSize = StaticMeshBounds.BoxExtent * 2;

			if (MinBounds.X <= StaticMeshSize.X && StaticMeshSize.X <= MaxBounds.X &&
				MinBounds.Y <= StaticMeshSize.Y && StaticMeshSize.Y <= MaxBounds.Y &&
				MinBounds.Z <= StaticMeshSize.Z && StaticMeshSize.Z <= MaxBounds.Z == (Inclusivity ==
					Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the static mesh bounds (%f, %f, %f) and (%f, %f, %f)"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("is") : TEXT("is not"), MinBounds.X,
	       MinBounds.Y, MinBounds.Z,
	       MaxBounds.X, MaxBounds.Y, MaxBounds.Z);
}

void UUDCoreEditorActorSubsystem::FilterActorsByWorldLocation(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FVector& WorldLocation,
	const float Radius,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		const FVector ActorLocation = Actor->GetActorLocation();

		if (ActorLocation.Equals(WorldLocation, Radius) == (Inclusivity == Include))
		{
			FilteredActors.AddUnique(Actor);
		}
	}

	UE_LOG(LogUDCoreEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s within the world location (%f, %f, %f) with the radius of %f"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("is") : TEXT("is not"), WorldLocation.X,
	       WorldLocation.Y, WorldLocation.Z, Radius);
}

void UUDCoreEditorActorSubsystem::FilterActorsByLODCount(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinLODs,
	const int32 MaxLODs,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh)
			{
				continue;
			}

			if ((StaticMesh->GetNumLODs() >= MinLODs && StaticMesh->GetNumLODs() <= MaxLODs) == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s between %i and %i LODs"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"), MinLODs,
	       MaxLODs);
}

void UUDCoreEditorActorSubsystem::FilterActorsByNaniteState(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const bool bNaniteEnabled,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh()->NaniteSettings.bEnabled == bNaniteEnabled == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s Nanite enabled"),
	       FilteredActors.Num(), bNaniteEnabled ? TEXT("has") : TEXT("does not have"));
}

void UUDCoreEditorActorSubsystem::FilterActorsByLightmapResolution(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const int32 MinLightmapResolution,
	const int32 MaxLightmapResolution,
	const EUDSearchLocation SearchLocation,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			if (SearchLocation == BaseAndOverride || SearchLocation ==
				OverrideOnly)
			{
				const int32 LightmapRes = StaticMeshComponent->OverriddenLightMapRes;

				if ((LightmapRes >= MinLightmapResolution && LightmapRes <= MaxLightmapResolution) == (Inclusivity ==
					Include))
				{
					FilteredActors.AddUnique(Actor);
				}
			}

			if (SearchLocation == BaseAndOverride || SearchLocation == BaseOnly)
			{
				const int32 LightmapRes = StaticMeshComponent->GetStaticMesh()->GetLightMapResolution();

				if ((LightmapRes >= MinLightmapResolution && LightmapRes <= MaxLightmapResolution) == (Inclusivity ==
					Include))
				{
					FilteredActors.AddUnique(Actor);
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display,
	       TEXT("Actor Filter: Found %i actors that %s between %i and %i lightmap resolution"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       MinLightmapResolution, MaxLightmapResolution);
}

void UUDCoreEditorActorSubsystem::FilterActorsByMobility(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EComponentMobility::Type Mobility,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->Mobility == Mobility == (Inclusivity == Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s mobility of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(Mobility));
}

void UUDCoreEditorActorSubsystem::FilterActorsByCollisionChannel(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionChannel CollisionChannel,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->GetCollisionObjectType() == CollisionChannel == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision channel of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionChannel));
}

void UUDCoreEditorActorSubsystem::FilterActorsByCollisionResponse(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionChannel CollisionChannel,
	const ECollisionResponse CollisionResponse,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->GetCollisionResponseToChannel(CollisionChannel) == CollisionResponse == (
				Inclusivity == Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision response of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionResponse));
}

void UUDCoreEditorActorSubsystem::FilterActorsByCollisionEnabled(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const ECollisionEnabled::Type CollisionEnabled,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->GetCollisionEnabled() == CollisionEnabled == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision enabled of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *UEnum::GetValueAsString(CollisionEnabled));
}

void UUDCoreEditorActorSubsystem::FilterActorsByCollisionProfile(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FName CollisionProfile,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->GetCollisionProfileName() == CollisionProfile == (Inclusivity ==
				Include))
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s collision profile of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *CollisionProfile.ToString());
}

void UUDCoreEditorActorSubsystem::FilterActorsByTextureName(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const FString TextureName,
	const EUDSearchLocation Source,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (Source == BaseAndOverride || Source == OverrideOnly)
			{
				for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
				{
					if (!Material)
					{
						continue;
					}

					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (!Expression)
						{
							continue;
						}

						// Check if the Material Expression is a Texture Sample Expression.
						if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
						{
							const UMaterialExpressionTextureSample* TextureSample = Cast<
								UMaterialExpressionTextureSample>(Expression);
							if (TextureSample->Texture.GetName().Contains(TextureName) == (Inclusivity ==
								Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}

						// Check if the Material Expression is a Texture Object instead of a Texture Sample.
						if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
						{
							const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression);
							if (TextureObject->Texture.GetName().Contains(TextureName) == (Inclusivity ==
								Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}
					}
				}
			}

			if (Source == BaseAndOverride || Source == BaseOnly)
			{
				for (int32 i = 0; i < StaticMeshComponent->GetStaticMesh()->GetStaticMaterials().Num(); i++)
				{
					UMaterialInterface* Material = StaticMeshComponent->GetStaticMesh()->GetMaterial(i);
					if (!Material)
					{
						continue;
					}

					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (!Expression)
						{
							continue;
						}

						// Check if the Material Expression is a Texture Sample Expression.
						if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
						{
							const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
							if (TextureSample->Texture.GetName().Contains(TextureName) == (Inclusivity == Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}

						// Check if the Material Expression is a Texture Object instead of a Texture Sample.
						if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
						{
							const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression);
							if (TextureObject->Texture.GetName().Contains(TextureName) == (Inclusivity == Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}
					}
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("Actor Filter: Found %i actors that %s texture name of %s"),
	       FilteredActors.Num(), Inclusivity == EUDInclusivity::Include ? TEXT("has") : TEXT("does not have"),
	       *TextureName);
}

void UUDCoreEditorActorSubsystem::FilterActorsByTexture(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	TSoftObjectPtr<UTexture2D> TextureReference,
	const EUDSearchLocation Source,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			if (Source == BaseAndOverride || Source == OverrideOnly)
			{
				for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
				{
					if (!Material)
					{
						continue;
					}

					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (!Expression)
						{
							continue;
						}

						// Check if the Material Expression is a Texture Sample Expression.
						if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
						{
							const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
							if (TextureSample->Texture == TextureReference == (Inclusivity == Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}

						// Check if the Material Expression is a Texture Object instead of a Texture Sample.
						if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
						{
							const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression);
							if (TextureObject->Texture == TextureReference == (Inclusivity == Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}
					}
				}
			}

			if (Source == BaseAndOverride || Source == BaseOnly)
			{
				if (!StaticMeshComponent->GetStaticMesh())
				{
					continue;
				}
				for (int32 i = 0; i < StaticMeshComponent->GetStaticMesh()->GetStaticMaterials().Num(); i++)
				{
					UMaterialInterface* Material = StaticMeshComponent->GetStaticMesh()->GetMaterial(i);
					if (!Material)
					{
						continue;
					}

					for (const auto& Expression : Material->GetMaterial()->GetExpressions())
					{
						if (!Expression)
						{
							continue;
						}

						// Check if the Material Expression
						if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
						{
							const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
							if (TextureSample->Texture == TextureReference == (Inclusivity == Include))
							{
								FilteredActors.AddUnique(Actor);
								break;
							}
						}
					}
				}
			}
		}
	}
}

void UUDCoreEditorActorSubsystem::FilterEmptyActors(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : Actors)
	{
		if (!Actor || FilteredActors.Contains(Actor)) { continue; }

		TArray<UActorComponent*> ActorComponents;
		Actor->GetComponents(ActorComponents);

		if (ActorComponents.IsEmpty() && Inclusivity == Include)
		{
			FilteredActors.AddUnique(Actor);
			continue;
		}

		if (ActorComponents.Num() == 1 && Inclusivity == Exclude)
		{
			const USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponents[0]);
			if (SceneComponent->GetNumChildrenComponents() == 0)
			{
				FilteredActors.AddUnique(Actor);
			}
			continue;
		}

		// If the actor has a root component, it will always have 2 components
		if (ActorComponents.Num() == 2 && Inclusivity == Include)
		{
			const USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponents[1]);
			if (SceneComponent->GetNumChildrenComponents() == 0)
			{
				FilteredActors.AddUnique(Actor);
			}
		}
	}
}

void UUDCoreEditorActorSubsystem::FilterActorsByMissingMaterials(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EUDSearchLocation Location,
	const EUDInclusivity Inclusivity)
{
	FilterActorsByMaterial(Actors, FilteredActors, nullptr, Location, Inclusivity);
}

void UUDCoreEditorActorSubsystem::FilterActorsByMissingStaticMeshes(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EUDInclusivity Inclusivity)
{
	FilterActorsByStaticMesh(Actors, FilteredActors, nullptr, Inclusivity);
}

void UUDCoreEditorActorSubsystem::FilterActorsByMissingTextures(
	const TArray<AActor*>& Actors,
	TArray<AActor*>& FilteredActors,
	const EUDSearchLocation Location,
	const EUDInclusivity Inclusivity)
{
	FilterActorsByTexture(Actors, FilteredActors, nullptr, Location, Inclusivity);
}

bool UUDCoreEditorActorSubsystem::IsActorWithinBoxBounds(AActor* Actor, UBoxComponent* BoxComponent)
{
	if (!Actor && !BoxComponent)
	{
		return false;
	}

	const FVector ActorLocation = Actor->GetActorLocation();
	const FVector BoxComponentLocation = BoxComponent->GetComponentLocation();

	// Need to multiply the box component extents by the actor scale to get the correct bounds
	const FVector BoxComponentExtent = BoxComponent->GetScaledBoxExtent() * Actor->GetActorScale();

	// Now check if the actor is within the box component bounds
	return ActorLocation.X >= BoxComponentLocation.X - BoxComponentExtent.X
		&& ActorLocation.X <= BoxComponentLocation.X + BoxComponentExtent.X
		&& ActorLocation.Y >= BoxComponentLocation.Y - BoxComponentExtent.Y
		&& ActorLocation.Y <= BoxComponentLocation.Y + BoxComponentExtent.Y
		&& ActorLocation.Z >= BoxComponentLocation.Z - BoxComponentExtent.Z
		&& ActorLocation.Z <= BoxComponentLocation.Z + BoxComponentExtent.Z;
}

bool UUDCoreEditorActorSubsystem::IsActorWithinSphereBounds(AActor* Actor, USphereComponent* SphereComponent)
{
	if (!Actor && !SphereComponent)
	{
		return false;
	}
	const float DeltaLoc = FVector::Dist(Actor->GetActorLocation(), SphereComponent->GetComponentLocation());
	return DeltaLoc <= SphereComponent->GetScaledSphereRadius();
}

bool UUDCoreEditorActorSubsystem::IsActorWithinCapsuleBounds(AActor* Actor, UCapsuleComponent* CapsuleComponent)
{
	if (!Actor && !CapsuleComponent)
	{
		return false;
	}
	const float DeltaLoc = FVector::Dist(Actor->GetActorLocation(), CapsuleComponent->GetComponentLocation());
	return DeltaLoc <= CapsuleComponent->GetScaledCapsuleRadius() + CapsuleComponent->GetScaledCapsuleHalfHeight();
}

void UUDCoreEditorActorSubsystem::GetActorsByClass(
	TArray<AActor*>& FoundActors,
	const TSubclassOf<AActor> ActorClass,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> ActorsToFilter = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByClass(ActorsToFilter, FoundActors, ActorClass, Inclusivity);
}

void UUDCoreEditorActorSubsystem::GetActorsByName(
	TArray<AActor*>& FoundActors,
	const FString ActorName,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> ActorsToFilter = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();
	FilterActorsByName(ActorsToFilter, FoundActors, ActorName, Inclusivity);
}

void UUDCoreEditorActorSubsystem::GetActorsByMaterial(
	TArray<AActor*>& FoundActors,
	const UMaterialInterface* Material,
	const EUDSearchLocation MaterialSource,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TSoftObjectPtr<UMaterialInterface> MaterialSoftReference{FSoftObjectPath(Material)};
	GetActorsByMaterialSoftReference(FoundActors, MaterialSoftReference, MaterialSource, SelectionMethod, Inclusivity);
}


void UUDCoreEditorActorSubsystem::GetActorsByMaterialSoftReference(
	TArray<AActor*>& FoundActors,
	const TSoftObjectPtr<UMaterialInterface> Material,
	const EUDSearchLocation MaterialSource,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	TArray<AStaticMeshActor*> StaticMeshActors;
	FilterStaticMeshActors(StaticMeshActors, SourceActors);

	for (AStaticMeshActor* StaticMeshActor : StaticMeshActors)
	{
		if (!StaticMeshActor)
		{
			continue;
		}

		const UStaticMeshComponent* StaticMeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (StaticMeshComp == nullptr)
		{
			continue;
		}

		if (!StaticMeshComp->GetStaticMesh())
		{
			continue;
		}

		if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetNumMaterials(); i++)
			{
				if (StaticMeshComp->GetMaterial(i) == Material == (Inclusivity == Include))
				{
					FoundActors.AddUnique(StaticMeshActor);
					break;
				}
			}
		}


		if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetStaticMesh()->GetStaticMaterials().Num(); i++)
			{
				if (StaticMeshComp->GetStaticMesh()->GetMaterial(i) == Material == (Inclusivity == Include))
				{
					// Add only if not already added in the previous loop
					FoundActors.AddUnique(StaticMeshActor);
					break;
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with the material reference were found."),
	       FoundActors.Num());
}

void UUDCoreEditorActorSubsystem::GetActorsByMaterialName(
	TArray<AActor*>& FoundActors,
	const FString MaterialName,
	const EUDSearchLocation MaterialSource,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	TArray<AStaticMeshActor*> StaticMeshActors;
	FilterStaticMeshActors(StaticMeshActors, SourceActors);

	for (AStaticMeshActor* StaticMeshActor : StaticMeshActors)
	{
		if (!StaticMeshActor)
		{
			continue;
		}

		const UStaticMeshComponent* StaticMeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!StaticMeshComp)
		{
			continue;
		}

		if (!StaticMeshComp->GetStaticMesh())
		{
			continue;
		}

		if (MaterialSource == BaseAndOverride || MaterialSource == OverrideOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetNumMaterials(); i++)
			{
				if (StaticMeshComp->GetMaterial(i) == nullptr)
				{
					continue;
				}
				if (StaticMeshComp->GetMaterial(i)->GetName().Contains(MaterialName) == (Inclusivity ==
					Include))
				{
					FoundActors.AddUnique(StaticMeshActor);
					break;
				}
			}
		}

		if (MaterialSource == BaseAndOverride || MaterialSource == BaseOnly)
		{
			for (int32 i = 0; i < StaticMeshComp->GetStaticMesh()->GetStaticMaterials().Num(); i++)
			{
				if (StaticMeshComp->GetStaticMesh()->GetMaterial(i) == nullptr)
				{
					continue;
				}
				if (StaticMeshComp->GetStaticMesh()->GetMaterial(i)->GetName().Contains(MaterialName) == (Inclusivity ==
					Include))
				{
					FoundActors.AddUnique(StaticMeshActor);
					break;
				}
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with material %s were found."), FoundActors.Num(),
	       *MaterialName);
}

void UUDCoreEditorActorSubsystem::GetActorsByVertexCount(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	for (AActor* Actor : SourceActors)
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh)
			{
				continue;
			}

			const int32 VertexCount = StaticMesh->GetNumVertices(0);
			if ((VertexCount >= From && VertexCount <= To) == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with vertex count between %i and %i were found."),
	       FoundActors.Num(), From, To);
}

void UUDCoreEditorActorSubsystem::GetActorsByTriCount(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			const int32 TriCount = StaticMeshComponent->GetStaticMesh()->GetNumTriangles(0);

			if ((TriCount >= From && TriCount <= To) == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor,
		Display,
		TEXT("%i actors with triangle count between %i and %i were found."),
		FoundActors.Num(),
		From,
		To);
}

void UUDCoreEditorActorSubsystem::GetActorsByBoundingBox(
	TArray<AActor*>& FoundActors,
	const FVector Min,
	const FVector Max,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		if (Actor->GetComponentsBoundingBox().Min == Min && Actor->GetComponentsBoundingBox().Max == Max == (Inclusivity
			== Include))
		{
			FoundActors.AddUnique(Actor);
		}
	}


	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with bounding box between %s and %s were found."),
	       FoundActors.Num(), *Min.ToString(), *Max.ToString());
}

void UUDCoreEditorActorSubsystem::GetActorsByMeshSize(
	TArray<AActor*>& FoundActors,
	const float From,
	const float To,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			const double BoundBoxSize = StaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetSize().Size();

			if ((BoundBoxSize >= From && BoundBoxSize <= To) == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with mesh size between %f and %f were found."),
	       FoundActors.Num(), From, To);
}

void UUDCoreEditorActorSubsystem::GetActorsByWorldLocation(
	TArray<AActor*>& FoundActors,
	const FVector WorldLocation,
	const float Radius,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		if (Actor->GetActorLocation().Equals(WorldLocation, Radius) == (Inclusivity == Include))
		{
			FoundActors.AddUnique(Actor);
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with world location %s and radius %f were found."),
	       FoundActors.Num(), *WorldLocation.ToString(), Radius);
}

void UUDCoreEditorActorSubsystem::GetActorsByLODCount(
	TArray<AActor*>& FoundActors,
	const int32 LODCountFrom,
	const int32 LODCountTo,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}

			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (!StaticMesh)
			{
				continue;
			}

			if ((StaticMesh->GetNumLODs() >= LODCountFrom && StaticMesh->GetNumLODs() <= LODCountTo) == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with LOD count between %i and %i were found."),
	       FoundActors.Num(), LODCountFrom, LODCountTo);
}

void UUDCoreEditorActorSubsystem::GetActorsByNaniteEnabled(
	TArray<AActor*>& FoundActors,
	const bool bNaniteEnabled,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh()->NaniteSettings.bEnabled == bNaniteEnabled == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with nanite enabled %s were found."),
	       FoundActors.Num(), bNaniteEnabled ? TEXT("true") : TEXT("false"));
}

void UUDCoreEditorActorSubsystem::GetActorsByLightmapResolution(
	TArray<AActor*>& FoundActors,
	const int32 From,
	const int32 To,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		
		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents, true);

		for (const UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				continue;
			}

			const int32 LightmapRes = StaticMeshComponent->GetStaticMesh()->GetLightMapResolution();

			if ((LightmapRes >= From && LightmapRes <= To) == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}

	UE_LOG(
		LogUDCoreEditor,
		Display,
		TEXT("%i actors with lightmap resolution between %i and %i were found."),
		FoundActors.Num(),
		From,
		To);
}

void UUDCoreEditorActorSubsystem::GetActorsByMobility(
	TArray<AActor*>& FoundActors,
	const EComponentMobility::Type Mobility,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	for (AActor* Actor : SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors())
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}
		
		if (!Actor->GetRootComponent())
		{
			continue;
		}
		
		if (Actor->GetRootComponent()->Mobility == Mobility == (Inclusivity == Include))
		{
			FoundActors.AddUnique(Actor);
		}
	}

	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with mobility %s were found."),
	       FoundActors.Num(), *UEnum::GetValueAsName(Mobility).ToString());
}

void UUDCoreEditorActorSubsystem::GetActorsByStaticMesh(
	TArray<AActor*>& FoundActors,
	UStaticMesh* StaticMesh,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	GetActorsByStaticMeshSoftReference(FoundActors, StaticMesh, SelectionMethod, Inclusivity);
}

void UUDCoreEditorActorSubsystem::GetActorsByStaticMeshSoftReference(
	TArray<AActor*>& FoundActors,
	TSoftObjectPtr<UStaticMesh> StaticMesh,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	for (AActor* Actor : SourceActors)
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh() == StaticMesh == (Inclusivity == Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}
	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with static mesh %s were found."),
	       FoundActors.Num(), *StaticMesh.ToString());
}

void UUDCoreEditorActorSubsystem::GetActorsByStaticMeshName(
	TArray<AActor*>& FoundActors,
	const FString StaticMeshName,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	for (AActor* Actor : SourceActors)
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}
			
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			if (!StaticMeshComponent)
			{
				continue;
			}
			if (!StaticMeshComponent->GetStaticMesh())
			{
				// In the off chance that the static mesh is null, and the name is empty, we'll add the actor.
				// Otherwise, we'll skip it.
				if (StaticMeshName.IsEmpty()) { FoundActors.AddUnique(Actor); }
				continue;
			}

			if (StaticMeshComponent->GetStaticMesh()->GetName() == StaticMeshName == (Inclusivity ==
				Include))
			{
				FoundActors.AddUnique(Actor);
			}
		}
	}
	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with static mesh %s were found."),
	       FoundActors.Num(), *StaticMeshName);
}

void UUDCoreEditorActorSubsystem::GetActorsByTexture(
	TArray<AActor*>& FoundActors,
	UTexture2D* Texture,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	GetActorsByTextureSoftReference(FoundActors, Texture, SelectionMethod, Inclusivity);
}

void UUDCoreEditorActorSubsystem::GetActorsByTextureSoftReference(
	TArray<AActor*>& FoundActors,
	const TSoftObjectPtr<UTexture2D> Texture,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	for (AActor* Actor : SourceActors)
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}

			// Verify that the component is a static mesh component as that is the only component type that can have a texture.
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
			{
				if (!Material)
				{
					continue;
				}

				for (const auto& Expression : Material->GetMaterial()->GetExpressions())
				{
					if (!Expression)
					{
						continue;
					}

					// Check if the Material Expression is a Texture Sample Expression.
					if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
					{
						const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
						if (TextureSample->Texture == Texture == (Inclusivity == Include))
						{
							FoundActors.AddUnique(Actor);
							break;
						}
					}

					// Check if the Material Expression is a Texture Object instead of a Texture Sample.
					if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
					{
						const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression);
						if (TextureObject->Texture == Texture == (Inclusivity == Include))
						{
							FoundActors.AddUnique(Actor);
							break;
						}
					}
				}
			}
		}
	}
	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with texture %s were found."),
	       FoundActors.Num(), *Texture.ToString());
}

void UUDCoreEditorActorSubsystem::GetActorsByTextureName(
	TArray<AActor*>& FoundActors,
	const FString TextureName,
	const EUDSelectionMethod SelectionMethod,
	const EUDInclusivity Inclusivity)
{
	const TArray<AActor*> SourceActors = SelectionMethod == Selection ? GetSelectedLevelActors() : GetAllLevelActors();

	for (AActor* Actor : SourceActors)
	{
		if (!Actor || FoundActors.Contains(Actor))
		{
			continue;
		}

		for (const auto Component : Actor->GetComponents())
		{
			if (!Component)
			{
				continue;
			}

			// Verify that the component is a static mesh component as that is the only component type that can have a texture.
			if (!Component->IsA(UStaticMeshComponent::StaticClass()))
			{
				continue;
			}

			const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);

			for (UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
			{
				if (!Material)
				{
					continue;
				}

				for (const auto& Expression : Material->GetMaterial()->GetExpressions())
				{
					if (!Expression)
					{
						continue;
					}

					// Check if the Material Expression is a Texture Sample Expression.
					if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
					{
						const UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
						if (TextureSample->Texture.GetName().Contains(TextureName) == (Inclusivity ==
							Include))
						{
							FoundActors.AddUnique(Actor);
							break;
						}
					}

					// Check if the Material Expression is a Texture Object instead of a Texture Sample.
					if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
					{
						const UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Expression);
						if (TextureObject->Texture.GetName().Contains(TextureName) == (Inclusivity ==
							Include))
						{
							FoundActors.AddUnique(Actor);
							break;
						}
					}
				}
			}
		}
	}
	UE_LOG(LogUDCoreEditor, Display, TEXT("%i actors with texture %s were found."),
	       FoundActors.Num(), *TextureName);
}

void UUDCoreEditorActorSubsystem::GetInvalidActors(TArray<AActor*>& FoundActors)
{
	for (AActor* Actor : GetAllLevelActors()) { if (!IsValid(Actor)) { FoundActors.AddUnique(Actor); } }
	UE_LOG(LogUDCoreEditor, Display, TEXT("%i invalid actors were found."), FoundActors.Num());
}

void UUDCoreEditorActorSubsystem::PushOverrideMaterialsToSource(UStaticMeshComponent* StaticMeshComponent)
{
	if (!IsValid(StaticMeshComponent))
	{
		UE_LOG(LogUDCoreEditor, Error, TEXT("Static Mesh Component is invalid."));
		return;
	}

	for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); i++)
	{
		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
		if (!IsValid(StaticMesh)) { continue; }

		if (UMaterialInterface* Material = StaticMeshComponent->GetMaterial(i))
		{
			StaticMesh->SetMaterial(i, Material);
		}
	}
	UE_LOG(LogUDCoreEditor, Display, TEXT("Materials were pushed to source for %s."), *StaticMeshComponent->GetName());
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 5
// These functions were added for Unreal Engine 5.5 to solve a code regression.
// For more information, see https://github.com/UnrealDirective/UDCore/issues/6
// TODO: Remove these once UE 5.5 is no longer supported.

void UDCOREEDITOR_API UUDCoreEditorActorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FEditorDelegates::OnNewActorsDropped.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditNewActorsDropped);
	FEditorDelegates::OnNewActorsPlaced.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditNewActorsPlaced);

	FEditorDelegates::OnEditCutActorsBegin.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditCutActorsBegin);
	FEditorDelegates::OnEditCutActorsEnd.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditCutActorsEnd);

	FEditorDelegates::OnEditCopyActorsBegin.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditCopyActorsBegin);
	FEditorDelegates::OnEditCopyActorsEnd.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditCopyActorsEnd);

	FEditorDelegates::OnEditPasteActorsBegin.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditPasteActorsBegin);
	FEditorDelegates::OnEditPasteActorsEnd.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastEditPasteActorsEnd);

	FEditorDelegates::OnDuplicateActorsBegin.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastDuplicateActorsBegin);
	FEditorDelegates::OnDuplicateActorsEnd.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastDuplicateActorsEnd);

	FEditorDelegates::OnDeleteActorsBegin.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastDeleteActorsBegin);
	FEditorDelegates::OnDeleteActorsEnd.AddUObject(this, &UUDCoreEditorActorSubsystem::BroadcastDeleteActorsEnd);
}

void UUDCoreEditorActorSubsystem::BroadcastEditNewActorsDropped(const TArray<UObject*>& DroppedObjects, const TArray<AActor*>& DroppedActors) const
{
	OnNewActorsDropped.Broadcast(DroppedObjects, DroppedActors);
}

void UUDCoreEditorActorSubsystem::BroadcastEditNewActorsPlaced(UObject* ObjToUse, const TArray<AActor*>& PlacedActors) const
{
	OnNewActorsPlaced.Broadcast(ObjToUse, PlacedActors);
}

void UUDCoreEditorActorSubsystem::BroadcastEditCutActorsBegin() const
{
	OnEditCutActorsBegin.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastEditCutActorsEnd() const
{
	OnEditCutActorsEnd.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastEditCopyActorsBegin() const
{
	OnEditCopyActorsBegin.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastEditCopyActorsEnd() const
{
	OnEditCopyActorsEnd.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastEditPasteActorsBegin() const
{
	OnEditPasteActorsBegin.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastEditPasteActorsEnd() const
{
	OnEditPasteActorsEnd.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastDuplicateActorsBegin() const
{
	OnDuplicateActorsBegin.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastDuplicateActorsEnd() const
{
	OnDuplicateActorsEnd.Broadcast();
}

void UUDCoreEditorActorSubsystem::BroadcastDeleteActorsBegin() const
{
	OnDeleteActorsBegin.Broadcast();
}
void UUDCoreEditorActorSubsystem::BroadcastDeleteActorsEnd() const
{
	OnDeleteActorsEnd.Broadcast();
}

#endif
