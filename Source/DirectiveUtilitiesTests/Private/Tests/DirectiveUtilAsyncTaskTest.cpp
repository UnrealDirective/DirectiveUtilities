// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_Delay.h"
#include "Tasks/DirectiveUtilTask_AsyncLoadAsset.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR

namespace DirectiveUtilAsyncTaskTestHelpers
{
	/**
	 * Spawns a transient world, starts a cancellable delay in it, and returns a rooted listener bound
	 * to the delay's Completed delegate. The world is stored on the listener so the latent command can
	 * tick its timer manager across frames and tear it down once the scenario settles.
	 *
	 * A timer set on a never-ticked FTimerManager is queued as pending and only promoted to active on
	 * the first tick; it fires on a later tick. Because a manager can be ticked at most once per frame,
	 * driving the timer to completion requires advancing real frames, hence the latent command below.
	 */
	UDirectiveUtilDelegateListener* StartDelayScenario(float Duration, bool bCancel)
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
		if (!World)
		{
			return nullptr;
		}
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
		WorldContext.SetCurrentWorld(World);

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();
		Listener->ScenarioWorld = World;

		UDirectiveUtilTask_Delay* Task = UDirectiveUtilTask_Delay::CancellableDelay(World, Duration);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnCompleted);
		Task->Activate();

		if (bCancel)
		{
			Task->EndTask();
		}

		return Listener;
	}
}

/**
 * Latent command that ticks a delay scenario's world each frame until the delay completes or the
 * frame budget runs out, then asserts against the expected outcome and tears the world down.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FDirectiveUtilTickDelayScenario, FAutomationTestBase*, Test, UDirectiveUtilDelegateListener*, Listener, int32, FramesRemaining, bool, bExpectComplete);

bool FDirectiveUtilTickDelayScenario::Update()
{
	if (!Listener)
	{
		return true;
	}

	UWorld* World = Listener->ScenarioWorld.Get();
	if (World)
	{
		World->GetTimerManager().Tick(0.1f);
	}

	const bool bBudgetExhausted = (--FramesRemaining <= 0);
	if (Listener->bCompleted || bBudgetExhausted)
	{
		if (bExpectComplete)
		{
			Test->TestTrue(TEXT("Cancellable delay broadcasts Completed once its timer elapses"), Listener->bCompleted);
			Test->TestEqual(TEXT("Cancellable delay broadcasts Completed exactly once"), Listener->CompletedCount, 1);
		}
		else
		{
			Test->TestFalse(TEXT("EndTask cancels the delay so Completed never fires"), Listener->bCompleted);
		}

		if (World)
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
		Listener->ScenarioWorld = nullptr;
		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
		return true;
	}

	return false;
}

/**
 * DirectiveUtilTask_Delay: verifies the timer-backed completion fires, that EndTask cancels it before it fires,
 * and that activating with a null world context is guarded rather than crashing.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilDelayTaskTest, "DirectiveUtilities.AsyncTaskDelayTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilDelayTaskTest::RunTest(const FString& Parameters)
{
	// The null-world activation intentionally logs a warning.
	AddExpectedMessagePlain(TEXT("Cancellable Delay failed to activate. World is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Completion path: the delay fires once its timer elapses (driven across frames by the latent command).
	if (UDirectiveUtilDelegateListener* Completed = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, /*bCancel=*/false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Completed, 600, /*bExpectComplete=*/true));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay completion scenario."));
	}

	// Cancellation path: EndTask clears the timer so Completed never fires across a short window.
	if (UDirectiveUtilDelegateListener* Cancelled = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, /*bCancel=*/true))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Cancelled, 10, /*bExpectComplete=*/false));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay cancellation scenario."));
	}

	// Null world guard: activating with a null context object logs a warning and does not crash.
	UDirectiveUtilTask_Delay* NullWorldTask = UDirectiveUtilTask_Delay::CancellableDelay(nullptr, 0.05f);
	if (TestNotNull("CancellableDelay returns a task even with a null context", NullWorldTask))
	{
		NullWorldTask->AddToRoot();
		NullWorldTask->Activate();
		NullWorldTask->RemoveFromRoot();
	}

	return true;
}

/** Latent command that waits for an async-load listener to settle (completed or failed) or times out. */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FDirectiveUtilWaitForAsyncLoad, FAutomationTestBase*, Test, UDirectiveUtilDelegateListener*, Listener, int32, FramesRemaining);

