// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Tests/DirectiveUtilTestObject.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Tasks/DirectiveUtilTask_FileSystem.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

namespace DirectiveUtilAsyncFileTest
{
	constexpr double ScenarioTimeoutSeconds = 60.0;
	constexpr float NormalTimeDilation = 1.0f;

	void ResumeWorld(UWorld& World)
	{
		AWorldSettings* WorldSettings = World.GetWorldSettings(false, false);
		if (!WorldSettings)
		{
			return;
		}
		if (APlayerState* Pauser = WorldSettings->GetPauserPlayerState())
		{
			WorldSettings->SetPauserPlayerState(nullptr);
			Pauser->Destroy();
		}
		WorldSettings->SetTimeDilation(NormalTimeDilation);
	}

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
		if (Listener->ScenarioWorld)
		{
			ResumeWorld(*Listener->ScenarioWorld);
		}
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

	void ResetChanges(UDirectiveUtilDelegateListener& Listener)
	{
		Listener.FileChangeTypes.Reset();
		Listener.FileChangePaths.Reset();
	}

	int32 CountTemporaryFiles(const FString& Directory)
	{
		TArray<FString> TemporaryFiles;
		IFileManager::Get().FindFiles(TemporaryFiles, *(Directory / TEXT("*.tmp-*")), true, false);
		return TemporaryFiles.Num();
	}

