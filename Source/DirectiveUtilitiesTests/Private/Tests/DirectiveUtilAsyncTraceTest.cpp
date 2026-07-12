// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_AsyncTrace.h"
#include "Tasks/DirectiveUtilTask_MoveToLocation.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR

namespace DirectiveUtilAsyncTraceTestHelpers
{
	/** Creates a transient game world with a physics scene, initialized for play so traces resolve. */
	UWorld* CreateTraceWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		if (!World)
		{
			return nullptr;
		}
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
		return World;
	}

	/** Spawns a blocking cube actor at the origin so traces have something to hit. */
	AStaticMeshActor* SpawnBlockingCube(UWorld* World, UStaticMesh* CubeMesh)
	{
		AStaticMeshActor* Cube = World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		UStaticMeshComponent* Component = Cube->GetStaticMeshComponent();
		Component->SetMobility(EComponentMobility::Movable);
		Component->SetStaticMesh(CubeMesh);
		Component->SetCollisionProfileName(TEXT("BlockAll"));
		Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Component->UpdateCollisionProfile();
		return Cube;
	}
}

/**
 * Latent command that ticks a trace world each frame until every listener has reported completion
 * (or the frame budget runs out), asserts the expected outcome, and tears the world down.
 */
class FDirectiveUtilTickTraceWorld : public IAutomationLatentCommand
{
public:
	FDirectiveUtilTickTraceWorld(FAutomationTestBase* InTest, UWorld* InWorld, const TArray<UDirectiveUtilDelegateListener*>& InListeners, int32 InFrames)
		: Test(InTest)
		, World(InWorld)
		, Listeners(InListeners)
		, FramesRemaining(InFrames)
	{
	}

	virtual bool Update() override
	{
		if (UWorld* TickWorld = World.Get())
		{
			TickWorld->Tick(LEVELTICK_All, 0.05f);
		}

		bool bAllComplete = true;
		for (const UDirectiveUtilDelegateListener* Listener : Listeners)
		{
			if (Listener && !Listener->bCompleted)
			{
				bAllComplete = false;
				break;
			}
		}

		if (bAllComplete || --FramesRemaining <= 0)
		{
			for (UDirectiveUtilDelegateListener* Listener : Listeners)
			{
				if (!Listener)
				{
					continue;
				}
				Test->TestTrue(TEXT("Async trace broadcasts Completed"), Listener->bCompleted);
				Test->TestTrue(TEXT("Async trace through a blocking cube reports a hit"), Listener->HitCount > 0);
				Listener->Keepalive = nullptr;
				Listener->RemoveFromRoot();
			}

			if (UWorld* TearDownWorld = World.Get())
			{
				GEngine->DestroyWorldContext(TearDownWorld);
				TearDownWorld->DestroyWorld(false);
			}
			return true;
		}

		return false;
	}

private:
	FAutomationTestBase* Test;
	TWeakObjectPtr<UWorld> World;
	TArray<UDirectiveUtilDelegateListener*> Listeners;
	int32 FramesRemaining;
};

