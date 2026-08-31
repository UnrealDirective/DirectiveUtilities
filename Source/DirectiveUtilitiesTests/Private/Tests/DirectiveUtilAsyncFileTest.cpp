// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tests/DirectiveUtilTestObject.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Tasks/DirectiveUtilTask_FileSystem.h"
#include "TimerManager.h"

namespace DirectiveUtilAsyncFileTest
{
	UDirectiveUtilDelegateListener* CreateListener()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
		if (!GameInstance)
		{
			return nullptr;
		}
		GameInstance->AddToRoot();
		GameInstance->InitializeStandalone();

		UDirectiveUtilDelegateListener* Listener = NewObject<UDirectiveUtilDelegateListener>();
		Listener->AddToRoot();
		Listener->ScenarioGameInstance = GameInstance;
		Listener->ScenarioWorld = GameInstance->GetWorld();
		return Listener;
	}

	void ResetResult(UDirectiveUtilDelegateListener& Listener)
	{
		Listener.bCompleted = false;
		Listener.bFailed = false;
		Listener.LastError.Reset();
	}

	void DestroyListener(UDirectiveUtilDelegateListener* Listener, const FString& Directory)
	{
		if (!Listener)
		{
			return;
		}
		Listener->Keepalive = nullptr;
		if (UGameInstance* GameInstance = Listener->ScenarioGameInstance.Get())
		{
			GameInstance->Shutdown();
			GameInstance->RemoveFromRoot();
		}
		Listener->ScenarioWorld = nullptr;
		Listener->ScenarioGameInstance = nullptr;
		Listener->RemoveFromRoot();
		IFileManager::Get().DeleteDirectory(*Directory, false, true);
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(
	FDirectiveUtilRunAsyncFileScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	FString, Directory,
	int32, Stage,
	int32, FramesRemaining);

bool FDirectiveUtilRunAsyncFileScenario::Update()
{
	if (!Listener || !Listener->ScenarioWorld)
	{
		Test->AddError(TEXT("The async file scenario has no world."));
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	}
	if (--FramesRemaining <= 0)
	{
		Test->AddError(FString::Printf(TEXT("Async file scenario timed out at stage %d."), Stage));
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	}

	UWorld* World = Listener->ScenarioWorld;
	World->GetTimerManager().Tick(0.1f);
	Listener->ScenarioGameInstance->GetTimerManager().Tick(0.1f);
	const FString TextPath = FPaths::Combine(Directory, TEXT("state.txt"));
	const FString BinaryPath = FPaths::Combine(Directory, TEXT("state.bin"));
	const FString WatchedPath = FPaths::Combine(Directory, TEXT("watched.ini"));

	switch (Stage)
	{
	case 0:
	{
		UDirectiveUtilTask_WriteTextFile* Task = UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
			World, TextPath, TEXT("realtime-text"));
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWriteCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWriteFailed);
		Task->Activate();
		Stage = 1;
		return false;
	}
	case 1:
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Async text write completes"), Listener->bCompleted);
		Test->TestTrue(TEXT("Async text write has no error"), Listener->LastError.IsEmpty());
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Stage = 2;
		return false;
	case 2:
	{
		UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(World, TextPath);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTextFileCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTextFileFailed);
		Task->Activate();
		Stage = 3;
		return false;
	}
	case 3:
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Async text read completes"), Listener->bCompleted);
		Test->TestEqual(TEXT("Async text read returns contents"), Listener->LastString, FString(TEXT("realtime-text")));
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Stage = 4;
		return false;
	case 4:
	{
		const TArray<uint8> ExpectedBytes = { 0, 1, 2, 127, 255 };
		UDirectiveUtilTask_WriteBinaryFile* Task = UDirectiveUtilTask_WriteBinaryFile::WriteBinaryFileAsync(
			World, BinaryPath, ExpectedBytes);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWriteCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWriteFailed);
		Task->Activate();
		Stage = 5;
		return false;
	}
	case 5:
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Async binary write completes"), Listener->bCompleted);
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Stage = 6;
		return false;
	case 6:
	{
		UDirectiveUtilTask_ReadBinaryFile* Task = UDirectiveUtilTask_ReadBinaryFile::ReadBinaryFileAsync(World, BinaryPath);
		Listener->Keepalive = Task;
		Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBinaryFileCompleted);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnBinaryFileFailed);
		Task->Activate();
		Stage = 7;
		return false;
	}
	case 7:
	{
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Async binary read completes"), Listener->bCompleted);
		const TArray<uint8> ExpectedBytes = { 0, 1, 2, 127, 255 };
		Test->TestEqual(TEXT("Async binary read returns all bytes"), Listener->LastBytes, ExpectedBytes);
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Stage = 8;
		return false;
	}
	case 8:
	{
		UDirectiveUtilTask_WatchFile* Task = UDirectiveUtilTask_WatchFile::WatchFile(World, WatchedPath, 0.01f);
		Listener->Keepalive = Task;
		Task->Changed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileChanged);
		Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWatchFailed);
		Task->Activate();
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(WatchedPath, TEXT("one"));
		Stage = 9;
		return false;
	}
	case 9:
		if (Listener->FileChangeTypes.Num() < 1)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports creation"), Listener->FileChangeTypes[0], EDirectiveUtilFileChangeType::Created);
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(WatchedPath, TEXT("second"));
		Stage = 10;
		return false;
	case 10:
	{
		if (Listener->FileChangeTypes.Num() < 2)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports modification"), Listener->FileChangeTypes[1], EDirectiveUtilFileChangeType::Modified);
		IFileManager& FileManager = IFileManager::Get();
		Test->TestTrue(TEXT("Watched file is deleted"), FileManager.Delete(*WatchedPath, true, true, false));
		Test->TestFalse(TEXT("Deleted file no longer exists"), FileManager.FileExists(*WatchedPath));
		Stage = 11;
		return false;
	}
	case 11:
		if (Listener->FileChangeTypes.Num() < 3)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports deletion"), Listener->FileChangeTypes[2], EDirectiveUtilFileChangeType::Deleted);
		Test->TestEqual(TEXT("Watcher returns the requested path"), Listener->FileChangePaths[0], WatchedPath);
		if (UDirectiveUtilTask_WatchFile* Watcher = Cast<UDirectiveUtilTask_WatchFile>(Listener->Keepalive))
		{
			Watcher->Cancel();
			Test->TestFalse(TEXT("Cancelled watcher is inactive"), Watcher->IsActive());
		}
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Listener->UpdatedCount = 0;
		if (UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(World, TextPath))
		{
			Listener->Keepalive = Task;
			Task->Completed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTextFileCompleted);
			Task->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnTextFileFailed);
			Task->Activate();
			Task->Cancel();
			Test->TestFalse(TEXT("Cancelled async read is inactive"), Task->IsActive());
		}
		Stage = 12;
		return false;
	case 12:
		if (++Listener->UpdatedCount < 20)
		{
			return false;
		}
		Test->TestFalse(TEXT("Cancelled async read does not complete"), Listener->bCompleted);
		Test->TestFalse(TEXT("Cancelled async read does not fail"), Listener->bFailed);
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	default:
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilAsyncFileTest,
	"DirectiveUtilities.FileSystem.AsyncFileTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilAsyncFileTest::RunTest(const FString& Parameters)
{
	UDirectiveUtilDelegateListener* Listener = DirectiveUtilAsyncFileTest::CreateListener();
	if (!Listener || !Listener->ScenarioWorld)
	{
		AddError(TEXT("Failed to create a transient world for async file tests."));
		return false;
	}

	const FString Directory = FPaths::Combine(
		FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()),
		TEXT("DirectiveUtilitiesTests"),
		FGuid::NewGuid().ToString(EGuidFormats::Digits));
	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilRunAsyncFileScenario(this, Listener, Directory, 0, 1200));
	return true;
}
