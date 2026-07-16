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

namespace DirectiveUtilAsyncTaskTestHelpers
{
	UDirectiveUtilDelegateListener* StartDelayScenario(float Duration, bool bCancel)
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		if (!World)
		{
			return nullptr;
		}
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilDelayTaskTest, "DirectiveUtilities.AsyncTaskDelayTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilDelayTaskTest::RunTest(const FString& Parameters)
{
	AddExpectedMessagePlain(TEXT("Cancellable Delay failed to activate. World is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	if (UDirectiveUtilDelegateListener* Completed = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Completed, 600, true));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay completion scenario."));
	}

	if (UDirectiveUtilDelegateListener* Cancelled = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, true))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Cancelled, 10, false));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay cancellation scenario."));
	}

	UDirectiveUtilTask_Delay* NullWorldTask = UDirectiveUtilTask_Delay::CancellableDelay(nullptr, 0.05f);
	if (TestNotNull("CancellableDelay returns a task even with a null context", NullWorldTask))
	{
		NullWorldTask->AddToRoot();
		NullWorldTask->Activate();
		NullWorldTask->RemoveFromRoot();
	}

	return true;
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadAssetTest, "DirectiveUtilities.AsyncTaskLoadAssetTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadAssetTest::RunTest(const FString& Parameters)
{
	AddExpectedMessagePlain(TEXT("Async Load Asset failed to activate. The soft object reference is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

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

	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		const TSoftObjectPtr<UObject> SoftCube(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
		UDirectiveUtilTask_AsyncLoadAsset* Task = UDirectiveUtilTask_AsyncLoadAsset::AsyncLoadAsset(nullptr, SoftCube);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnObjectFailed);
		Task->Activate();

		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForAsyncLoad(this, Listener, 600));
	}

	return true;
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadAssetsTest, "DirectiveUtilities.AsyncTaskLoadAssetsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadAssetsTest::RunTest(const FString& Parameters)
{
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

		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForBatchAsyncLoad(this, Listener, 600));
	}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilAsyncLoadClassTest, "DirectiveUtilities.AsyncTaskLoadClassTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncLoadClassTest::RunTest(const FString& Parameters)
{
	AddExpectedMessagePlain(TEXT("Async Load Class failed to activate. The soft class reference is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

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

	{
		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();

		const TSoftClassPtr<UObject> SoftClass(AStaticMeshActor::StaticClass());
		UDirectiveUtilTask_AsyncLoadClass* Task = UDirectiveUtilTask_AsyncLoadClass::AsyncLoadClass(nullptr, SoftClass);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnClassFailed);
		Task->Activate();

		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilWaitForAsyncLoad(this, Listener, 600));
	}

	return true;
}
