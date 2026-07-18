// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tasks/DirectiveUtilTask_Delay.h"
#include "Tasks/DirectiveUtilTask_Flow.h"
#include "Tasks/DirectiveUtilTask_AsyncLoadAsset.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace DirectiveUtilAsyncTaskTestHelpers
{
	UDirectiveUtilDelegateListener* CreateScenarioListener()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
		if (!GameInstance)
		{
			return nullptr;
		}
		GameInstance->AddToRoot();
		GameInstance->InitializeStandalone();
		UWorld* World = GameInstance->GetWorld();
		if (!World)
		{
			GameInstance->Shutdown();
			GameInstance->RemoveFromRoot();
			return nullptr;
		}

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();
		Listener->ScenarioWorld = World;
		Listener->ScenarioGameInstance = GameInstance;
		return Listener;
	}

	void DestroyScenario(UDirectiveUtilDelegateListener* Listener)
	{
		if (!Listener)
		{
			return;
		}

		UGameInstance* GameInstance = Listener->ScenarioGameInstance.Get();
		Listener->Keepalive = nullptr;
		Listener->ScenarioWorld = nullptr;
		Listener->ScenarioGameInstance = nullptr;
		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance->RemoveFromRoot();
		}
		Listener->RemoveFromRoot();
	}

	UDirectiveUtilDelegateListener* StartDelayScenario(float Duration, bool bCancel, bool bUseEndTask = false)
	{
		UDirectiveUtilDelegateListener* Listener = CreateScenarioListener();
		if (!Listener)
		{
			return nullptr;
		}

		UDirectiveUtilTask_Delay* Task = UDirectiveUtilTask_Delay::CancellableDelay(Listener->ScenarioWorld, Duration);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnCompleted);
		Task->Activate();

		if (bCancel)
		{
			if (bUseEndTask)
			{
				Task->EndTask();
			}
			else
			{
				Task->Cancel();
			}
		}

		return Listener;
	}

	UDirectiveUtilDelegateListener* StartDurationScenario(float Duration, float UpdateInterval, bool bCancel)
	{
		UDirectiveUtilDelegateListener* Listener = CreateScenarioListener();
		if (!Listener)
		{
			return nullptr;
		}

		UDirectiveUtilTask_UpdateForDuration* Task = UDirectiveUtilTask_UpdateForDuration::UpdateForDuration(
			Listener->ScenarioWorld,
			Duration,
			UpdateInterval);
		Listener->Keepalive = Task;
		Task->Updated.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnDurationUpdated);
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnCompleted);
		Task->Activate();
		if (bCancel)
		{
			Task->Cancel();
		}
		return Listener;
	}

	UDirectiveUtilDelegateListener* StartRepeatScenario(int32 Count, float Interval, float InitialDelay, bool bCancel)
	{
		UDirectiveUtilDelegateListener* Listener = CreateScenarioListener();
		if (!Listener)
		{
			return nullptr;
		}

		UDirectiveUtilTask_RepeatWithInterval* Task = UDirectiveUtilTask_RepeatWithInterval::RepeatWithInterval(
			Listener->ScenarioWorld,
			Count,
			Interval,
			InitialDelay);
		Listener->Keepalive = Task;
		Task->Iteration.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnRepeatIteration);
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnCompleted);
		Task->Activate();
		if (bCancel)
		{
			Task->Cancel();
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
			Test->TestFalse(TEXT("Cancel prevents the delay from firing Completed"), Listener->bCompleted);
		}

		DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
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
		if (const UDirectiveUtilTask_Delay* Task = Cast<UDirectiveUtilTask_Delay>(Completed->Keepalive))
		{
			TestTrue(TEXT("Cancellable delay is active after activation"), Task->IsActive());
		}
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Completed, 600, true));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay completion scenario."));
	}

	if (UDirectiveUtilDelegateListener* Cancelled = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, true))
	{
		if (UDirectiveUtilTask_Delay* Task = Cast<UDirectiveUtilTask_Delay>(Cancelled->Keepalive))
		{
			TestFalse(TEXT("Cancelled delay is inactive"), Task->IsActive());
			Task->Cancel();
		}
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Cancelled, 10, false));
	}
	else
	{
		AddError(TEXT("Failed to create a transient world for the delay cancellation scenario."));
	}

	if (UDirectiveUtilDelegateListener* LegacyCancelled = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.05f, true, true))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, LegacyCancelled, 10, false));
	}
	else
	{
		AddError(TEXT("Failed to create the EndTask compatibility scenario."));
	}

	if (UDirectiveUtilDelegateListener* Immediate = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(0.0f, false))
	{
		TestFalse(TEXT("A non-positive delay does not complete during activation"), Immediate->bCompleted);
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, Immediate, 3, true));
	}
	else
	{
		AddError(TEXT("Failed to create the immediate delay scenario."));
	}

	if (UDirectiveUtilDelegateListener* NonFinite = DirectiveUtilAsyncTaskTestHelpers::StartDelayScenario(
		std::numeric_limits<float>::quiet_NaN(),
		false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDelayScenario(this, NonFinite, 3, true));
	}
	else
	{
		AddError(TEXT("Failed to create the non-finite delay scenario."));
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

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FDirectiveUtilTickDurationCadenceScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	double, Deadline);