/**
 * DirectiveUtilTask_AsyncTrace: verifies the null-world guard broadcasts an empty result, and that each trace
 * shape (line, sphere, box, capsule) resolves against a blocking body and reports a hit.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncTraceTest, "DirectiveUtilities.AsyncTaskTraceTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncTraceTest::RunTest(const FString& Parameters)
{
	// The null-world activation intentionally logs a warning.
	AddExpectedMessagePlain(TEXT("Async Trace failed to activate. World is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Null world guard: activating with a null context broadcasts an empty result and does not crash.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_AsyncTrace* Task = UDirectiveUtilTask_AsyncTrace::AsyncLineTraceByChannel(nullptr, FVector::ZeroVector, FVector(0, 0, 100), ETraceTypeQuery::TraceTypeQuery1, false);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTraceCompleted);
		Task->Activate();

		TestTrue("Null world trace still broadcasts Completed", Listener->bCompleted);
		TestEqual("Null world trace reports no hits", Listener->HitCount, 0);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeMesh)
	{
		AddInfo(TEXT("Engine cube mesh unavailable; skipping async trace hit scenarios."));
		return true;
	}

	UWorld* World = DirectiveUtilAsyncTraceTestHelpers::CreateTraceWorld();
	if (!World)
	{
		AddError(TEXT("Failed to create a transient game world for the async trace test."));
		return false;
	}
	DirectiveUtilAsyncTraceTestHelpers::SpawnBlockingCube(World, CubeMesh);

	// Trace straight down through the cube at the origin so every shape intersects it.
	const FVector Start(0.0f, 0.0f, 500.0f);
	const FVector End(0.0f, 0.0f, -500.0f);
	const ETraceTypeQuery Channel = ETraceTypeQuery::TraceTypeQuery1; // Visibility, which BlockAll blocks.

	auto MakeListener = [](UDirectiveUtilTask_AsyncTrace* Task) -> UDirectiveUtilDelegateListener*
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTraceCompleted);
		Task->Activate();
		return Listener;
	};

	TArray<UDirectiveUtilDelegateListener*> Listeners;
	Listeners.Add(MakeListener(UDirectiveUtilTask_AsyncTrace::AsyncLineTraceByChannel(World, Start, End, Channel, false)));
	Listeners.Add(MakeListener(UDirectiveUtilTask_AsyncTrace::AsyncSphereTraceByChannel(World, Start, End, 25.0f, Channel, false)));
	Listeners.Add(MakeListener(UDirectiveUtilTask_AsyncTrace::AsyncBoxTraceByChannel(World, Start, End, FVector(25.0f), FRotator::ZeroRotator, Channel, false)));
	Listeners.Add(MakeListener(UDirectiveUtilTask_AsyncTrace::AsyncCapsuleTraceByChannel(World, Start, End, 25.0f, 50.0f, Channel, false)));

	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickTraceWorld(this, World, Listeners, 120));

	return true;
}

/**
 * Latent command that ticks a move-to-location world each frame until the listener reports
 * completion (or the frame budget runs out), asserts a single failed completion, and tears the
 * world down.
 */
class FDirectiveUtilTickMoveToLocationWorld : public IAutomationLatentCommand
{
public:
	FDirectiveUtilTickMoveToLocationWorld(FAutomationTestBase* InTest, UWorld* InWorld, UDirectiveUtilDelegateListener* InListener, int32 InFrames)
		: Test(InTest)
		, World(InWorld)
		, Listener(InListener)
		, FramesRemaining(InFrames)
	{
	}

	virtual bool Update() override
	{
		if (UWorld* TickWorld = World.Get())
		{
			TickWorld->Tick(LEVELTICK_All, 0.05f);
		}

		if (Listener && !Listener->bCompleted && --FramesRemaining > 0)
		{
			return false;
		}

		if (Listener)
		{
			Test->TestTrue(TEXT("Move without navigation broadcasts Completed"), Listener->bCompleted);
			Test->TestFalse(TEXT("Move without navigation reports failure"), Listener->bLastSuccess);
			Test->TestEqual(TEXT("Move without navigation completes exactly once"), Listener->CompletedCount, 1);
			Listener->Keepalive = nullptr;
			Listener->RemoveFromRoot();
		}

		if (UWorld* TearDownWorld = World.Get())
		{
			GEngine->DestroyWorldContext(TearDownWorld);
			TearDownWorld->DestroyWorld(false);
		}
		return true;
	}

private:
	FAutomationTestBase* Test;
	TWeakObjectPtr<UWorld> World;
	UDirectiveUtilDelegateListener* Listener;
	int32 FramesRemaining;
};