	template <typename TaskType>
	void BindWrite(TaskType& Task, UDirectiveUtilDelegateListener& Listener)
	{
		Listener.Keepalive = &Task;
		Task.Completed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnFileWriteCompleted);
		Task.Failed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnFileWriteFailed);
	}

	void BindTextRead(UDirectiveUtilTask_ReadTextFile& Task, UDirectiveUtilDelegateListener& Listener)
	{
		Listener.Keepalive = &Task;
		Task.Completed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnTextFileCompleted);
		Task.Failed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnTextFileFailed);
	}

	void BindWatch(UDirectiveUtilTask_WatchFile& Task, UDirectiveUtilDelegateListener& Listener)
	{
		Listener.Keepalive = &Task;
		Task.Changed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnFileChanged);
		Task.Failed.AddDynamic(&Listener, &UDirectiveUtilDelegateListener::OnFileWatchFailed);
	}

	FString ReadText(const FString& Path)
	{
		FString Contents;
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(Path, Contents);
		return Contents;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(
	FDirectiveUtilRunAsyncFileScenario,
	FAutomationTestBase*, Test,
	UDirectiveUtilDelegateListener*, Listener,
	FString, Directory,
	int32, Stage,
	double, Deadline);

bool FDirectiveUtilRunAsyncFileScenario::Update()
{
	if (!Listener || !Listener->ScenarioWorld)
	{
		Test->AddError(TEXT("The async file scenario has no world."));
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	}
	if (FPlatformTime::Seconds() > Deadline)
	{
		Test->AddError(FString::Printf(TEXT("Async file scenario timed out at stage %d."), Stage));
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	}

	UWorld* World = Listener->ScenarioWorld;
	const FString TextPath = FPaths::Combine(Directory, TEXT("state.txt"));
	const FString BinaryPath = FPaths::Combine(Directory, TEXT("state.bin"));
	const FString WatchedPath = FPaths::Combine(Directory, TEXT("watched.ini"));
	const FString RelativeWatchedPath = FPaths::Combine(
		TEXT("DirectiveUtilitiesTests"), FPaths::GetCleanFilename(Directory), TEXT("relative.ini"));
	const FString FreshPath = FPaths::Combine(Directory, TEXT("fresh.txt"));
	const FString CancelledWritePath = FPaths::Combine(Directory, TEXT("cancelled.bin"));
	const FString PausedWatchedPath = FPaths::Combine(Directory, TEXT("paused.ini"));
	const TArray<uint8> ExpectedBytes = { 0, 1, 2, 127, 255 };

	switch (Stage)
	{
	case 0:
	{
		UDirectiveUtilTask_WriteTextFile* Task = UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
			World, TextPath, TEXT("realtime-text"));
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
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
		DirectiveUtilAsyncFileTest::BindTextRead(*Task, *Listener);
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
		UDirectiveUtilTask_WriteBinaryFile* Task = UDirectiveUtilTask_WriteBinaryFile::WriteBinaryFileAsync(
			World, BinaryPath, ExpectedBytes);
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
		Task->Activate();
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
		{
			TArray<uint8> WrittenBytes;
			UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, WrittenBytes);
			Test->TestEqual(TEXT("A second Activate does not write the file again"), WrittenBytes, ExpectedBytes);
		}
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
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Async binary read completes"), Listener->bCompleted);
		Test->TestEqual(TEXT("Async binary read returns all bytes"), Listener->LastBytes, ExpectedBytes);
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Stage = 8;
		return false;
	case 8:
	{
		for (const float InvalidInterval : { 0.0f, -1.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity() })
		{
			UDirectiveUtilTask_WatchFile* InvalidWatcher = UDirectiveUtilTask_WatchFile::WatchFile(World, WatchedPath, InvalidInterval);
			DirectiveUtilAsyncFileTest::BindWatch(*InvalidWatcher, *Listener);
			InvalidWatcher->Activate();
			Test->TestTrue(*FString::Printf(TEXT("A poll interval of %f fires Failed"), InvalidInterval), Listener->bFailed);
			Test->TestFalse(*FString::Printf(TEXT("A poll interval of %f leaves the watcher inactive"), InvalidInterval), InvalidWatcher->IsActive());
			DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		}
		UDirectiveUtilTask_WatchFile* Task = UDirectiveUtilTask_WatchFile::WatchFile(World, WatchedPath, 0.01f);
		DirectiveUtilAsyncFileTest::BindWatch(*Task, *Listener);
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
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(WatchedPath, TEXT("two"));
		Stage = 10;
		return false;
	case 10:
		if (Listener->FileChangeTypes.Num() < 2)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports a same-size rewrite"), Listener->FileChangeTypes[1], EDirectiveUtilFileChangeType::Modified);
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(WatchedPath, TEXT("second"));
		Stage = 11;
		return false;
	case 11:
	{
		if (Listener->FileChangeTypes.Num() < 3)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports a size change"), Listener->FileChangeTypes[2], EDirectiveUtilFileChangeType::Modified);
		IFileManager& FileManager = IFileManager::Get();
		Test->TestTrue(TEXT("Watched file is deleted"), FileManager.Delete(*WatchedPath, true, true, false));
		Test->TestFalse(TEXT("Deleted file no longer exists"), FileManager.FileExists(*WatchedPath));
		Stage = 12;
		return false;
	}
	case 12:
	{
		if (Listener->FileChangeTypes.Num() < 4)
		{
			return false;
		}
		Test->TestEqual(TEXT("Watcher reports deletion"), Listener->FileChangeTypes[3], EDirectiveUtilFileChangeType::Deleted);
		Test->TestEqual(TEXT("Watcher returns the requested path"), Listener->FileChangePaths[0], WatchedPath);
		if (UDirectiveUtilTask_WatchFile* Watcher = Cast<UDirectiveUtilTask_WatchFile>(Listener->Keepalive))
		{
			Watcher->Cancel();
			Test->TestFalse(TEXT("Cancelled watcher is inactive"), Watcher->IsActive());
		}
		DirectiveUtilAsyncFileTest::ResetChanges(*Listener);
		UDirectiveUtilTask_WatchFile* RelativeWatcher = UDirectiveUtilTask_WatchFile::WatchFile(World, RelativeWatchedPath, 0.01f);
		DirectiveUtilAsyncFileTest::BindWatch(*RelativeWatcher, *Listener);
		RelativeWatcher->Activate();
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(FPaths::Combine(Directory, TEXT("relative.ini")), TEXT("relative"));
		Stage = 13;
		return false;
	}
	case 13:
	{
		if (Listener->FileChangeTypes.Num() < 1)
		{
			return false;
		}
		Test->TestEqual(TEXT("A Saved-relative watcher reports creation"), Listener->FileChangeTypes[0], EDirectiveUtilFileChangeType::Created);
		Test->TestEqual(TEXT("A Saved-relative watcher returns the relative path"), Listener->FileChangePaths[0], RelativeWatchedPath);
		if (UDirectiveUtilTask_WatchFile* Watcher = Cast<UDirectiveUtilTask_WatchFile>(Listener->Keepalive))
		{
			Watcher->Cancel();
		}
		DirectiveUtilAsyncFileTest::ResetChanges(*Listener);
		UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(
			World, FPaths::Combine(Directory, TEXT("missing.txt")));
		DirectiveUtilAsyncFileTest::BindTextRead(*Task, *Listener);
		Listener->LastString = TEXT("stale");
		Task->Activate();
		Stage = 14;
		return false;
	}
	case 14:
	{
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("Reading a missing file fires Failed"), Listener->bFailed);
		Test->TestFalse(TEXT("Reading a missing file does not complete"), Listener->bCompleted);
		Test->TestFalse(TEXT("A failed read reports an error"), Listener->LastError.IsEmpty());
		Test->TestTrue(TEXT("A failed read returns empty contents"), Listener->LastString.IsEmpty());
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		UDirectiveUtilTask_WriteTextFile* Task = UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
			World, TextPath, TEXT("replaced"), true, false);
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
		Task->Activate();
		Stage = 15;
		return false;
	}
	case 15:
	{
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("An async no-overwrite write to an existing file fails"), Listener->bFailed);
		Test->TestEqual(TEXT("A refused async write keeps the file"),
			DirectiveUtilAsyncFileTest::ReadText(TextPath), FString(TEXT("realtime-text")));
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		UDirectiveUtilTask_WriteTextFile* Task = UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
			World, TextPath, TEXT("non-atomic"), true, true, false);
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
		Task->Activate();
		Stage = 16;
		return false;
	}
	case 16:
	{
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("An async non-atomic write completes"), Listener->bCompleted);
		Test->TestEqual(TEXT("An async non-atomic write replaces the file"),
			DirectiveUtilAsyncFileTest::ReadText(TextPath), FString(TEXT("non-atomic")));
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		UDirectiveUtilTask_WriteTextFile* Task = UDirectiveUtilTask_WriteTextFile::WriteTextFileAsync(
			World, FreshPath, TEXT("fresh"), true, false);
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
		Task->Activate();
		Stage = 17;
		return false;
	}
	case 17:
	{
		if (!Listener->bCompleted && !Listener->bFailed)
		{
			return false;
		}
		Test->TestTrue(TEXT("An async no-overwrite write creates a missing file"), Listener->bCompleted);
		Test->TestEqual(TEXT("An async no-overwrite write stores the text"),
			DirectiveUtilAsyncFileTest::ReadText(FreshPath), FString(TEXT("fresh")));
		Test->TestEqual(TEXT("Async writes leave no temporary files"),
			DirectiveUtilAsyncFileTest::CountTemporaryFiles(Directory), 0);
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		Listener->UpdatedCount = 0;
		UDirectiveUtilTask_WriteBinaryFile* Task = UDirectiveUtilTask_WriteBinaryFile::WriteBinaryFileAsync(
			World, CancelledWritePath, ExpectedBytes);
		DirectiveUtilAsyncFileTest::BindWrite(*Task, *Listener);
		Task->Activate();
		Task->Cancel();
		Test->TestFalse(TEXT("A cancelled async write is inactive"), Task->IsActive());
		Stage = 18;
		return false;
	}
	case 18:
	{
		if (++Listener->UpdatedCount < 20 || !IFileManager::Get().FileExists(*CancelledWritePath))
		{
			return false;
		}
		Test->TestFalse(TEXT("A cancelled async write does not complete"), Listener->bCompleted);
		Test->TestFalse(TEXT("A cancelled async write does not fail"), Listener->bFailed);
		TArray<uint8> CancelledBytes;
		UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(CancelledWritePath, CancelledBytes);
		Test->TestEqual(TEXT("A write cancelled in flight still finishes on disk"), CancelledBytes, ExpectedBytes);
		Listener->UpdatedCount = 0;
		UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(World, TextPath);
		DirectiveUtilAsyncFileTest::BindTextRead(*Task, *Listener);
		Task->Activate();
		Task->Cancel();
		Test->TestFalse(TEXT("Cancelled async read is inactive"), Task->IsActive());
		Stage = 19;
		return false;
	}
	case 19:
		if (++Listener->UpdatedCount < 20)
		{
			return false;
		}
		Test->TestFalse(TEXT("Cancelled async read does not complete"), Listener->bCompleted);
		Test->TestFalse(TEXT("Cancelled async read does not fail"), Listener->bFailed);
		DirectiveUtilAsyncFileTest::ResetResult(*Listener);
		DirectiveUtilAsyncFileTest::ResetChanges(*Listener);
		Stage = 20;
		return false;
	case 20:
	{
		AWorldSettings* WorldSettings = World->GetWorldSettings(false, false);
		if (!WorldSettings)
		{
			Test->AddError(TEXT("The scenario world has no world settings."));
			DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
			return true;
		}
		Test->TestFalse(TEXT("The scenario world starts unpaused"), World->IsPaused());
		Test->TestEqual(TEXT("The scenario world starts at normal speed"),
			WorldSettings->GetEffectiveTimeDilation(), DirectiveUtilAsyncFileTest::NormalTimeDilation, 0.0f);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APlayerState* Pauser = World->SpawnActor<APlayerState>(SpawnParameters);
		if (!Pauser)
		{
			Test->AddError(TEXT("The scenario world could not spawn a pauser player state."));
			DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
			return true;
		}
		WorldSettings->SetPauserPlayerState(Pauser);
		WorldSettings->SetTimeDilation(WorldSettings->MinGlobalTimeDilation);
		Test->TestTrue(TEXT("The scenario world is paused"), World->IsPaused());
		Test->TestEqual(TEXT("The scenario world runs at the minimum time dilation"),
			WorldSettings->GetEffectiveTimeDilation(), WorldSettings->MinGlobalTimeDilation, 0.0f);

		UDirectiveUtilTask_WatchFile* Task = UDirectiveUtilTask_WatchFile::WatchFile(World, PausedWatchedPath, 0.01f);
		DirectiveUtilAsyncFileTest::BindWatch(*Task, *Listener);
		Task->Activate();
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(PausedWatchedPath, TEXT("paused"));
		Stage = 21;
		return false;
	}
	case 21:
		if (Listener->FileChangeTypes.Num() < 1)
		{
			return false;
		}
		Test->TestTrue(TEXT("The world is still paused when the watcher reports creation"), World->IsPaused());
		Test->TestEqual(TEXT("A watcher reports creation while the world is paused"),
			Listener->FileChangeTypes[0], EDirectiveUtilFileChangeType::Created);
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(PausedWatchedPath, TEXT("changed while paused"));
		Stage = 22;
		return false;
	case 22:
	{
		if (Listener->FileChangeTypes.Num() < 2)
		{
			return false;
		}
		Test->TestTrue(TEXT("The world is still paused when the watcher reports a change"), World->IsPaused());
		Test->TestEqual(TEXT("A watcher reports a modification while the world is paused"),
			Listener->FileChangeTypes[1], EDirectiveUtilFileChangeType::Modified);
		if (UDirectiveUtilTask_WatchFile* Watcher = Cast<UDirectiveUtilTask_WatchFile>(Listener->Keepalive))
		{
			Watcher->Cancel();
		}
		DirectiveUtilAsyncFileTest::ResumeWorld(*World);
		Test->TestFalse(TEXT("The scenario world resumes after the pause test"), World->IsPaused());
		const AWorldSettings* WorldSettings = World->GetWorldSettings(false, false);
		Test->TestTrue(TEXT("The scenario world returns to normal speed"),
			WorldSettings && WorldSettings->GetEffectiveTimeDilation() == DirectiveUtilAsyncFileTest::NormalTimeDilation);
		DirectiveUtilAsyncFileTest::DestroyListener(Listener, Directory);
		return true;
	}
	default:
		return true;
	}
}