bool FDirectiveUtilTickDurationCadenceScenario::Update()
{
	if (!Listener)
	{
		Listener = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(0.1f, 0.02f, false);
		Deadline = FPlatformTime::Seconds() + 2.0;
		if (!Listener)
		{
			Test->AddError(TEXT("Failed to create the Update for Duration cadence scenario."));
			return true;
		}
		return false;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (!TimerManager.HasBeenTickedThisFrame())
		{
			TimerManager.Tick(0.01f);
		}
	}

	if (!Listener->bCompleted && FPlatformTime::Seconds() < Deadline)
	{
		return false;
	}

	Test->TestTrue(TEXT("Update for Duration completes with a positive update interval"), Listener->bCompleted);
	Test->TestTrue(TEXT("Update for Duration emits repeated interval updates"), Listener->UpdatedCount >= 4);
	Test->TestTrue(TEXT("Update for Duration does not emit excessive interval updates"), Listener->UpdatedCount <= 7);
	Test->TestTrue(TEXT("Update for Duration emits progress before its final update"), Listener->UpdateAlphas.Num() > 1 && Listener->UpdateAlphas[0] > 0.0f && Listener->UpdateAlphas[0] < 1.0f);
	Test->TestEqual(TEXT("Update for Duration interval scenario completes once"), Listener->CompletedCount, 1);
	if (const UDirectiveUtilTask_UpdateForDuration* Task = Cast<UDirectiveUtilTask_UpdateForDuration>(Listener->Keepalive))
	{
		Test->TestFalse(TEXT("Completed Update for Duration is inactive"), Task->IsActive());
	}
	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FDirectiveUtilCancelDurationAfterUpdateScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	double, Deadline,
	int32, UpdatesAtCancellation);

