// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilSaveGameFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilSaveGameFunctionLibraryTest, "DirectiveUtilities.SaveGameFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilSaveGameFunctionLibraryTest::RunTest(const FString& Parameters)
{
	{
		USaveGame* Save = NewObject<UDirectiveUtilTestSaveGame>();
		TArray<uint8> Bytes;
		const bool bSaved = UDirectiveUtilSaveGameFunctionLibrary::SaveGameToBytes(Save, Bytes);
		TestTrue("SaveGameToBytes should succeed for a valid object", bSaved);
		TestTrue("SaveGameToBytes should produce non-empty data", Bytes.Num() > 0);

		USaveGame* Loaded = UDirectiveUtilSaveGameFunctionLibrary::LoadGameFromBytes(Bytes);
		TestNotNull("LoadGameFromBytes should return a valid object", Loaded);
	}

	{
		TArray<uint8> Bytes;
		Bytes.Add(1);
		TestFalse("SaveGameToBytes should fail for a null object", UDirectiveUtilSaveGameFunctionLibrary::SaveGameToBytes(nullptr, Bytes));
		TestEqual("SaveGameToBytes should clear the output for a null object", Bytes.Num(), 0);
		TestNull("LoadGameFromBytes should return null for empty data", UDirectiveUtilSaveGameFunctionLibrary::LoadGameFromBytes(TArray<uint8>()));
	}

	{
		const FString TestSlot = TEXT("DirectiveUtilitiesAutomationTestSlot");
		UGameplayStatics::DeleteGameInSlot(TestSlot, 0);

		USaveGame* Save = NewObject<UDirectiveUtilTestSaveGame>();
		const bool bWritten = UGameplayStatics::SaveGameToSlot(Save, TestSlot, 0);
		TestTrue("SaveGameToSlot should write the temporary test slot", bWritten);

		if (bWritten)
		{
			TestTrue("GetAllSaveSlotNames should include the written slot",
				UDirectiveUtilSaveGameFunctionLibrary::GetAllSaveSlotNames().Contains(TestSlot));

			FDateTime Timestamp;
			TestTrue("GetSaveSlotTimestamp should succeed for an existing slot",
				UDirectiveUtilSaveGameFunctionLibrary::GetSaveSlotTimestamp(TestSlot, Timestamp));
			TestTrue("GetSaveSlotTimestamp should return local time for a fresh save",
				FMath::Abs((FDateTime::Now() - Timestamp).GetTotalMinutes()) < 5.0);
		}

		// Clean up
		UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
		TestFalse("A deleted slot should no longer be listed",
			UDirectiveUtilSaveGameFunctionLibrary::GetAllSaveSlotNames().Contains(TestSlot));

		FDateTime MissingTimestamp;
		TestFalse("GetSaveSlotTimestamp should fail for a missing slot",
			UDirectiveUtilSaveGameFunctionLibrary::GetSaveSlotTimestamp(TestSlot, MissingTimestamp));
	}

	{
		const FString SlotA = TEXT("DirectiveUtilitiesAutomationTestSlotA");
		const FString SlotB = TEXT("DirectiveUtilitiesAutomationTestSlotB");
		const FString SlotC = TEXT("DirectiveUtilitiesAutomationTestSlotC");
		UGameplayStatics::DeleteGameInSlot(SlotA, 0);
		UGameplayStatics::DeleteGameInSlot(SlotB, 0);
		UGameplayStatics::DeleteGameInSlot(SlotC, 0);

		USaveGame* Save = NewObject<UDirectiveUtilTestSaveGame>();
		const bool bWritten = UGameplayStatics::SaveGameToSlot(Save, SlotA, 0);
		TestTrue("SaveGameToSlot should write the slot-management test slot", bWritten);

		if (bWritten)
		{
			TestTrue("DoesSaveSlotExist should find the written slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotA));
			TestFalse("DoesSaveSlotExist should not find a missing slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotB));

			TestTrue("RenameSaveSlot should rename onto a free slot name",
				UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(SlotA, SlotB));
			TestFalse("RenameSaveSlot should remove the old slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotA));
			TestTrue("RenameSaveSlot should create the new slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotB));
			TestNotNull("A renamed slot should still deserialize",
				UGameplayStatics::LoadGameFromSlot(SlotB, 0));

			USaveGame* OtherSave = NewObject<UDirectiveUtilTestSaveGame>();
			TestTrue("SaveGameToSlot should write the collision test slot",
				UGameplayStatics::SaveGameToSlot(OtherSave, SlotC, 0));
			TestFalse("RenameSaveSlot should refuse to rename onto an existing slot",
				UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(SlotB, SlotC));
			TestTrue("A refused rename should keep the source slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotB));
			TestTrue("A refused rename should keep the target slot",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotC));

			TestFalse("DoesSaveSlotExist should reject an invalid slot name",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(TEXT("../escape")));
			TestFalse("DeleteSaveSlot should reject an invalid slot name",
				UDirectiveUtilSaveGameFunctionLibrary::DeleteSaveSlot(TEXT("../escape")));
			TestFalse("DoesSaveSlotExist should reject a nested slot path",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(TEXT("Profiles/Slot1")));
			TestFalse("RenameSaveSlot should reject an invalid source slot name",
				UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(TEXT("../escape"), SlotC));
			TestFalse("RenameSaveSlot should reject an invalid destination slot name",
				UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(SlotC, TEXT("../escape")));
			TestTrue("A rejected rename should leave the source slot intact",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotC));

			TestTrue("DeleteSaveSlot should delete an existing slot",
				UDirectiveUtilSaveGameFunctionLibrary::DeleteSaveSlot(SlotB));
			TestFalse("A deleted slot should no longer exist",
				UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(SlotB));
		}

		// Clean up
		UGameplayStatics::DeleteGameInSlot(SlotA, 0);
		UGameplayStatics::DeleteGameInSlot(SlotB, 0);
		UGameplayStatics::DeleteGameInSlot(SlotC, 0);
	}

	return true;
}