bool FDirectiveUtilWaitForAsyncLoad::Update()
{
	if (!Listener)
	{
		return true;
	}

	if (Listener->bFailed)
	{
		Test->AddError(TEXT("Async Load Asset reported failure while loading a valid engine asset."));
		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
		return true;
	}

	if (Listener->bCompleted)
	{
		Test->TestNotNull(TEXT("Async Load Asset resolved the requested asset"), Listener->LastObject.Get());
		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
		return true;
	}

	if (--FramesRemaining <= 0)
	{
		Test->AddError(TEXT("Async Load Asset did not complete within the frame budget."));
		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
		return true;
	}

	return false;
}

/**
 * DirectiveUtilTask_AsyncLoadAsset: verifies the null soft-reference path broadcasts Failed synchronously, and
 * that loading a real engine asset eventually broadcasts Completed with the resolved object.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadAssetTest, "DirectiveUtilities.AsyncTaskLoadAssetTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadAssetTest::RunTest(const FString& Parameters)
{
	// The null soft-reference path intentionally logs a warning.
	AddExpectedMessagePlain(TEXT("Async Load Asset failed to activate. The soft object reference is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Failure path: a null soft reference broadcasts Failed synchronously on activation.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_AsyncLoadAsset* Task = UDirectiveUtilTask_AsyncLoadAsset::AsyncLoadAsset(nullptr, TSoftObjectPtr<UObject>());
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectFailed);
		Task->Activate();

		TestTrue("Null soft reference broadcasts Failed", Listener->bFailed);
		TestFalse("Null soft reference does not broadcast Completed", Listener->bCompleted);

		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
	}

	// Success path: loading a real engine asset broadcasts Completed with the resolved object.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		const TSoftObjectPtr<UObject> SoftCube(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
		UDirectiveUtilTask_AsyncLoadAsset* Task = UDirectiveUtilTask_AsyncLoadAsset::AsyncLoadAsset(nullptr, SoftCube);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectFailed);
		Task->Activate();

		// The streamable completion delegate fires on a later engine tick; poll until it settles.
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForAsyncLoad(this, Listener, 600));
	}

	return true;
}

/** Latent command that waits for a batch async-load listener to complete, then asserts the slot contract. */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FDirectiveUtilWaitForBatchAsyncLoad, FAutomationTestBase*, Test, UDirectiveUtilDelegateListener*, Listener, int32, FramesRemaining);

bool FDirectiveUtilWaitForBatchAsyncLoad::Update()
{
	if (!Listener)
	{
		return true;
	}

	if (Listener->bCompleted)
	{
		Test->TestEqual(TEXT("Async Load Assets broadcasts Completed exactly once"), Listener->CompletedCount, 1);
		Test->TestEqual(TEXT("Async Load Assets preserves the input slot count"), Listener->LastObjects.Num(), 3);
		if (Listener->LastObjects.Num() == 3)
		{
			Test->TestNotNull(TEXT("Async Load Assets resolves the first asset"), Listener->LastObjects[0].Get());
			Test->TestNotNull(TEXT("Async Load Assets resolves the second asset"), Listener->LastObjects[1].Get());
			Test->TestNull(TEXT("Async Load Assets keeps a null slot for an unset reference"), Listener->LastObjects[2].Get());
		}
		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
		return true;
	}

	if (--FramesRemaining <= 0)
	{
		Test->AddError(TEXT("Async Load Assets did not complete within the frame budget."));
		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
		return true;
	}

	return false;
}

/** Latent command that waits a fixed frame window and then asserts a cancelled batch load never completed. */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FDirectiveUtilVerifyCancelledBatchLoad, FAutomationTestBase*, Test, UDirectiveUtilDelegateListener*, Listener, int32, FramesRemaining);

bool FDirectiveUtilVerifyCancelledBatchLoad::Update()
{
	if (!Listener)
	{
		return true;
	}

	if (--FramesRemaining > 0)
	{
		return false;
	}

	Test->TestFalse(TEXT("Cancel prevents the batch Completed broadcast"), Listener->bCompleted);
	Listener->RemoveFromRoot();
	Listener->Keepalive = nullptr;
	return true;
}

