// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "Subsystems/DirectiveUtilEditorActorSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

#include <limits>

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
	const FVector BeforeInvalidSnap = Middle->GetActorLocation();
	const FDirectiveUtilActorOperationResult InvalidDistanceResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{Middle},
		FVector::DownVector,
		std::numeric_limits<float>::quiet_NaN(),
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestTrue(TEXT("Surface snapping skips a non-finite distance"), InvalidDistanceResult.SkippedActors.Contains(Middle));
	TestTrue(TEXT("A non-finite distance preserves the actor transform"), Middle->GetActorLocation().Equals(BeforeInvalidSnap));

	const FDirectiveUtilActorOperationResult InvalidDirectionResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{Middle},
		FVector(std::numeric_limits<float>::infinity(), 0.0f, -1.0f),
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestTrue(TEXT("Surface snapping skips a non-finite direction"), InvalidDirectionResult.SkippedActors.Contains(Middle));

	const FDirectiveUtilActorOperationResult InvalidChannelResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{Middle},
		FVector::DownVector,
		500.0f,
		static_cast<ECollisionChannel>(ECC_MAX),
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestTrue(TEXT("Surface snapping skips an invalid collision channel"), InvalidChannelResult.SkippedActors.Contains(Middle));

	FDirectiveUtilActorOperationResult SnapResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{Middle},
		FVector::DownVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestEqual(TEXT("Surface snapping changes the actor"), SnapResult.ChangedActors.Num(), 1);
	TestTrue(TEXT("Pivot snapping reaches the floor surface"), FMath::IsNearlyEqual(Middle->GetActorLocation().Z, 10.0f, 0.1f));

	AActor* BoundsActor = SpawnBox(FVector(150.0f, 150.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);
	FDirectiveUtilActorOperationResult BoundsSnapResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{BoundsActor},
		FVector::DownVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Bounds,
		false);
	TestEqual(TEXT("Bounds snapping changes the actor"), BoundsSnapResult.ChangedActors.Num(), 1);
	TestTrue(TEXT("Bounds snapping places the lower bound on the surface"), FMath::IsNearlyEqual(BoundsActor->GetActorLocation().Z, 20.0f, 0.1f));

	AActor* NormalActor = SpawnBox(FVector(-150.0f, -150.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);
	NormalActor->SetActorRotation(FRotator(45.0f, 0.0f, 0.0f));
	FDirectiveUtilActorOperationResult NormalSnapResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{NormalActor},
		FVector::DownVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		true);
	TestEqual(TEXT("Normal-aligned snapping changes the actor"), NormalSnapResult.ChangedActors.Num(), 1);
	TestTrue(TEXT("Normal-aligned snapping points the actor up from the surface"), NormalActor->GetActorUpVector().Equals(FVector::UpVector, 0.01f));

	AActor* MissActor = SpawnBox(FVector(500.0f, 500.0f, 100.0f), FVector(10.0f), ECC_WorldDynamic);
	FDirectiveUtilActorOperationResult MissResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{MissActor},
		FVector::DownVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestTrue(TEXT("Surface snapping reports collision misses as skipped"), MissResult.SkippedActors.Contains(MissActor));

	Floor->Destroy();
	AActor* Ceiling = SpawnBox(FVector(0.0f, 0.0f, 50.0f), FVector(20.0f, 20.0f, 10.0f), ECC_WorldStatic);
	World->UpdateWorldComponents(true, false);
	AActor* ActorWithoutRoot = World->SpawnActor<AActor>();
	ActorWithoutRoot->ClearFlags(RF_Transient);
	FDirectiveUtilActorOperationResult FailedMoveResult = UDirectiveUtilEditorActorSubsystem::SnapActorsToSurface(
		{ActorWithoutRoot},
		FVector::UpVector,
		500.0f,
		ECC_Visibility,
		EDirectiveUtilSurfacePlacement::Pivot,
		false);
	TestTrue(TEXT("Surface snapping reports a failed transform as skipped"), FailedMoveResult.SkippedActors.Contains(ActorWithoutRoot));
	TestFalse(TEXT("A failed transform is not reported as changed"), FailedMoveResult.ChangedActors.Contains(ActorWithoutRoot));

	AActor* TransientActor = SpawnBox(FVector::ZeroVector, FVector(10.0f), ECC_WorldDynamic);
	TransientActor->SetFlags(RF_Transient);
	FDirectiveUtilActorOperationResult InvalidResult = UDirectiveUtilEditorActorSubsystem::AlignActors(
		{First, TransientActor, nullptr},
		EDirectiveUtilActorLayoutAxis::X,
		EDirectiveUtilActorAlignment::Center);
	TestTrue(TEXT("Transient actors are skipped"), InvalidResult.SkippedActors.Contains(TransientActor));

	Ceiling->Destroy();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