class FDirectiveUtilRunAsyncFileWithoutGameInstance : public IAutomationLatentCommand
{
public:
	FDirectiveUtilRunAsyncFileWithoutGameInstance(FAutomationTestBase* InTest, const FString& InDirectory)
		: Test(InTest)
		, Directory(InDirectory)
	{
	}

	virtual bool Update() override
	{
		const FString TextPath = FPaths::Combine(Directory, TEXT("detached.txt"));
		const FString WatchedPath = FPaths::Combine(Directory, TEXT("detached.ini"));
		if (Deadline == 0.0)
		{
			Deadline = FPlatformTime::Seconds() + DirectiveUtilAsyncFileTest::ScenarioTimeoutSeconds;
		}
		if (FPlatformTime::Seconds() > Deadline)
		{
			Test->AddError(FString::Printf(TEXT("Async file scenario without a game instance timed out at stage %d."), Stage));
			return TearDown();
		}

		switch (Stage)
		{
		case 0:
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			Listener = NewObject<UDirectiveUtilDelegateListener>();
			Listener->AddToRoot();
			if (!World)
			{
				Test->AddError(TEXT("Failed to create a world without a game instance."));
				return TearDown();
			}
			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			WorldContext.SetCurrentWorld(World);
			Test->TestNull(TEXT("The scenario world has no game instance"), World->GetGameInstance());

			UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(TextPath, TEXT("detached"));
			UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(World, TextPath);
			DirectiveUtilAsyncFileTest::BindTextRead(*Task, *Listener);
			Task->Activate();
			Test->TestTrue(TEXT("An async read without a game instance is active"), Task->IsActive());
			Test->TestTrue(TEXT("A pending async read without a game instance is rooted"), Task->IsRooted());
			Stage = 1;
			return false;
		}
		case 1:
		{
			if (!Listener->bCompleted && !Listener->bFailed)
			{
				return false;
			}
			Test->TestTrue(TEXT("An async read without a game instance completes"), Listener->bCompleted);
			Test->TestEqual(TEXT("An async read without a game instance returns contents"),
				Listener->LastString, FString(TEXT("detached")));
			Test->TestFalse(TEXT("A finished async read releases its root"), Listener->Keepalive->IsRooted());
			DirectiveUtilAsyncFileTest::ResetResult(*Listener);
			UDirectiveUtilTask_WatchFile* Watcher = UDirectiveUtilTask_WatchFile::WatchFile(World, WatchedPath, 0.01f);
			Watcher->Changed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileChanged);
			Watcher->Failed.AddDynamic(Listener, &UDirectiveUtilDelegateListener::OnFileWatchFailed);
			Listener->Keepalive = nullptr;
			UnreferencedWatcher = Watcher;
			Watcher->Activate();
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
			Test->TestTrue(TEXT("A watcher without a game instance survives garbage collection"), UnreferencedWatcher.IsValid());
			UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(WatchedPath, TEXT("watched"));
			Stage = 2;
			return false;
		}
		case 2:
		{
			if (Listener->FileChangeTypes.Num() < 1 && !Listener->bFailed)
			{
				return false;
			}
			Test->TestFalse(TEXT("A watcher without a game instance does not fail"), Listener->bFailed);
			if (Listener->FileChangeTypes.Num() > 0)
			{
				Test->TestEqual(TEXT("A watcher without a game instance reports creation"),
					Listener->FileChangeTypes[0], EDirectiveUtilFileChangeType::Created);
			}
			UDirectiveUtilTask_WatchFile* Watcher = UnreferencedWatcher.Get();
			Test->TestTrue(TEXT("The watcher is still alive while it watches"), Watcher != nullptr);
			DestroyWorld();
			if (Watcher)
			{
				Test->TestFalse(TEXT("World cleanup stops a watcher without a game instance"), Watcher->IsActive());
				Test->TestFalse(TEXT("World cleanup releases the watcher root"), Watcher->IsRooted());
			}

			DirectiveUtilAsyncFileTest::ResetResult(*Listener);
			UDirectiveUtilTask_ReadTextFile* Task = UDirectiveUtilTask_ReadTextFile::ReadTextFileAsync(nullptr, TextPath);
			DirectiveUtilAsyncFileTest::BindTextRead(*Task, *Listener);
			Task->Activate();
			Test->TestTrue(TEXT("An async read without a world fires Failed"), Listener->bFailed);
			Test->TestFalse(TEXT("An async read without a world reports an error"), Listener->LastError.IsEmpty());
			Test->TestFalse(TEXT("A rejected async read releases its root"), Task->IsRooted());
			return TearDown();
		}
		default:
			return TearDown();
		}
	}

private:
	void DestroyWorld()
	{
		if (World)
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			World = nullptr;
		}
	}

	bool TearDown()
	{
		DestroyWorld();
		if (Listener)
		{
			Listener->Keepalive = nullptr;
			Listener->RemoveFromRoot();
			Listener = nullptr;
		}
		IFileManager::Get().DeleteDirectory(*Directory, false, true);
		return true;
	}

	FAutomationTestBase* Test;
	FString Directory;
	UWorld* World = nullptr;
	UDirectiveUtilDelegateListener* Listener = nullptr;
	TWeakObjectPtr<UDirectiveUtilTask_WatchFile> UnreferencedWatcher;
	int32 Stage = 0;
	double Deadline = 0.0;
};

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
	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilRunAsyncFileScenario(
		this, Listener, Directory, 0, FPlatformTime::Seconds() + DirectiveUtilAsyncFileTest::ScenarioTimeoutSeconds));
	ADD_LATENT_AUTOMATION_COMMAND(FDirectiveUtilRunAsyncFileWithoutGameInstance(this, Directory));
	return true;
}