/**
 * DirectiveUtilTask_AsyncLoadAssets: verifies a batch resolves in input order with null slots for unset references,
 * that an empty input completes immediately with an empty array, and that Cancel suppresses Completed.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadAssetsTest, "DirectiveUtilities.AsyncTaskLoadAssetsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadAssetsTest::RunTest(const FString& Parameters)
{
	// Empty input: Completed broadcasts synchronously on activation with an empty array.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_AsyncLoadAssets* Task = UDirectiveUtilTask_AsyncLoadAssets::AsyncLoadAssets(nullptr, TArray<TSoftObjectPtr<UObject>>());
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectsCompleted);
		Task->Activate();

		TestTrue("Empty batch broadcasts Completed immediately", Listener->bCompleted);
		TestEqual("Empty batch broadcasts Completed exactly once", Listener->CompletedCount, 1);
		TestEqual("Empty batch reports an empty array", Listener->LastObjects.Num(), 0);

		Listener->Keepalive = nullptr;
		Listener->RemoveFromRoot();
	}

	// Batch path: two valid engine meshes plus an unset reference resolve in input order.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		TArray<TSoftObjectPtr<UObject>> Assets;
		Assets.Add(TSoftObjectPtr<UObject>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube"))));
		Assets.Add(TSoftObjectPtr<UObject>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere.Sphere"))));
		Assets.Add(TSoftObjectPtr<UObject>());

		UDirectiveUtilTask_AsyncLoadAssets* Task = UDirectiveUtilTask_AsyncLoadAssets::AsyncLoadAssets(nullptr, Assets);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectsCompleted);
		Task->Activate();

		// The streamable completion delegate fires on a later engine tick; poll until it settles.
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForBatchAsyncLoad(this, Listener, 600));
	}

	// Cancel path: cancelling before completion suppresses the Completed broadcast. Uses a mesh no
	// other test loads so the request is genuinely in flight when Cancel arrives; if the asset is
	// already in memory the batch completes synchronously and there is nothing left to cancel.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		TArray<TSoftObjectPtr<UObject>> Assets;
		Assets.Add(TSoftObjectPtr<UObject>(FSoftObjectPath(TEXT("/Engine/EngineMeshes/SM_MatPreviewMesh_01.SM_MatPreviewMesh_01"))));

		UDirectiveUtilTask_AsyncLoadAssets* Task = UDirectiveUtilTask_AsyncLoadAssets::AsyncLoadAssets(nullptr, Assets);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectsCompleted);
		Task->Activate();

		if (Listener->bCompleted)
		{
			AddInfo(TEXT("Batch load completed synchronously (asset already in memory); skipping the cancel scenario."));
			Listener->Keepalive = nullptr;
			Listener->RemoveFromRoot();
		}
		else
		{
			Task->Cancel();
			ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilVerifyCancelledBatchLoad(this, Listener, 10));
		}
	}

	return true;
}

/**
 * DirectiveUtilTask_AsyncLoadClass: verifies the null soft-class path broadcasts Failed synchronously, and that
 * loading a real class broadcasts Completed with the resolved class.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadClassTest, "DirectiveUtilities.AsyncTaskLoadClassTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadClassTest::RunTest(const FString& Parameters)
{
	// The null soft-class path intentionally logs a warning.
	AddExpectedMessagePlain(TEXT("Async Load Class failed to activate. The soft class reference is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	// Failure path: a null soft class reference broadcasts Failed synchronously on activation.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		UDirectiveUtilTask_AsyncLoadClass* Task = UDirectiveUtilTask_AsyncLoadClass::AsyncLoadClass(nullptr, TSoftClassPtr<UObject>());
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassFailed);
		Task->Activate();

		TestTrue("Null soft class broadcasts Failed", Listener->bFailed);
		TestFalse("Null soft class does not broadcast Completed", Listener->bCompleted);

		Listener->RemoveFromRoot();
		Listener->Keepalive = nullptr;
	}

	// Success path: loading a real class broadcasts Completed with the resolved class.
	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		const TSoftClassPtr<UObject> SoftClass(AStaticMeshActor::StaticClass());
		UDirectiveUtilTask_AsyncLoadClass* Task = UDirectiveUtilTask_AsyncLoadClass::AsyncLoadClass(nullptr, SoftClass);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassFailed);
		Task->Activate();

		// The streamable completion delegate fires on a later engine tick; poll until it settles.
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForAsyncLoad(this, Listener, 600));
	}

	return true;
}

#endif // WITH_EDITOR