/**
 * DirectiveUtilTask_MoveToLocation: verifies the guard paths (null controller, controller without a pawn) and
 * EndTask all broadcast Completed(false) without crashing, that a second EndTask does not broadcast
 * again, and that a move with no navigation data terminates with failure. The successful navigation
 * path requires a built navigation mesh and is exercised in a project-level test rather than here.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilMoveToLocationTest, "DirectiveUtilities.AsyncTaskMoveToLocationTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMoveToLocationTest::RunTest(const FString& Parameters)
{
	// The guard paths intentionally log a warning.
	AddExpectedMessagePlain(TEXT("Controller or pawn has been destroyed while moving to location. Aborting."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Null controller guard: activating broadcasts Completed(false).
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToLocation* Task = UDirectiveUtilTask_MoveToLocation::MoveToLocation(nullptr, nullptr, FVector(100.0f, 0.0f, 0.0f));
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->Activate();

		TestTrue("Null controller broadcasts Completed", Listener->bCompleted);
		TestFalse("Null controller reports failure", Listener->bLastSuccess);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the move-to-location test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	// Controller-without-pawn guard: activating broadcasts Completed(false).
	{
		APlayerController* Controller = World->SpawnActor<APlayerController>();

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToLocation* Task = UDirectiveUtilTask_MoveToLocation::MoveToLocation(World, Controller, FVector(100.0f, 0.0f, 0.0f));
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->Activate();

		TestTrue("Controller without a pawn broadcasts Completed", Listener->bCompleted);
		TestFalse("Controller without a pawn reports failure", Listener->bLastSuccess);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	// EndTask broadcasts Completed(false) and clears timers without crashing.
	{
		APlayerController* Controller = World->SpawnActor<APlayerController>();

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToLocation* Task = UDirectiveUtilTask_MoveToLocation::MoveToLocation(World, Controller, FVector(100.0f, 0.0f, 0.0f));
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->EndTask();

		TestTrue("EndTask broadcasts Completed", Listener->bCompleted);
		TestFalse("EndTask reports failure", Listener->bLastSuccess);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	// Double completion guard: a second EndTask does not broadcast Completed again.
	{
		APlayerController* Controller = World->SpawnActor<APlayerController>();

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToLocation* Task = UDirectiveUtilTask_MoveToLocation::MoveToLocation(World, Controller, FVector(100.0f, 0.0f, 0.0f));
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->EndTask();
		Task->EndTask();

		TestEqual("Double EndTask broadcasts Completed exactly once", Listener->CompletedCount, 1);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	// No-navigation failure: without a navmesh the idle path-following check terminates the task
	// with failure instead of polling forever.
	{
		// SimpleMoveToLocation may warn when the world has no navigation system.
		AddExpectedMessagePlain(TEXT("SimpleMoveToActor called for NavSys:"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);
		AddExpectedMessagePlain(TEXT("SimpleMove failed for"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

		UWorld* MoveWorld = DirectiveUtilAsyncTraceTestHelpers::CreateTraceWorld();
		if (!MoveWorld)
		{
			AddInfo(TEXT("Failed to create a transient game world; skipping the no-navigation move scenario."));
			return true;
		}

		APlayerController* Controller = MoveWorld->SpawnActor<APlayerController>();
		ADefaultPawn* Pawn = MoveWorld->SpawnActor<ADefaultPawn>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (!Controller || !Pawn)
		{
			AddInfo(TEXT("Failed to spawn a controller or pawn; skipping the no-navigation move scenario."));
			GEngine->DestroyWorldContext(MoveWorld);
			MoveWorld->DestroyWorld(false);
			return true;
		}
		Controller->SetPawn(Pawn);

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToLocation* Task = UDirectiveUtilTask_MoveToLocation::MoveToLocation(MoveWorld, Controller, FVector(10000.0f, 0.0f, 0.0f), 100.0f, false);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->Activate();

		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickMoveToLocationWorld(this, MoveWorld, Listener, 120));
	}

	return true;
}

/**
 * DirectiveUtilTask_MoveToActor: verifies the guard paths (null controller, null goal) broadcast Completed(false)
 * without crashing, that a second EndTask does not broadcast again, and that a move with no
 * navigation data terminates with failure. The successful navigation path requires a built
 * navigation mesh and is exercised in a project-level test rather than here.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilMoveToActorTest, "DirectiveUtilities.AsyncTaskMoveToActorTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMoveToActorTest::RunTest(const FString& Parameters)
{
	// The guard paths intentionally log a warning.
	AddExpectedMessagePlain(TEXT("Controller, pawn, or goal has been destroyed while moving to actor. Aborting."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Null controller guard: activating broadcasts Completed(false).
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToActor* Task = UDirectiveUtilTask_MoveToActor::MoveToActor(nullptr, nullptr, nullptr);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->Activate();

		TestTrue("Null controller broadcasts Completed", Listener->bCompleted);
		TestFalse("Null controller reports failure", Listener->bLastSuccess);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the move-to-actor test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	// Null goal guard: a controller with a pawn but no goal broadcasts Completed(false).
	{
		APlayerController* Controller = World->SpawnActor<APlayerController>();
		ADefaultPawn* Pawn = World->SpawnActor<ADefaultPawn>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Controller && Pawn)
		{
			Controller->SetPawn(Pawn);

			UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
			Listener->AddToRoot();

			UDirectiveUtilTask_MoveToActor* Task = UDirectiveUtilTask_MoveToActor::MoveToActor(World, Controller, nullptr);
			Listener->Keepalive = Task;
			Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
			Task->Activate();

			TestTrue("Null goal broadcasts Completed", Listener->bCompleted);
			TestFalse("Null goal reports failure", Listener->bLastSuccess);

			Listener->Keepalive = nullptr;
			Listener->RemoveFromRoot();
		}
		else
		{
			AddInfo(TEXT("Failed to spawn a controller or pawn; skipping the null-goal scenario."));
		}
	}

	// Double completion guard: a second EndTask does not broadcast Completed again.
	{
		APlayerController* Controller = World->SpawnActor<APlayerController>();

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToActor* Task = UDirectiveUtilTask_MoveToActor::MoveToActor(World, Controller, nullptr);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->EndTask();
		Task->EndTask();

		TestEqual("Double EndTask broadcasts Completed exactly once", Listener->CompletedCount, 1);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	// No-navigation failure: without a navmesh the idle path-following check terminates the task
	// with failure instead of polling forever.
	{
		// SimpleMoveToActor may warn when the world has no navigation system.
		AddExpectedMessagePlain(TEXT("SimpleMoveToActor called for NavSys:"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);
		AddExpectedMessagePlain(TEXT("SimpleMove failed for"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

		UWorld* MoveWorld = DirectiveUtilAsyncTraceTestHelpers::CreateTraceWorld();
		if (!MoveWorld)
		{
			AddInfo(TEXT("Failed to create a transient game world; skipping the no-navigation move scenario."));
			return true;
		}

		APlayerController* Controller = MoveWorld->SpawnActor<APlayerController>();
		ADefaultPawn* Pawn = MoveWorld->SpawnActor<ADefaultPawn>(FVector::ZeroVector, FRotator::ZeroRotator);
		AStaticMeshActor* GoalActor = MoveWorld->SpawnActor<AStaticMeshActor>(FVector(10000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		if (!Controller || !Pawn || !GoalActor)
		{
			AddInfo(TEXT("Failed to spawn a controller, pawn, or goal; skipping the no-navigation move scenario."));
			GEngine->DestroyWorldContext(MoveWorld);
			MoveWorld->DestroyWorld(false);
			return true;
		}
		Controller->SetPawn(Pawn);

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_MoveToActor* Task = UDirectiveUtilTask_MoveToActor::MoveToActor(MoveWorld, Controller, GoalActor, 100.0f, false);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBoolCompleted);
		Task->Activate();

		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickMoveToLocationWorld(this, MoveWorld, Listener, 120));
	}

	return true;
}

#endif // WITH_EDITOR
