// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilInputFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "DirectiveUtilLogChannels.h"
#include "Logging/StructuredLog.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

bool UDirectiveUtilInputFunctionLibrary::TryGetEnhancedInputSubsystemFromController(
	AController* PlayerController,
	UEnhancedInputLocalPlayerSubsystem*& EnhancedInput)
{
	if (!PlayerController)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("PlayerController is null. Cannot set input mapping contexts."));
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(PlayerController);
	if (!PC)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("Controller is not a PlayerController. Cannot get enhanced input subsystem."));
		return false;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("LocalPlayer not found. Cannot set input mapping contexts."));
		return false;
	}

	EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if(!EnhancedInput)
	{
		UE_LOG(LogDirectiveUtil, Warning, TEXT("EnhancedInput subsystem not found. Cannot remove input mapping contexts."));
		return false;
	}

	return true;
}

EDirectiveUtilSuccessStatus UDirectiveUtilInputFunctionLibrary::AddInputMappingContexts(
	AController* PlayerController,
	const TArray<FDirectiveUtilEnhancedInputContextData>& Contexts,
	const bool bClearPrevious)
{
	if (Contexts.IsEmpty()) { return EDirectiveUtilSuccessStatus::Failure; }

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	const bool bEnhancedInputRetrievedFromController = TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput);

	if(!bEnhancedInputRetrievedFromController)
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	TArray<TPair<const UInputMappingContext*, int32>> LoadedContexts;
	TArray<int32> FailedIndices;
	for (int32 Index = 0; Index < Contexts.Num(); ++Index)
	{
		const auto& [InputContext, Priority] = Contexts[Index];
		if (const UInputMappingContext* MappingContext = InputContext.LoadSynchronous())
		{
			LoadedContexts.Emplace(MappingContext, Priority);
		}
		else
		{
			FailedIndices.Add(Index);
		}
	}

	if (FailedIndices.Num() > 0)
	{
		FString FailedIndicesStr = FString::JoinBy(FailedIndices, TEXT(", "), [](const int32 Index) { return FString::Printf(TEXT("%d"), Index); });
		UE_LOGFMT(LogDirectiveUtil, Warning, "{FailedIndicies} Input Mapping Contexts failed to load and were not added! The failed indexes are [{FailedIndicieIndexes}]", FailedIndices.Num(), FailedIndicesStr);
	}

	if (LoadedContexts.IsEmpty())
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	if (bClearPrevious)
	{
		EnhancedInput->ClearAllMappings();
	}

	for (const auto& [MappingContext, Priority] : LoadedContexts)
	{
		EnhancedInput->AddMappingContext(MappingContext, Priority);
	}

	UE_LOGFMT(LogDirectiveUtil, Verbose, "{MappingCount} Input mapping contexts set successfully.", Contexts.Num() - FailedIndices.Num());
	return EDirectiveUtilSuccessStatus::Success;
}

EDirectiveUtilSuccessStatus UDirectiveUtilInputFunctionLibrary::RemoveInputMappingContexts(
	AController* PlayerController,
	const TArray<TSoftObjectPtr<UInputMappingContext>>& Contexts)
{
	if (Contexts.IsEmpty()) { return EDirectiveUtilSuccessStatus::Failure;; }

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	const bool bEnhancedInputRetrievedFromController = TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput);

	if(!bEnhancedInputRetrievedFromController)
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	TArray<int32> FailedIndices;
	for (int32 Index = 0; Index < Contexts.Num(); ++Index)
	{
		const TSoftObjectPtr<UInputMappingContext>& Context = Contexts[Index];
		if (const UInputMappingContext* MappingContext = Context.LoadSynchronous())
		{
			EnhancedInput->RemoveMappingContext(MappingContext);
		} else
		{
			FailedIndices.Add(Index);
		}
	}

	if (FailedIndices.Num() > 0)
	{
		FString FailedIndicesStr = FString::JoinBy(FailedIndices, TEXT(", "), [](const int32 Index) { return FString::Printf(TEXT("%d"), Index); });
		UE_LOGFMT(LogDirectiveUtil, Warning, "{FailedIndicies} Input Mapping Contexts failed to load and were not removed! The failed indexes are [{FailedIndicieIndexes}]", FailedIndices.Num(), FailedIndicesStr);
	}

	if (FailedIndices.Num() == Contexts.Num())
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	UE_LOGFMT(LogDirectiveUtil, Verbose, "{MappingCount} Input mapping contexts removed successfully.", Contexts.Num() - FailedIndices.Num());
	return EDirectiveUtilSuccessStatus::Success;
}

EDirectiveUtilSuccessStatus UDirectiveUtilInputFunctionLibrary::SwapInputMappingContexts(
 AController* PlayerController,
 const TSoftObjectPtr<UInputMappingContext> PreviousContext,
 const TSoftObjectPtr<UInputMappingContext> NewContext,
 const int32 Priority,
 const bool bUsePreviousPriority)
{
	const UInputMappingContext* LoadedPreviousMappingContext = PreviousContext.LoadSynchronous();
	const UInputMappingContext* LoadedNewMappingContext = NewContext.LoadSynchronous();

	if (!LoadedPreviousMappingContext || !LoadedNewMappingContext)
	{
		UE_LOGFMT(LogDirectiveUtil, Warning, "Both the previous and new input mapping contexts must be valid.");
		return EDirectiveUtilSuccessStatus::Failure;
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	if (!TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput))
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	if (int32 PreviousPriority; EnhancedInput->HasMappingContext(LoadedPreviousMappingContext, PreviousPriority))
	{
		const int32 TargetPriority = bUsePreviousPriority ? PreviousPriority : Priority;
		EnhancedInput->RemoveMappingContext(LoadedPreviousMappingContext);
		EnhancedInput->AddMappingContext(LoadedNewMappingContext, TargetPriority);
		UE_LOGFMT(LogDirectiveUtil, Verbose, "Input mapping contexts {PreviousContext} and {NewContext} swapped successfully at priority {Priority}.", LoadedPreviousMappingContext->GetName(), LoadedNewMappingContext->GetName(), TargetPriority);
	}
	else
	{
		EnhancedInput->AddMappingContext(LoadedNewMappingContext, Priority);
		UE_LOGFMT(LogDirectiveUtil, Warning, "Previous input mapping context {PreviousContext} not found. New context {NewContext} added at priority {BackupPriority}.", LoadedPreviousMappingContext->GetName(), LoadedNewMappingContext->GetName(), Priority);
	}

	return EDirectiveUtilSuccessStatus::Success;
}

UEnhancedInputLocalPlayerSubsystem* UDirectiveUtilInputFunctionLibrary::GetEnhancedInputSubsystem(AController* PlayerController)
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInput = nullptr;
	TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput);
	return EnhancedInput;
}

bool UDirectiveUtilInputFunctionLibrary::IsInputMappingContextActive(AController* PlayerController, TSoftObjectPtr<UInputMappingContext> Context)
{
	const UInputMappingContext* MappingContext = Context.LoadSynchronous();
	if (!MappingContext)
	{
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	if (!TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput))
	{
		return false;
	}

	int32 OutPriority;
	return EnhancedInput->HasMappingContext(MappingContext, OutPriority);
}

EDirectiveUtilSuccessStatus UDirectiveUtilInputFunctionLibrary::ClearAllInputMappingContexts(AController* PlayerController)
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	if (!TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput))
	{
		return EDirectiveUtilSuccessStatus::Failure;
	}

	EnhancedInput->ClearAllMappings();
	return EDirectiveUtilSuccessStatus::Success;
}