bool FDirectiveUtilCancelDurationAfterUpdateScenario::Update()
{
	if (!Listener)
	{
		Listener = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(0.5f, 0.02f, false);
		Deadline = FPlatformTime::Seconds() + 2.0;
		if (!Listener)
		{
			Test->AddError(TEXT("Failed to create the Update for Duration cancellation scenario."));
			return true;
		}
		return false;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (!TimerManager.HasBeenTickedThisFrame())
		{
			TimerManager.Tick(0.01f);
		}
	}

	if (UpdatesAtCancellation == 0 && Listener->UpdatedCount > 0)
	{
		UpdatesAtCancellation = Listener->UpdatedCount;
		if (UDirectiveUtilTask_UpdateForDuration* Task = Cast<UDirectiveUtilTask_UpdateForDuration>(Listener->Keepalive))
		{
			Task->Cancel();
		}
		Deadline = FPlatformTime::Seconds() + 0.05;
	}

	if (FPlatformTime::Seconds() < Deadline)
	{
		return false;
	}

	Test->TestTrue(TEXT("Update for Duration reached an update before cancellation"), UpdatesAtCancellation > 0);
	Test->TestFalse(TEXT("Cancelling Update for Duration after an update prevents completion"), Listener->bCompleted);
	Test->TestEqual(TEXT("Cancelling Update for Duration stops later updates"), Listener->UpdatedCount, UpdatesAtCancellation);
	if (const UDirectiveUtilTask_UpdateForDuration* Task = Cast<UDirectiveUtilTask_UpdateForDuration>(Listener->Keepalive))
	{
		Test->TestFalse(TEXT("Cancelled Update for Duration is inactive"), Task->IsActive());
	}
	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(
	FDirectiveUtilTickDurationScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	int32, FramesRemaining,
	bool, bExpectComplete,
	float, ExpectedDuration);

bool FDirectiveUtilTickDurationScenario::Update()
{
	if (!Listener)
	{
		return true;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		World->GetTimerManager().Tick(0.1f);
	}

	const bool bBudgetExhausted = --FramesRemaining <= 0;
	if (!Listener->bCompleted && !bBudgetExhausted)
	{
		return false;
	}

	if (bExpectComplete)
	{
		Test->TestTrue(TEXT("Update for Duration fires Completed"), Listener->bCompleted);
		Test->TestEqual(TEXT("Update for Duration fires Completed once"), Listener->CompletedCount, 1);
		Test->TestTrue(TEXT("Update for Duration emits at least one update"), Listener->UpdatedCount > 0);
		Test->TestEqual(TEXT("Update for Duration ends at alpha 1"), Listener->LastAlpha, 1.0f);
		Test->TestEqual(TEXT("Update for Duration reports its final elapsed time"), Listener->LastElapsedTime, FMath::Max(ExpectedDuration, 0.0f));
		float PreviousElapsedTime = 0.0f;
		for (int32 UpdateIndex = 0; UpdateIndex < Listener->UpdatedCount; ++UpdateIndex)
		{
			Test->TestTrue(TEXT("Update for Duration elapsed time is monotonic"), Listener->UpdateElapsedTimes[UpdateIndex] >= PreviousElapsedTime);
			Test->TestTrue(TEXT("Update for Duration delta time is non-negative"), Listener->UpdateDeltaTimes[UpdateIndex] >= 0.0f);
			Test->TestTrue(TEXT("Update for Duration alpha stays in range"), Listener->UpdateAlphas[UpdateIndex] >= 0.0f && Listener->UpdateAlphas[UpdateIndex] <= 1.0f);
			PreviousElapsedTime = Listener->UpdateElapsedTimes[UpdateIndex];
		}
	}
	else
	{
		Test->TestFalse(TEXT("Cancel prevents Update for Duration from firing Completed"), Listener->bCompleted);
		Test->TestEqual(TEXT("Cancel prevents Update for Duration callbacks"), Listener->UpdatedCount, 0);
	}

	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilUpdateForDurationTaskTest,
	"DirectiveUtilities.AsyncTaskUpdateForDurationTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilUpdateForDurationTaskTest::RunTest(const FString& Parameters)
{
	AddExpectedMessagePlain(TEXT("Update for Duration failed to activate. World is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	if (UDirectiveUtilDelegateListener* Completed = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(0.01f, 0.002f, false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDurationScenario(this, Completed, 600, true, 0.01f));
	}
	else
	{
		AddError(TEXT("Failed to create the Update for Duration completion scenario."));
	}

	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDurationCadenceScenario(this, nullptr, 0.0));
	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilCancelDurationAfterUpdateScenario(this, nullptr, 0.0, 0));

	if (UDirectiveUtilDelegateListener* Immediate = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(0.0f, 0.0f, false))
	{
		TestFalse(TEXT("Update for Duration does not complete during activation"), Immediate->bCompleted);
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDurationScenario(this, Immediate, 3, true, 0.0f));
	}
	else
	{
		AddError(TEXT("Failed to create the immediate Update for Duration scenario."));
	}

	if (UDirectiveUtilDelegateListener* NonFinite = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(std::numeric_limits<float>::quiet_NaN(), 0.0f, false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDurationScenario(this, NonFinite, 3, true, 0.0f));
	}
	else
	{
		AddError(TEXT("Failed to create the non-finite Update for Duration scenario."));
	}

	if (UDirectiveUtilDelegateListener* Cancelled = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(1.0f, 0.0f, true))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickDurationScenario(this, Cancelled, 3, false, 1.0f));
	}
	else
	{
		AddError(TEXT("Failed to create the cancelled Update for Duration scenario."));
	}

	if (UDirectiveUtilDelegateListener* PerTick = DirectiveUtilAsyncTaskTestHelpers::StartDurationScenario(10.0f, 0.0f, false))
	{
		if (UWorld* World = PerTick->ScenarioWorld.Get())
		{
			World->GetTimerManager().Tick(1.0f);
			TestEqual(TEXT("A zero update interval emits at most one update per timer tick"), PerTick->UpdatedCount, 1);
		}
		if (UDirectiveUtilTask_UpdateForDuration* Task = Cast<UDirectiveUtilTask_UpdateForDuration>(PerTick->Keepalive))
		{
			Task->Cancel();
		}
		DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(PerTick);
	}
	else
	{
		AddError(TEXT("Failed to create the per-tick Update for Duration scenario."));
	}

	UDirectiveUtilTask_UpdateForDuration* NullWorldTask = UDirectiveUtilTask_UpdateForDuration::UpdateForDuration(nullptr, 1.0f);
	if (TestNotNull("UpdateForDuration returns a task with a null context", NullWorldTask))
	{
		NullWorldTask->AddToRoot();
		NullWorldTask->Activate();
		NullWorldTask->RemoveFromRoot();
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(
	FDirectiveUtilTickRepeatCadenceScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	double, Deadline,
	int32, TickCount,
	TArray<int32>, IterationTicks);

bool FDirectiveUtilTickRepeatCadenceScenario::Update()
{
	if (!Listener)
	{
		Listener = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(3, 0.03f, 0.04f, false);
		Deadline = FPlatformTime::Seconds() + 2.0;
		if (!Listener)
		{
			Test->AddError(TEXT("Failed to create the Repeat with Interval cadence scenario."));
			return true;
		}
		return false;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (!TimerManager.HasBeenTickedThisFrame())
		{
			TimerManager.Tick(0.01f);
		}
	}
	++TickCount;
	while (IterationTicks.Num() < Listener->IterationCount)
	{
		IterationTicks.Add(TickCount);
	}

	if (!Listener->bCompleted && FPlatformTime::Seconds() < Deadline)
	{
		return false;
	}

	Test->TestTrue(TEXT("Repeat with Interval completes with positive delays"), Listener->bCompleted);
	Test->TestEqual(TEXT("Repeat with Interval emits every delayed iteration"), IterationTicks.Num(), 3);
	if (IterationTicks.Num() == 3)
	{
		Test->TestTrue(TEXT("Repeat with Interval respects its initial delay"), IterationTicks[0] > 1);
		Test->TestTrue(TEXT("Repeat with Interval separates its first interval"), IterationTicks[1] > IterationTicks[0]);
		Test->TestTrue(TEXT("Repeat with Interval separates its second interval"), IterationTicks[2] > IterationTicks[1]);
	}
	Test->TestEqual(TEXT("Repeat with Interval delayed scenario completes once"), Listener->CompletedCount, 1);
	if (const UDirectiveUtilTask_RepeatWithInterval* Task = Cast<UDirectiveUtilTask_RepeatWithInterval>(Listener->Keepalive))
	{
		Test->TestFalse(TEXT("Completed Repeat with Interval is inactive"), Task->IsActive());
	}
	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FDirectiveUtilCancelRepeatAfterIterationScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	double, Deadline,
	int32, IterationsAtCancellation);

bool FDirectiveUtilCancelRepeatAfterIterationScenario::Update()
{
	if (!Listener)
	{
		Listener = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(10, 0.02f, 0.0f, false);
		Deadline = FPlatformTime::Seconds() + 2.0;
		if (!Listener)
		{
			Test->AddError(TEXT("Failed to create the Repeat with Interval cancellation scenario."));
			return true;
		}
		return false;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (!TimerManager.HasBeenTickedThisFrame())
		{
			TimerManager.Tick(0.01f);
		}
	}

	if (IterationsAtCancellation == 0 && Listener->IterationCount > 0)
	{
		IterationsAtCancellation = Listener->IterationCount;
		if (UDirectiveUtilTask_RepeatWithInterval* Task = Cast<UDirectiveUtilTask_RepeatWithInterval>(Listener->Keepalive))
		{
			Task->Cancel();
		}
		Deadline = FPlatformTime::Seconds() + 0.05;
	}

	if (FPlatformTime::Seconds() < Deadline)
	{
		return false;
	}

	Test->TestTrue(TEXT("Repeat with Interval reached an iteration before cancellation"), IterationsAtCancellation > 0);
	Test->TestFalse(TEXT("Cancelling Repeat with Interval after an iteration prevents completion"), Listener->bCompleted);
	Test->TestEqual(TEXT("Cancelling Repeat with Interval stops later iterations"), Listener->IterationCount, IterationsAtCancellation);
	if (const UDirectiveUtilTask_RepeatWithInterval* Task = Cast<UDirectiveUtilTask_RepeatWithInterval>(Listener->Keepalive))
	{
		Test->TestFalse(TEXT("Cancelled Repeat with Interval is inactive"), Task->IsActive());
	}
	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(
	FDirectiveUtilTickRepeatScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	int32, FramesRemaining,
	int32, ExpectedIterations,
	bool, bExpectComplete);

bool FDirectiveUtilTickRepeatScenario::Update()
{
	if (!Listener)
	{
		return true;
	}

	if (UWorld* World = Listener->ScenarioWorld.Get())
	{
		World->GetTimerManager().Tick(0.1f);
	}

	const bool bBudgetExhausted = --FramesRemaining <= 0;
	if (!Listener->bCompleted && !bBudgetExhausted)
	{
		return false;
	}

	if (bExpectComplete)
	{
		Test->TestTrue(TEXT("Repeat with Interval fires Completed"), Listener->bCompleted);
		Test->TestEqual(TEXT("Repeat with Interval fires Completed once"), Listener->CompletedCount, 1);
		Test->TestEqual(TEXT("Repeat with Interval emits the requested iteration count"), Listener->IterationCount, ExpectedIterations);
		TArray<int32> ExpectedIndices;
		TArray<int32> ExpectedRemaining;
		for (int32 Index = 0; Index < ExpectedIterations; ++Index)
		{
			ExpectedIndices.Add(Index);
			ExpectedRemaining.Add(ExpectedIterations - Index - 1);
		}
		Test->TestEqual(TEXT("Repeat with Interval reports zero-based indices"), Listener->IterationIndices, ExpectedIndices);
		Test->TestEqual(TEXT("Repeat with Interval reports remaining iterations"), Listener->IterationRemaining, ExpectedRemaining);
	}
	else
	{
		Test->TestFalse(TEXT("Cancel prevents Repeat with Interval from firing Completed"), Listener->bCompleted);
		Test->TestEqual(TEXT("Cancel prevents Repeat with Interval callbacks"), Listener->IterationCount, 0);
	}

	DirectiveUtilAsyncTaskTestHelpers::DestroyScenario(Listener);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilRepeatWithIntervalTaskTest,
	"DirectiveUtilities.AsyncTaskRepeatWithIntervalTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilRepeatWithIntervalTaskTest::RunTest(const FString& Parameters)
{
	AddExpectedMessagePlain(TEXT("Repeat with Interval failed to activate. World is null."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	if (UDirectiveUtilDelegateListener* Repeated = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(3, 0.0f, 0.0f, false))
	{
		TestEqual(TEXT("Repeat with Interval does not iterate during activation"), Repeated->IterationCount, 0);
		if (UWorld* World = Repeated->ScenarioWorld.Get())
		{
			World->GetTimerManager().Tick(1.0f);
			TestEqual(TEXT("A zero interval emits at most one iteration per timer tick"), Repeated->IterationCount, 1);
		}
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, Repeated, 6, 3, true));
	}
	else
	{
		AddError(TEXT("Failed to create the Repeat with Interval scenario."));
	}

	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatCadenceScenario(this, nullptr, 0.0, 0, TArray<int32>()));
	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilCancelRepeatAfterIterationScenario(this, nullptr, 0.0, 0));

	if (UDirectiveUtilDelegateListener* Empty = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(0, 0.1f, 0.1f, false))
	{
		TestFalse(TEXT("An empty Repeat with Interval does not complete during activation"), Empty->bCompleted);
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, Empty, 3, 0, true));
	}
	else
	{
		AddError(TEXT("Failed to create the empty Repeat with Interval scenario."));
	}

	if (UDirectiveUtilDelegateListener* NegativeCount = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(-4, 0.1f, 0.1f, false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, NegativeCount, 3, 0, true));
	}
	else
	{
		AddError(TEXT("Failed to create the negative-count Repeat with Interval scenario."));
	}

	if (UDirectiveUtilDelegateListener* NegativeTiming = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(2, -1.0f, -1.0f, false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, NegativeTiming, 5, 2, true));
	}
	else
	{
		AddError(TEXT("Failed to create the negative-timing Repeat with Interval scenario."));
	}

	if (UDirectiveUtilDelegateListener* NonFinite = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(
		2,
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::quiet_NaN(),
		false))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, NonFinite, 5, 2, true));
	}
	else
	{
		AddError(TEXT("Failed to create the non-finite Repeat with Interval scenario."));
	}

	if (UDirectiveUtilDelegateListener* Cancelled = DirectiveUtilAsyncTaskTestHelpers::StartRepeatScenario(3, 0.0f, 0.0f, true))
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilTickRepeatScenario(this, Cancelled, 3, 0, false));
	}
	else
	{
		AddError(TEXT("Failed to create the cancelled Repeat with Interval scenario."));
	}

	UDirectiveUtilTask_RepeatWithInterval* NullWorldTask = UDirectiveUtilTask_RepeatWithInterval::RepeatWithInterval(nullptr, 1, 0.0f);
	if (TestNotNull("RepeatWithInterval returns a task with a null context", NullWorldTask))
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
