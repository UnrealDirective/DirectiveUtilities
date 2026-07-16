// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "Subsystems/DirectiveUtilEditorActorSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilEditorActorLayoutTest,
	"DirectiveUtilities.EditorActorLayoutTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorLayoutTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create an editor world"));
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	auto SpawnBox = [World](const FVector Location, const FVector Extent, const ECollisionChannel ObjectType) {
		AActor* Actor = World->SpawnActor<AActor>();
		Actor->ClearFlags(RF_Transient);
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionObjectType(ObjectType);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Actor->SetActorLocation(Location);
		return Actor;
	};

	AActor* First = SpawnBox(FVector(0.0f, 0.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);
	AActor* Middle = SpawnBox(FVector(30.0f, 20.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);
	AActor* Last = SpawnBox(FVector(100.0f, 40.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);

	FDirectiveUtilActorOperationResult AlignResult = UDirectiveUtilEditorActorSubsystem::AlignActors(
		{First, Middle, Last},
		EDirectiveUtilActorLayoutAxis::Y,
		EDirectiveUtilActorAlignment::Minimum);
	TestEqual(TEXT("Alignment changes two actors"), AlignResult.ChangedActors.Num(), 2);
	TestTrue(TEXT("Alignment preserves X"), FMath::IsNearlyEqual(Middle->GetActorLocation().X, 30.0f));
	TestTrue(TEXT("Alignment matches minimum bounds"), FMath::IsNearlyEqual(Middle->GetActorLocation().Y, 0.0f));

	FDirectiveUtilActorOperationResult DistributeResult = UDirectiveUtilEditorActorSubsystem::DistributeActors(
		{First, Middle, Last},
		EDirectiveUtilActorLayoutAxis::X,
		EDirectiveUtilActorDistribution::Centers);
	TestEqual(TEXT("Distribution changes the middle actor"), DistributeResult.ChangedActors.Num(), 1);
	TestTrue(TEXT("Center distribution uses equal spacing"), FMath::IsNearlyEqual(Middle->GetActorLocation().X, 50.0f));
	TestTrue(TEXT("Distribution preserves Y"), FMath::IsNearlyEqual(Middle->GetActorLocation().Y, 0.0f));

	AActor* Floor = SpawnBox(FVector::ZeroVector, FVector(200.0f, 200.0f, 10.0f), ECC_WorldStatic);
	World->UpdateWorldComponents(true, false);
	FDirectiveUtilActorOperationResult SnapResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{Middle},
		FVector::DownVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestEqual(TEXT("Surface snapping changes the actor"), SnapResult.ChangedActors.Num(), 1);
	TestTrue(TEXT("Pivot snapping reaches the floor surface"), FMath::IsNearlyEqual(Middle->GetActorLocation().Z, 10.0f, 0.1f));

	AActor* TransientActor = SpawnBox(FVector::ZeroVector, FVector(10.0f), ECC_WorldDynamic);
	TransientActor->SetFlags(RF_Transient);
	FDirectiveUtilActorOperationResult InvalidResult = UDirectiveUtilEditorActorSubsystem::AlignActors(
		{First, TransientActor, nullptr},
		EDirectiveUtilActorLayoutAxis::X,
		EDirectiveUtilActorAlignment::Center);
	TestTrue(TEXT("Transient actors are skipped"), InvalidResult.SkippedActors.Contains(TransientActor));

	Floor->Destroy();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
