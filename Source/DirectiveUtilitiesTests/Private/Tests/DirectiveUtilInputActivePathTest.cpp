// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilInputFunctionLibrary.h"
#include "Types/DirectiveUtilInputTypes.h"
#include "Types/DirectiveUtilTypes.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR

/**
 * Exercises the Enhanced Input library's active paths against a real EnhancedInput subsystem,
 * which requires a live local player. A standalone game instance + local player is built for this;
 * if that cannot be created in the headless harness the test skips its assertions rather than failing.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilInputActivePathTest, "DirectiveUtilities.InputActivePathTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilInputActivePathTest::RunTest(const FString& Parameters)
{
	// These warnings only fire on the graceful-skip paths; allow (but never require) them.
	AddExpectedMessagePlain(TEXT("LocalPlayer not found. Cannot set input mapping contexts."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);
	AddExpectedMessagePlain(TEXT("EnhancedInput subsystem not found."), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	if (!GameInstance)
	{
		AddInfo(TEXT("Could not create a game instance; skipping Enhanced Input active-path assertions."));
		return true;
	}
	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone();

	UWorld* World = GameInstance->GetWorld();

	// CreateLocalPlayer ensures when there is no game viewport client (headless), so build the local
	// player directly via AddLocalPlayer, which initializes the local-player subsystems without one.
	UClass* LocalPlayerClass = GEngine->LocalPlayerClass ? GEngine->LocalPlayerClass.Get() : ULocalPlayer::StaticClass();
	ULocalPlayer* LocalPlayer = NewObject<ULocalPlayer>(GEngine, LocalPlayerClass);
	if (World && LocalPlayer)
	{
		GameInstance->AddLocalPlayer(LocalPlayer, FPlatformMisc::GetPlatformUserForUserIndex(0));
	}

	if (!World || !LocalPlayer)
	{
		AddInfo(TEXT("Local player unavailable in the headless harness; skipping Enhanced Input active-path assertions."));
		GameInstance->Shutdown();
		GameInstance->RemoveFromRoot();
		return true;
	}

	APlayerController* PlayerController = World->SpawnActor<APlayerController>();
	if (!PlayerController)
	{
		AddInfo(TEXT("Could not spawn a player controller; skipping Enhanced Input active-path assertions."));
		GameInstance->Shutdown();
		GameInstance->RemoveFromRoot();
		return true;
	}
	PlayerController->SetPlayer(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = UDirectiveUtilInputFunctionLibrary::GetEnhancedInputSubsystem(PlayerController);
	if (!Subsystem)
	{
		AddInfo(TEXT("Enhanced Input subsystem unavailable for the synthetic local player; skipping active-path assertions."));
		GameInstance->Shutdown();
		GameInstance->RemoveFromRoot();
		return true;
	}

	// We have a live subsystem: exercise the active add/active/swap/remove/clear paths for real.
	TestNotNull("GetEnhancedInputSubsystem returns the subsystem for a live local player", Subsystem);

	UInputMappingContext* ContextA = NewObject<UInputMappingContext>(GetTransientPackage());
	UInputMappingContext* ContextB = NewObject<UInputMappingContext>(GetTransientPackage());
	ContextA->AddToRoot();
	ContextB->AddToRoot();
	const TSoftObjectPtr<UInputMappingContext> SoftA(ContextA);
	const TSoftObjectPtr<UInputMappingContext> SoftB(ContextB);

	// Add context A.
	FDirectiveUtilEnhancedInputContextData DataA;
	DataA.InputContext = SoftA;
	DataA.Priority = 0;
	TArray<FDirectiveUtilEnhancedInputContextData> ToAdd;
	ToAdd.Add(DataA);
	TestEqual("AddInputMappingContexts succeeds for a live controller",
		UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(PlayerController, ToAdd, false),
		EDirectiveUtilSuccessStatus::Success);
	TestTrue("Added context is reported active",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftA));

	// Swap A -> B.
	TestEqual("SwapInputMappingContexts succeeds",
		UDirectiveUtilInputFunctionLibrary::SwapInputMappingContexts(PlayerController, SoftA, SoftB, 0, false),
		EDirectiveUtilSuccessStatus::Success);
	TestFalse("Swapped-out context is inactive",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftA));
	TestTrue("Swapped-in context is active",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftB));

	// Remove B.
	TArray<TSoftObjectPtr<UInputMappingContext>> ToRemove;
	ToRemove.Add(SoftB);
	TestEqual("RemoveInputMappingContexts succeeds",
		UDirectiveUtilInputFunctionLibrary::RemoveInputMappingContexts(PlayerController, ToRemove),
		EDirectiveUtilSuccessStatus::Success);
	TestFalse("Removed context is inactive",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftB));

	// Clear all (after re-adding A).
	UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(PlayerController, ToAdd, false);
	TestEqual("ClearAllInputMappingContexts succeeds",
		UDirectiveUtilInputFunctionLibrary::ClearAllInputMappingContexts(PlayerController),
		EDirectiveUtilSuccessStatus::Success);
	TestFalse("Context is inactive after clear-all",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftA));

	// A clearing add whose every context fails to load must leave the existing mappings untouched.
	AddExpectedMessagePlain(TEXT("Input Mapping Contexts failed to load and were not added!"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, -1);
	UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(PlayerController, ToAdd, false);
	FDirectiveUtilEnhancedInputContextData UnresolvableData;
	UnresolvableData.Priority = 0;
	TArray<FDirectiveUtilEnhancedInputContextData> UnresolvableToAdd;
	UnresolvableToAdd.Add(UnresolvableData);
	TestEqual("AddInputMappingContexts fails when no context loads",
		UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(PlayerController, UnresolvableToAdd, true),
		EDirectiveUtilSuccessStatus::Failure);
	TestTrue("Previously active context survives a failed clearing add",
		UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(PlayerController, SoftA));

	ContextA->RemoveFromRoot();
	ContextB->RemoveFromRoot();
	GameInstance->Shutdown();
	GameInstance->RemoveFromRoot();

	return true;
}

#endif // WITH_EDITOR
