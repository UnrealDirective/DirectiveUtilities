// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilSaveGameFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

namespace
{
	FString GetSaveGamesDirectory()
	{
		return FPaths::ProjectSavedDir() / TEXT("SaveGames");
	}

	FString GetSaveSlotFilePath(const FString& SlotName)
	{
		return GetSaveGamesDirectory() / (SlotName + TEXT(".sav"));
	}

	bool IsValidSaveSlotName(const FString& SlotName)
	{
		return UDirectiveUtilStringFunctionLibrary::IsValidFileName(SlotName);
	}
}

TArray<FString> UDirectiveUtilSaveGameFunctionLibrary::GetAllSaveSlotNames()
{
	TArray<FString> SlotNames;

	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(GetSaveGamesDirectory() / TEXT("*.sav")), true, false);

	SlotNames.Reserve(Files.Num());
	for (const FString& File : Files)
	{
		SlotNames.Add(FPaths::GetBaseFilename(File));
	}
	return SlotNames;
}

bool UDirectiveUtilSaveGameFunctionLibrary::GetSaveSlotTimestamp(const FString& SlotName, FDateTime& OutTimestamp)
{
	OutTimestamp = FDateTime();
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}

	const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*GetSaveSlotFilePath(SlotName));
	if (Timestamp == FDateTime::MinValue())
	{
		return false;
	}

	OutTimestamp = Timestamp + (FDateTime::Now() - FDateTime::UtcNow());
	return true;
}

bool UDirectiveUtilSaveGameFunctionLibrary::SaveGameToBytes(USaveGame* SaveGameObject, TArray<uint8>& OutBytes)
{
	OutBytes.Reset();
	if (!SaveGameObject)
	{
		return false;
	}
	return UGameplayStatics::SaveGameToMemory(SaveGameObject, OutBytes);
}

USaveGame* UDirectiveUtilSaveGameFunctionLibrary::LoadGameFromBytes(const TArray<uint8>& SaveData)
{
	if (SaveData.Num() == 0)
	{
		return nullptr;
	}
	return UGameplayStatics::LoadGameFromMemory(SaveData);
}

bool UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(const FString& SlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool UDirectiveUtilSaveGameFunctionLibrary::DeleteSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}
	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

bool UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(const FString& OldSlotName, const FString& NewSlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(OldSlotName) || !IsValidSaveSlotName(NewSlotName) || OldSlotName == NewSlotName)
	{
		return false;
	}
	if (!UGameplayStatics::DoesSaveGameExist(OldSlotName, UserIndex) || UGameplayStatics::DoesSaveGameExist(NewSlotName, UserIndex))
	{
		return false;
	}

	TArray<uint8> SaveData;
	if (!UGameplayStatics::LoadDataFromSlot(SaveData, OldSlotName, UserIndex))
	{
		return false;
	}
	if (!UGameplayStatics::SaveDataToSlot(SaveData, NewSlotName, UserIndex))
	{
		return false;
	}
	// If this delete fails the new copy is kept alongside the original, so the save is never lost.
	return UGameplayStatics::DeleteGameInSlot(OldSlotName, UserIndex);
}
