// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilDataTableFunctionLibrary.h"
#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"
#include "Tests/DirectiveUtilTestDataTableRows.h"
#include "Engine/CompositeDataTable.h"
#include "Internationalization/Text.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/EnumProperty.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

#if WITH_EDITOR
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/UserDefinedEnum.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Kismet2/StructureEditorUtils.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#endif

#include <limits>

namespace
{
	template <typename RowType>
	RowType* FindRowByName(const UDataTable* Table, const TCHAR* RowName)
	{
		return Table != nullptr ? reinterpret_cast<RowType*>(Table->GetRowMap().FindRef(FName(RowName))) : nullptr;
	}

	UDataTable* ImportWithHeader(UScriptStruct* RowStruct, const FString& CsvText, FString& OutErrorMessage)
	{
		return UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			RowStruct, CsvText, EDirectiveUtilCsvDelimiter::Comma, true, OutErrorMessage);
	}

	bool FindExportedCell(const FString& CsvText, const FString& RowName, const FString& ColumnName, FString& OutCell)
	{
		FDirectiveUtilCsvDocument Document;
		FString ParseError;
		if (!UDirectiveUtilCsvFunctionLibrary::ParseCsv(CsvText, EDirectiveUtilCsvDelimiter::Comma, Document, ParseError)
			|| Document.Rows.IsEmpty())
		{
			return false;
		}

		const int32 ColumnIndex = Document.Rows[0].Cells.IndexOfByKey(ColumnName);
		if (ColumnIndex == INDEX_NONE)
		{
			return false;
		}
		for (int32 RowIndex = 1; RowIndex < Document.Rows.Num(); ++RowIndex)
		{
			const TArray<FString>& Cells = Document.Rows[RowIndex].Cells;
			if (Cells.IsValidIndex(ColumnIndex) && Cells[0] == RowName)
			{
				OutCell = Cells[ColumnIndex];
				return true;
			}
		}
		return false;
	}

	void SetEnumPropertyValue(const FEnumProperty* Property, void* RowMemory, const int64 Value)
	{
		Property->GetUnderlyingProperty()->SetIntPropertyValue(Property->ContainerPtrToValuePtr<void>(RowMemory), Value);
	}

	void SetByteEnumPropertyValue(const FByteProperty* Property, void* RowMemory, const int64 Value)
	{
		Property->SetIntPropertyValue(Property->ContainerPtrToValuePtr<void>(RowMemory), Value);
	}

	FString QuoteCsvCell(const FString& Cell)
	{
		return FString(TEXT("\"")) + Cell.Replace(TEXT("\""), TEXT("\"\"")) + TEXT("\"");
	}

	struct FRejectedPropertyCase
	{
		const TCHAR* Description;
		UScriptStruct* RowStruct;
		const TCHAR* PropertyPath;
		const TCHAR* TopLevelColumn;
	};

	struct FInvalidRowKeyCase
	{
		const TCHAR* Description;
		const TCHAR* Key;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilDataTableFunctionLibraryTest,
	"DirectiveUtilities.DataTableFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilDataTableFunctionLibraryTest::RunTest(const FString& Parameters)
{
	FString ErrorMessage;

	UDataTable* NullTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		nullptr, TEXT("A"), EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	TestNull(TEXT("A null row struct builds nothing"), NullTable);
	TestTrue(TEXT("A null row struct reports why"), !ErrorMessage.IsEmpty());
	TestNull(TEXT("A native struct that is not a table row is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			TBaseStructure<FVector>::Get(), TEXT("1,2,3"),
			EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage));

	const FString InventoryCsv = TEXT(
		"Name,Quantity,Weight,bEnchanted,Label,Rarity,Description\n"
		"Sword,3,2.5,true,\"Fine blade\",Rare,\"A keen edge\"\n"
		"Potion,12,0.4,false,,Common,\n");

	UDataTable* Table = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), InventoryCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("A header CSV builds a table"), Table);
	if (Table == nullptr)
	{
		AddError(ErrorMessage);
		return false;
	}

	TestEqual(TEXT("Every data row was imported"), Table->GetRowMap().Num(), 2);
	const UScriptStruct* ImportedRowStruct = Table->GetRowStruct();
	TestTrue(TEXT("The row struct carries over"), ImportedRowStruct == FDirectiveUtilTestInventoryRow::StaticStruct());

	const uint8* SwordBytes = Table->GetRowMap().FindRef(FName(TEXT("Sword")));
	const FDirectiveUtilTestInventoryRow* Sword = reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(SwordBytes);
	TestNotNull(TEXT("The Name column became the row key"), SwordBytes);
	if (Sword != nullptr)
	{
		TestEqual(TEXT("int32 import"), Sword->Quantity, 3);
		TestTrue(TEXT("float import"), FMath::IsNearlyEqual(Sword->Weight, 2.5f));
		TestTrue(TEXT("bool import"), Sword->bEnchanted);
		TestEqual(TEXT("string import"), Sword->Label, FString(TEXT("Fine blade")));
		// An enum written at the row base address instead of the property offset would
		// land on Quantity and leave Rarity at its default.
		TestEqual(TEXT("enumeration import"), Sword->Rarity, EDirectiveUtilTestRarity::Rare);
		TestEqual(TEXT("text import"), Sword->Description.ToString(), FString(TEXT("A keen edge")));
	}

	const uint8* PotionBytes = Table->GetRowMap().FindRef(FName(TEXT("Potion")));
	const FDirectiveUtilTestInventoryRow* Potion = reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(PotionBytes);
	if (Potion != nullptr)
	{
		TestEqual(TEXT("An empty cell imports an empty string"), Potion->Label, FString());
		TestFalse(TEXT("bool false imports as false"), Potion->bEnchanted);
	}

	UDataTable* LifecycleTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestLifecycleRow::StaticStruct(), TEXT("Name,Value\nOne,7\n"),
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	const FDirectiveUtilTestLifecycleRow* LifecycleRow = LifecycleTable
		? reinterpret_cast<const FDirectiveUtilTestLifecycleRow*>(LifecycleTable->GetRowMap().FindRef(FName(TEXT("One"))))
		: nullptr;
	TestNotNull(TEXT("A lifecycle test row imports"), LifecycleRow);
	if (LifecycleRow != nullptr)
	{
		TestTrue(TEXT("Row post-import callbacks run"), LifecycleRow->bPostImported);
		TestTrue(TEXT("Row table-change callbacks run"), LifecycleRow->bTableChanged);
	}

	const FString TextHistoryCsv = TEXT(
		"Name,Description\n"
		"One,\"NSLOCTEXT(\"\"DirectiveTests\"\",\"\"History\"\",\"\"Localized text\"\")\"\n");
	UDataTable* TextHistoryTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), TextHistoryCsv,
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("Serialized FText history imports"), TextHistoryTable);
	FString TextHistoryExport;
	FString TextHistoryExportError;
	if (TextHistoryTable != nullptr)
	{
		TestTrue(TEXT("Serialized FText history exports"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				TextHistoryTable, EDirectiveUtilCsvDelimiter::Comma, true, TextHistoryExport, TextHistoryExportError));
		TestEqual(TEXT("FText exports its display string without a text macro"), TextHistoryExport,
			FString(TEXT("Name,Quantity,Weight,bEnchanted,Label,Rarity,Description\nOne,0,0,false,,Common,Localized text\n")));
	}

	FString Exported;
	FString ExportError;
	TestTrue(TEXT("Export writes the table back to CSV"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(Table, EDirectiveUtilCsvDelimiter::Comma, true, Exported, ExportError));
	TestTrue(TEXT("Export starts with the header"),
		Exported.StartsWith(TEXT("Name,Quantity,Weight,bEnchanted,Label,Rarity,Description\n")));
	TestTrue(TEXT("Export keeps imported values"), Exported.Contains(TEXT("\nSword,3,2.5,true,Fine blade,Rare,A keen edge\n")));
	TestTrue(TEXT("Export writes enumeration names"), Exported.Contains(TEXT(",Rare,")));
	TestTrue(TEXT("Rows export in alphabetical order"),
		Exported.Find(TEXT("\nPotion,")) < Exported.Find(TEXT("\nSword,")));
	FString AliasedExportOutputs;
	TestFalse(TEXT("CSV and error outputs cannot alias"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			Table, EDirectiveUtilCsvDelimiter::Comma, true, AliasedExportOutputs, AliasedExportOutputs));
	TestTrue(TEXT("Aliased export outputs report the problem"), !AliasedExportOutputs.IsEmpty());

	FDirectiveUtilCsvDocument Reparsed;
	FString ReparseError;
	TestTrue(TEXT("Exported text reparses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(Exported, EDirectiveUtilCsvDelimiter::Comma, Reparsed, ReparseError));
	FString ExportedCell;
	// Rows sort alphabetically, so Potion is row 1 and Sword is row 2.
	TestTrue(TEXT("The exported Sword row is readable"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Reparsed, 2, 4, ExportedCell));
	TestEqual(TEXT("Quoted export text survives the round trip"), ExportedCell, FString(TEXT("Fine blade")));

	UDataTable* ReimportedTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), Exported, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("An export reimports through the same node"), ReimportedTable);
	const uint8* ReimportedBytes = ReimportedTable ? ReimportedTable->GetRowMap().FindRef(FName(TEXT("Sword"))) : nullptr;
	const FDirectiveUtilTestInventoryRow* Reimported = reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(ReimportedBytes);
	if (Reimported != nullptr)
	{
		TestEqual(TEXT("Round-tripped int32"), Reimported->Quantity, 3);
		TestEqual(TEXT("Round-tripped enumeration"), Reimported->Rarity, EDirectiveUtilTestRarity::Rare);
		TestEqual(TEXT("Round-tripped text"), Reimported->Description.ToString(), FString(TEXT("A keen edge")));
	}

	FString HeaderlessExport;
	TestTrue(TEXT("Headerless export writes positional rows"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			Table, EDirectiveUtilCsvDelimiter::Comma, false, HeaderlessExport, ExportError));
	UDataTable* HeaderlessReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), HeaderlessExport, EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	TestNotNull(TEXT("Headerless export reimports with the positional layout"), HeaderlessReimport);
	const FDirectiveUtilTestInventoryRow* FirstHeaderlessRow = HeaderlessReimport
		? reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(HeaderlessReimport->GetRowMap().FindRef(FName(TEXT("Row_1"))))
		: nullptr;
	const FDirectiveUtilTestInventoryRow* SecondHeaderlessRow = HeaderlessReimport
		? reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(HeaderlessReimport->GetRowMap().FindRef(FName(TEXT("Row_2"))))
		: nullptr;
	if (FirstHeaderlessRow != nullptr && SecondHeaderlessRow != nullptr)
	{
		TestEqual(TEXT("Headerless row one keeps its first property"), FirstHeaderlessRow->Quantity, 12);
		TestEqual(TEXT("Headerless row two keeps its first property"), SecondHeaderlessRow->Quantity, 3);
		TestEqual(TEXT("Headerless row two keeps its string property"), SecondHeaderlessRow->Label, FString(TEXT("Fine blade")));
	}

	UDataTable* EmptyStringTable = NewObject<UDataTable>();
	EmptyStringTable->RowStruct = FDirectiveUtilTestNameCollisionRow::StaticStruct();
	FDirectiveUtilTestNameCollisionRow EmptyStringRow;
	EmptyStringTable->AddRow(FName(TEXT("Only")), EmptyStringRow);
	FString EmptyStringCsv;
	TestTrue(TEXT("A headerless single empty property exports"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			EmptyStringTable, EDirectiveUtilCsvDelimiter::Comma, false, EmptyStringCsv, ExportError));
	TestEqual(TEXT("The empty property has an explicit CSV field"), EmptyStringCsv, FString(TEXT("\"\"\n")));
	UDataTable* EmptyStringReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestNameCollisionRow::StaticStruct(), EmptyStringCsv,
		EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	const FDirectiveUtilTestNameCollisionRow* ReimportedEmptyString = EmptyStringReimport
		? reinterpret_cast<const FDirectiveUtilTestNameCollisionRow*>(
			EmptyStringReimport->GetRowMap().FindRef(FName(TEXT("Row_1"))))
		: nullptr;
	TestNotNull(TEXT("A headerless single empty property reimports"), ReimportedEmptyString);
	if (ReimportedEmptyString != nullptr)
	{
		TestEqual(TEXT("The empty property remains empty"), ReimportedEmptyString->Name, FString());
	}

	UDataTable* PositionalTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("7,1.25,true,Wand"), EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	TestNotNull(TEXT("Positional import builds a table"), PositionalTable);
	const uint8* GeneratedKeyBytes = PositionalTable ? PositionalTable->GetRowMap().FindRef(FName(TEXT("Row_1"))) : nullptr;
	const FDirectiveUtilTestInventoryRow* GeneratedKey = reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(GeneratedKeyBytes);
	if (GeneratedKey != nullptr)
	{
		TestEqual(TEXT("Positional int32 import"), GeneratedKey->Quantity, 7);
		TestTrue(TEXT("Positional float import"), FMath::IsNearlyEqual(GeneratedKey->Weight, 1.25f));
		TestEqual(TEXT("Generated row key label"), GeneratedKey->Label, FString(TEXT("Wand")));
	}

	const FString WaypointCsv = TEXT(
		"Name,Location,WaypointName\n"
		"Camp,\"(X=100.0,Y=200.0,Z=0.0)\",Camp_Fire\n");
	UDataTable* WaypointTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestWaypointRow::StaticStruct(), WaypointCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	if (WaypointTable == nullptr)
	{
		AddError(FString::Printf(TEXT("Waypoint import failed: %s"), *ErrorMessage));
	}
	const uint8* CampBytes = WaypointTable ? WaypointTable->GetRowMap().FindRef(FName(TEXT("Camp"))) : nullptr;
	const FDirectiveUtilTestWaypointRow* Camp = reinterpret_cast<const FDirectiveUtilTestWaypointRow*>(CampBytes);
	if (Camp != nullptr)
	{
		TestEqual(TEXT("FVector import"), Camp->Location, FVector(100.0, 200.0, 0.0));
		TestEqual(TEXT("FName import"), Camp->WaypointName, FName(TEXT("Camp_Fire")));
	}

	FString WaypointExport;
	if (WaypointTable != nullptr)
	{
		TestTrue(TEXT("The waypoint table exports"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(WaypointTable, EDirectiveUtilCsvDelimiter::Comma, true, WaypointExport, ExportError));
		TestTrue(TEXT("Struct properties export in engine text format"),
			WaypointExport.Contains(TEXT("\nCamp,\"(X=100,Y=200,Z=0)\",Camp_Fire\n")));
	}

	const FString NumericCsv = TEXT(
		"Name,Signed8,Signed16,Signed64,Unsigned8,Unsigned16,Unsigned32,Unsigned64,Single,Double,LegacyRarity\n"
		"Bounds,-128,-32768,-9223372036854775808,255,65535,4294967295,18446744073709551615,3.5,1.7976931348623157e+308,LegacyRare\n");
	UDataTable* NumericTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestNumericRow::StaticStruct(), NumericCsv,
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	const FDirectiveUtilTestNumericRow* NumericRow = NumericTable
		? reinterpret_cast<const FDirectiveUtilTestNumericRow*>(
			NumericTable->GetRowMap().FindRef(FName(TEXT("Bounds"))))
		: nullptr;
	TestNotNull(TEXT("Every supported numeric property imports"), NumericRow);
	if (NumericRow != nullptr)
	{
		TestEqual(TEXT("int8 minimum imports"), NumericRow->Signed8, static_cast<int8>(MIN_int8));
		TestEqual(TEXT("int16 minimum imports"), NumericRow->Signed16, static_cast<int16>(MIN_int16));
		TestEqual(TEXT("int64 minimum imports"), NumericRow->Signed64, static_cast<int64>(MIN_int64));
		TestEqual(TEXT("uint8 maximum imports"), NumericRow->Unsigned8, static_cast<uint8>(MAX_uint8));
		TestEqual(TEXT("uint16 maximum imports"), NumericRow->Unsigned16, static_cast<uint16>(MAX_uint16));
		TestEqual(TEXT("uint32 maximum imports"), NumericRow->Unsigned32, static_cast<uint32>(MAX_uint32));
		TestEqual(TEXT("uint64 maximum imports"), NumericRow->Unsigned64, static_cast<uint64>(MAX_uint64));
		TestEqual(TEXT("float imports"), NumericRow->Single, 3.5f);
		TestEqual(TEXT("double maximum imports"), NumericRow->Double, TNumericLimits<double>::Max());
		TestEqual(TEXT("byte-backed enumeration imports"),
			static_cast<uint8>(NumericRow->LegacyRarity.GetValue()), static_cast<uint8>(LegacyRare));
	}
	FString NumericExport;
	if (NumericTable != nullptr)
	{
		TestTrue(TEXT("Every supported numeric property exports"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				NumericTable, EDirectiveUtilCsvDelimiter::Comma, true, NumericExport, ExportError));
		TestTrue(TEXT("uint64 maximum exports without becoming negative"),
			NumericExport.Contains(TEXT("18446744073709551615")));
		TestTrue(TEXT("A byte-backed enumeration exports its authored name"),
			NumericExport.Contains(TEXT("LegacyRare")));
	}

	const FString UnknownColumnCsv = TEXT("Name,Bogus\nOne,1\n");
	UDataTable* UnknownTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), UnknownColumnCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNull(TEXT("An unknown column rejects the import"), UnknownTable);
	TestTrue(TEXT("The unknown column is named in the error"), ErrorMessage.Contains(TEXT("Bogus")));

	const FString DuplicateCsv = TEXT(
		"Name,Quantity\n"
		"Sword,1\n"
		"Sword,2\n");
	UDataTable* DuplicateTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), DuplicateCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNull(TEXT("Duplicate row names reject the import"), DuplicateTable);
	TestTrue(TEXT("The duplicate is reported"), ErrorMessage.Contains(TEXT("repeats the row name")));

	const FString BadNumberCsv = TEXT(
		"Name,Quantity\n"
		"Sword,abc\n");
	UDataTable* BadNumberTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), BadNumberCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNull(TEXT("A malformed number rejects the import"), BadNumberTable);
	TestTrue(TEXT("The failing property is named"), ErrorMessage.Contains(TEXT("Quantity")));

	const FString BadFloatCsv = TEXT(
		"Name,Weight\n"
		"Sword,1.2.3\n");
	TestNull(TEXT("A malformed float rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), BadFloatCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	const FString OverflowFloatCsv = TEXT(
		"Name,Weight\n"
		"Sword,1e100\n");
	TestNull(TEXT("A double that overflows float rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), OverflowFloatCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));

	const FString BadEnumCsv = TEXT(
		"Name,Rarity\n"
		"Sword,Mythic\n");
	TestNull(TEXT("An unknown enumeration name rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), BadEnumCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));

	const FString OversizedName = FString::ChrN(NAME_SIZE, TEXT('A'));
	TestNull(TEXT("An oversized row name is rejected without constructing an FName"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(),
			FString(TEXT("Name,Quantity\n")) + OversizedName + TEXT(",1\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("The None row name is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Quantity\nNone,1\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("An oversized FName property value is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestWaypointRow::StaticStruct(),
			FString(TEXT("Name,WaypointName\nOne,")) + OversizedName + TEXT("\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("An oversized enumeration name is rejected without constructing an FName"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(),
			FString(TEXT("Name,Rarity\nOne,")) + OversizedName + TEXT("\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));

	TestNull(TEXT("A headered row with extra cells is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Quantity\nOne,1,extra\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("A headered row with missing cells is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Quantity\nOne\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("A headerless row with extra cells is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("1,2,true,label,Common,text,extra\n"),
			EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage));
	TestNull(TEXT("Trailing struct text is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestWaypointRow::StaticStruct(),
			TEXT("Name,Location\nOne,\"(X=1,Y=2,Z=3)junk\"\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("Fixed-size reflected arrays are rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestFixedArrayRow::StaticStruct(), TEXT("Name,FixedValues\nOne,1\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The fixed-size array error names the property"), ErrorMessage.Contains(TEXT("FixedValues")));
	TestNull(TEXT("A struct that can reference objects is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInstancedStructRow::StaticStruct(), TEXT("Name,Payload\nOne,()\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The object-referencing struct error names the property"), ErrorMessage.Contains(TEXT("Payload")));
	UDataTable* InstancedStructTable = NewObject<UDataTable>();
	InstancedStructTable->RowStruct = FDirectiveUtilTestInstancedStructRow::StaticStruct();
	FString InstancedStructExport;
	TestFalse(TEXT("A struct that can reference objects does not export"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			InstancedStructTable, EDirectiveUtilCsvDelimiter::Comma, true, InstancedStructExport, ErrorMessage));

	TestNull(TEXT("An invalid delimiter is rejected by DataTable import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), InventoryCsv,
			static_cast<EDirectiveUtilCsvDelimiter>(255), true, ErrorMessage));
	FString InvalidDelimiterExport;
	TestFalse(TEXT("An invalid delimiter is rejected by DataTable export"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			Table, static_cast<EDirectiveUtilCsvDelimiter>(255), true, InvalidDelimiterExport, ExportError));

	FString AliasedCsv(TEXT("Name,Quantity\nAliased,4\n"));
	UDataTable* AliasedTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), AliasedCsv,
		EDirectiveUtilCsvDelimiter::Comma, true, AliasedCsv);
	TestNotNull(TEXT("DataTable CSV text may alias the error output"), AliasedTable);
	TestEqual(TEXT("A successful aliased DataTable import clears its error"), AliasedCsv, FString());

	const FString BlankLineCsv = TEXT(
		"Name,Quantity\n"
		"Sword,1\n"
		"\n"
		"Shield,2\n");
	UDataTable* BlankLineTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), BlankLineCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("Blank lines do not break the import"), BlankLineTable);
	if (BlankLineTable != nullptr)
	{
		TestEqual(TEXT("Blank lines contribute no rows"), BlankLineTable->GetRowMap().Num(), 2);
	}

	const FString DuplicateColumnCsv = TEXT(
		"Name,Quantity,quantity\n"
		"Sword,1,2\n");
	TestNull(TEXT("Two columns cannot bind the same property"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), DuplicateColumnCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));

	const FString NestedCollectionCsv = TEXT(
		"Name,Payload\n"
		"One,\"(Values=())\"\n");
	TestNull(TEXT("A collection nested inside a struct rejects import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestNestedCollectionRow::StaticStruct(), NestedCollectionCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The nested unsupported property is named"), ErrorMessage.Contains(TEXT("Payload.Values")));

	UDataTable* NestedCollectionTable = NewObject<UDataTable>();
	NestedCollectionTable->RowStruct = FDirectiveUtilTestNestedCollectionRow::StaticStruct();
	TestFalse(TEXT("A collection nested inside a struct rejects export"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			NestedCollectionTable, EDirectiveUtilCsvDelimiter::Comma, true, Exported, ExportError));

	UDataTable* NameCollisionTable = NewObject<UDataTable>();
	NameCollisionTable->RowStruct = FDirectiveUtilTestNameCollisionRow::StaticStruct();
	TestFalse(TEXT("Header export rejects a property named Name"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			NameCollisionTable, EDirectiveUtilCsvDelimiter::Comma, true, Exported, ExportError));

	UDataTable* InvalidRowTable = NewObject<UDataTable>();
	InvalidRowTable->RowStruct = TBaseStructure<FVector>::Get();
	TestFalse(TEXT("Export rejects a native struct that is not a table row"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			InvalidRowTable, EDirectiveUtilCsvDelimiter::Comma, false, Exported, ExportError));
	TestFalse(TEXT("Replacement rejects a native struct that is not a table row"),
		UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
			InvalidRowTable, TEXT("1,2,3"), EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage));

	UDataTable* NonFiniteTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Weight\nOne,1\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	FDirectiveUtilTestInventoryRow* NonFiniteRow = NonFiniteTable
		? reinterpret_cast<FDirectiveUtilTestInventoryRow*>(NonFiniteTable->GetRowMap().FindRef(FName(TEXT("One"))))
		: nullptr;
	if (NonFiniteRow != nullptr)
	{
		NonFiniteRow->Weight = std::numeric_limits<float>::infinity();
		TestFalse(TEXT("A non-finite number rejects export"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				NonFiniteTable, EDirectiveUtilCsvDelimiter::Comma, true, Exported, ExportError));
	}

	UCompositeDataTable* CompositeTable = NewObject<UCompositeDataTable>();
	CompositeTable->RowStruct = FDirectiveUtilTestInventoryRow::StaticStruct();
	const int32 CompositeRowsBeforeReplacement = CompositeTable->GetRowMap().Num();
	TestFalse(TEXT("Composite Data Tables reject direct replacement"),
		UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
			CompositeTable, InventoryCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestEqual(TEXT("A rejected composite replacement preserves its rows"),
		CompositeTable->GetRowMap().Num(), CompositeRowsBeforeReplacement);

	if (LifecycleTable != nullptr)
	{
		TestTrue(TEXT("Replacement runs lifecycle callbacks on the target table"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				LifecycleTable, TEXT("Name,Value\nTwo,9\n"),
				EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		const FDirectiveUtilTestLifecycleRow* ReplacedLifecycleRow =
			reinterpret_cast<const FDirectiveUtilTestLifecycleRow*>(
				LifecycleTable->GetRowMap().FindRef(FName(TEXT("Two"))));
		TestNotNull(TEXT("The lifecycle replacement row exists"), ReplacedLifecycleRow);
		if (ReplacedLifecycleRow != nullptr)
		{
			TestTrue(TEXT("Replacement calls row post-import"), ReplacedLifecycleRow->bPostImported);
			TestTrue(TEXT("Replacement calls row table-change"), ReplacedLifecycleRow->bTableChanged);
		}
	}

	UDataTable* MutableTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), InventoryCsv,
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("A table for replacement was created"), MutableTable);
	if (MutableTable != nullptr)
	{
		TestFalse(TEXT("An invalid replacement fails"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				MutableTable, BadNumberCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestEqual(TEXT("A failed replacement preserves every row"), MutableTable->GetRowMap().Num(), 2);
		const FDirectiveUtilTestInventoryRow* PreservedSword = reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(
			MutableTable->GetRowMap().FindRef(FName(TEXT("Sword"))));
		if (PreservedSword != nullptr)
		{
			TestEqual(TEXT("A failed replacement preserves row values"), PreservedSword->Quantity, 3);
		}

		const FString ReplacementCsv = TEXT(
			"Name,Quantity,Weight,bEnchanted,Label,Rarity,Description\n"
			"Sword,9,2.5,true,Changed,Legendary,Updated\n"
			"Bow,2,1.0,false,Longbow,Common,Ranged\n");
		TestTrue(TEXT("A valid replacement succeeds"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				MutableTable, ReplacementCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestEqual(TEXT("Replacement installs every new row"), MutableTable->GetRowMap().Num(), 2);
		TestFalse(TEXT("Replacement removes old rows"), MutableTable->GetRowMap().Contains(FName(TEXT("Potion"))));

		TArray<FName> AddedRows;
		TArray<FName> RemovedRows;
		TArray<FName> ChangedRows;
		TestTrue(TEXT("Tables with the same row struct diff"),
			UDirectiveUtilDataTableFunctionLibrary::DiffDataTables(
				Table, MutableTable, AddedRows, RemovedRows, ChangedRows, ErrorMessage));
		TestEqual(TEXT("The added row is reported"), AddedRows, TArray<FName>({ FName(TEXT("Bow")) }));
		TestEqual(TEXT("The removed row is reported"), RemovedRows, TArray<FName>({ FName(TEXT("Potion")) }));
		TestEqual(TEXT("The changed row is reported"), ChangedRows, TArray<FName>({ FName(TEXT("Sword")) }));

		TestFalse(TEXT("Tables with different row structs do not diff"),
			UDirectiveUtilDataTableFunctionLibrary::DiffDataTables(
				Table, WaypointTable, AddedRows, RemovedRows, ChangedRows, ErrorMessage));
		TestTrue(TEXT("A failed diff clears added rows"), AddedRows.IsEmpty());
		TestTrue(TEXT("A failed diff clears removed rows"), RemovedRows.IsEmpty());
		TestTrue(TEXT("A failed diff clears changed rows"), ChangedRows.IsEmpty());

		TArray<FName> SharedDiffOutput;
		TestFalse(TEXT("Diff output arrays cannot alias"),
			UDirectiveUtilDataTableFunctionLibrary::DiffDataTables(
				Table, MutableTable, SharedDiffOutput, SharedDiffOutput, ChangedRows, ErrorMessage));
		TestTrue(TEXT("An aliased diff output reports the problem"), !ErrorMessage.IsEmpty());
	}

	const FString PrecisionCsv = TEXT(
		"Name,Single,Double,Location\n"
		"Small,0.0000001,123.4567891,\"(X=0.0000001,Y=123.4567891,Z=1e-30)\"\n"
		"Tiny,1e-30,0.30000000000000004,\"(X=0.30000000000000004,Y=-2.5e-300,Z=1e+300)\"\n");
	UDataTable* PrecisionTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestPrecisionRow::StaticStruct(), PrecisionCsv, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("Small and long floating-point values import"), PrecisionTable);
	FString PrecisionExport;
	if (PrecisionTable != nullptr)
	{
		TestTrue(TEXT("Floating-point values export"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				PrecisionTable, EDirectiveUtilCsvDelimiter::Comma, true, PrecisionExport, ExportError));
		TestTrue(TEXT("A double exports every significant digit"), PrecisionExport.Contains(TEXT("123.4567891")));
		TestTrue(TEXT("A double that needs 17 digits exports all of them"), PrecisionExport.Contains(TEXT("0.30000000000000004")));
	}
	UDataTable* PrecisionReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestPrecisionRow::StaticStruct(), PrecisionExport, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("The floating-point export reimports"), PrecisionReimport);
	const FDirectiveUtilTestPrecisionRow* SmallRow = PrecisionReimport
		? reinterpret_cast<const FDirectiveUtilTestPrecisionRow*>(PrecisionReimport->GetRowMap().FindRef(FName(TEXT("Small"))))
		: nullptr;
	const FDirectiveUtilTestPrecisionRow* TinyRow = PrecisionReimport
		? reinterpret_cast<const FDirectiveUtilTestPrecisionRow*>(PrecisionReimport->GetRowMap().FindRef(FName(TEXT("Tiny"))))
		: nullptr;
	if (SmallRow != nullptr && TinyRow != nullptr)
	{
		TestTrue(TEXT("float 0.0000001 survives export exactly"), SmallRow->Single == static_cast<float>(0.0000001));
		TestTrue(TEXT("double 123.4567891 survives export exactly"), SmallRow->Double == 123.4567891);
		TestTrue(TEXT("Nested struct doubles survive export exactly"),
			SmallRow->Location == FVector(0.0000001, 123.4567891, 1e-30));
		TestTrue(TEXT("float 1e-30 survives export exactly"), TinyRow->Single == static_cast<float>(1e-30));
		TestTrue(TEXT("A 17-digit double survives export exactly"), TinyRow->Double == 0.30000000000000004);
		TestTrue(TEXT("Nested extreme doubles survive export exactly"),
			TinyRow->Location == FVector(0.30000000000000004, -2.5e-300, 1e+300));
	}

	UDataTable* FloatLimitSource = NewObject<UDataTable>();
	FloatLimitSource->RowStruct = FDirectiveUtilTestPrecisionRow::StaticStruct();
	FDirectiveUtilTestPrecisionRow FloatLimitRow;
	FloatLimitRow.Single = TNumericLimits<float>::Max();
	FloatLimitSource->AddRow(FName(TEXT("Highest")), FloatLimitRow);
	FloatLimitRow.Single = TNumericLimits<float>::Lowest();
	FloatLimitSource->AddRow(FName(TEXT("Lowest")), FloatLimitRow);
	FString FloatLimitExport;
	TestTrue(TEXT("The float limits export"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			FloatLimitSource, EDirectiveUtilCsvDelimiter::Comma, true, FloatLimitExport, ExportError));
	UDataTable* FloatLimitReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestPrecisionRow::StaticStruct(), FloatLimitExport, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("The float limit export reimports"), FloatLimitReimport);
	const FDirectiveUtilTestPrecisionRow* HighestRow = FloatLimitReimport
		? reinterpret_cast<const FDirectiveUtilTestPrecisionRow*>(FloatLimitReimport->GetRowMap().FindRef(FName(TEXT("Highest"))))
		: nullptr;
	const FDirectiveUtilTestPrecisionRow* LowestRow = FloatLimitReimport
		? reinterpret_cast<const FDirectiveUtilTestPrecisionRow*>(FloatLimitReimport->GetRowMap().FindRef(FName(TEXT("Lowest"))))
		: nullptr;
	TestTrue(TEXT("The largest float survives export exactly"),
		HighestRow != nullptr && HighestRow->Single == TNumericLimits<float>::Max());
	TestTrue(TEXT("The lowest float survives export exactly"),
		LowestRow != nullptr && LowestRow->Single == TNumericLimits<float>::Lowest());
	TestNull(TEXT("A float cell that rounds past the largest float is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestPrecisionRow::StaticStruct(), TEXT("Name,Single\nOver,3.4028236e38\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));

	UDataTable* StructTextSource = NewObject<UDataTable>();
	StructTextSource->RowStruct = FDirectiveUtilTestStructTextRow::StaticStruct();
	FDirectiveUtilTestStructTextRow StructTextRow;
	StructTextRow.Nested.Label = TEXT("Comma, \"quote\", back\\slash (paren)");
	StructTextRow.Nested.Caption = FText::FromString(TEXT("Plain caption"));
	StructTextRow.Nested.Rarity = EDirectiveUtilTestRarity::Legendary;
	StructTextRow.Nested.Scale = 0.1f;
	StructTextRow.Id = FGuid(0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF);
	StructTextSource->AddRow(FName(TEXT("One")), StructTextRow);
	FString StructTextExport;
	TestTrue(TEXT("Nested and native-text structs export"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
			StructTextSource, EDirectiveUtilCsvDelimiter::Comma, true, StructTextExport, ExportError));
	TestTrue(TEXT("A native-text struct exports its own format"),
		StructTextExport.Contains(TEXT("0123456789ABCDEF0123456789ABCDEF")));
	TestFalse(TEXT("Nested text exports without a text macro"),
		StructTextExport.Contains(TEXT("INVTEXT")) || StructTextExport.Contains(TEXT("NSLOCTEXT")));
	UDataTable* StructTextReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestStructTextRow::StaticStruct(), StructTextExport, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("The struct text export reimports"), StructTextReimport);
	const FDirectiveUtilTestStructTextRow* ReimportedStructText = StructTextReimport
		? reinterpret_cast<const FDirectiveUtilTestStructTextRow*>(StructTextReimport->GetRowMap().FindRef(FName(TEXT("One"))))
		: nullptr;
	if (ReimportedStructText != nullptr)
	{
		TestEqual(TEXT("A quoted nested string survives"), ReimportedStructText->Nested.Label, StructTextRow.Nested.Label);
		TestEqual(TEXT("Nested text survives as plain text"), ReimportedStructText->Nested.Caption.ToString(), FString(TEXT("Plain caption")));
		TestEqual(TEXT("A nested enumeration survives"), ReimportedStructText->Nested.Rarity, EDirectiveUtilTestRarity::Legendary);
		TestTrue(TEXT("A nested float survives exactly"), ReimportedStructText->Nested.Scale == 0.1f);
		TestTrue(TEXT("A native-text struct survives"), ReimportedStructText->Id == StructTextRow.Id);
	}

	TestNull(TEXT("An unknown struct member rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestWaypointRow::StaticStruct(), TEXT("Name,Location\nOne,\"(X=1,Yy=2,Z=3)\"\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The unknown struct member is named"), ErrorMessage.Contains(TEXT("Yy")));
	TestNull(TEXT("A malformed struct member value rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestWaypointRow::StaticStruct(), TEXT("Name,Location\nOne,\"(X=abc,Y=2,Z=3)\"\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The malformed struct member is named"), ErrorMessage.Contains(TEXT("Location.X")));
	TestNull(TEXT("A repeated struct member rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestWaypointRow::StaticStruct(), TEXT("Name,Location\nOne,\"(X=1,X=2)\"\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("Invalid text for a native-text struct rejects the import"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestStructTextRow::StaticStruct(), TEXT("Name,Id\nOne,(Bogus=1)\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	UDataTable* UnwrappedStructTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestWaypointRow::StaticStruct(), TEXT("Name,Location\nOne,\"X=1,Y=2,Z=3\"\n"),
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	const FDirectiveUtilTestWaypointRow* UnwrappedStructRow = UnwrappedStructTable
		? reinterpret_cast<const FDirectiveUtilTestWaypointRow*>(UnwrappedStructTable->GetRowMap().FindRef(FName(TEXT("One"))))
		: nullptr;
	TestNotNull(TEXT("Struct members without parentheses import"), UnwrappedStructRow);
	if (UnwrappedStructRow != nullptr)
	{
		TestTrue(TEXT("Unwrapped struct members keep their values"), UnwrappedStructRow->Location == FVector(1.0, 2.0, 3.0));
	}

	UDataTable* EmptyRowTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestEmptyRow::StaticStruct(), TEXT("Name\nAlpha\nBeta\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("A row struct with no properties imports keyed rows"), EmptyRowTable);
	if (EmptyRowTable != nullptr)
	{
		TestEqual(TEXT("Every key-only row imports"), EmptyRowTable->GetRowMap().Num(), 2);
		FString EmptyRowExport;
		TestTrue(TEXT("A row struct with no properties exports"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				EmptyRowTable, EDirectiveUtilCsvDelimiter::Comma, true, EmptyRowExport, ExportError));
		TestEqual(TEXT("Key-only rows export as a Name column"), EmptyRowExport, FString(TEXT("Name\nAlpha\nBeta\n")));
		TestTrue(TEXT("A row struct with no properties can be replaced"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				EmptyRowTable, TEXT("Name\nGamma\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestTrue(TEXT("The replacement key-only row exists"), EmptyRowTable->GetRowMap().Contains(FName(TEXT("Gamma"))));
		TestEqual(TEXT("The replacement leaves one key-only row"), EmptyRowTable->GetRowMap().Num(), 1);
	}
	UDataTable* HeaderlessEmptyRowTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestEmptyRow::StaticStruct(), TEXT("\"\"\n\"\"\n"), EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	TestNotNull(TEXT("Headerless rows import for a row struct with no properties"), HeaderlessEmptyRowTable);
	if (HeaderlessEmptyRowTable != nullptr)
	{
		TestEqual(TEXT("Each headerless empty row becomes a generated key"), HeaderlessEmptyRowTable->GetRowMap().Num(), 2);
	}

	TestNull(TEXT("A post-import problem rejects creation"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestRejectingRow::StaticStruct(), TEXT("Name,Value\nBad,-1\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The post-import problem is reported"), ErrorMessage.Contains(TEXT("negative value")));
	UDataTable* RejectingTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestRejectingRow::StaticStruct(), TEXT("Name,Value\nKeep,1\n"),
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	TestNotNull(TEXT("A table with a post-import check imports valid rows"), RejectingTable);
	if (RejectingTable != nullptr)
	{
		int32 ChangeBroadcasts = 0;
		const FDelegateHandle ChangeHandle = RejectingTable->OnDataTableChanged().AddLambda([&ChangeBroadcasts]()
		{
			++ChangeBroadcasts;
		});
		const uint8* KeepRowBefore = RejectingTable->GetRowMap().FindRef(FName(TEXT("Keep")));
		TestFalse(TEXT("A post-import problem rejects replacement"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				RejectingTable, TEXT("Name,Value\nBad,-1\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestEqual(TEXT("A rejected replacement does not broadcast a change"), ChangeBroadcasts, 0);
		TestEqual(TEXT("A rejected replacement keeps the row count"), RejectingTable->GetRowMap().Num(), 1);
		TestTrue(TEXT("A rejected replacement keeps the original row memory"),
			RejectingTable->GetRowMap().FindRef(FName(TEXT("Keep"))) == KeepRowBefore);
		TestTrue(TEXT("A valid replacement after a rejected one succeeds"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				RejectingTable, TEXT("Name,Value\nNew,2\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestEqual(TEXT("A successful replacement broadcasts one change"), ChangeBroadcasts, 1);
		TestFalse(TEXT("A successful replacement removes the old row"), RejectingTable->GetRowMap().Contains(FName(TEXT("Keep"))));
		RejectingTable->OnDataTableChanged().Remove(ChangeHandle);
	}

	TestNull(TEXT("A header row without a Name column is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Quantity\n3\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The missing key column is explained"), ErrorMessage.Contains(TEXT("Name column")));
	TestNull(TEXT("A row name that is not a valid key is rejected rather than renamed"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Quantity\nIron Sword,1\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestTrue(TEXT("The rejected row name is quoted"), ErrorMessage.Contains(TEXT("Iron Sword")));

	TestNull(TEXT("The generated MAX entry of an enumeration is rejected by name"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Rarity\nOne,EDirectiveUtilTestRarity_MAX\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("The generated MAX entry of an enumeration is rejected by value"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Rarity\nOne,3\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	TestNull(TEXT("The generated MAX entry of a byte-backed enumeration is rejected"),
		UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
			FDirectiveUtilTestNumericRow::StaticStruct(), TEXT("Name,LegacyRarity\nOne,EDirectiveUtilTestLegacyRarity_MAX\n"),
			EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
	UDataTable* MaxEnumTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(), TEXT("Name,Rarity\nOne,Legendary\n"),
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	FDirectiveUtilTestInventoryRow* MaxEnumRow = MaxEnumTable
		? reinterpret_cast<FDirectiveUtilTestInventoryRow*>(MaxEnumTable->GetRowMap().FindRef(FName(TEXT("One"))))
		: nullptr;
	TestNotNull(TEXT("The last declared enumeration value imports"), MaxEnumRow);
	if (MaxEnumRow != nullptr)
	{
		MaxEnumRow->Rarity = static_cast<EDirectiveUtilTestRarity>(3);
		FString MaxEnumExport;
		TestFalse(TEXT("The generated MAX entry of an enumeration is rejected on export"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				MaxEnumTable, EDirectiveUtilCsvDelimiter::Comma, true, MaxEnumExport, ExportError));
		TestTrue(TEXT("The MAX export failure names the property"), ExportError.Contains(TEXT("Rarity")));
	}

	UDataTable* ByteOrderMarkTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestInventoryRow::StaticStruct(),
		FString::Chr(static_cast<TCHAR>(0xFEFF)) + TEXT("Name,Quantity\nMarked,4\n"),
		EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
	const FDirectiveUtilTestInventoryRow* ByteOrderMarkRow = ByteOrderMarkTable
		? reinterpret_cast<const FDirectiveUtilTestInventoryRow*>(ByteOrderMarkTable->GetRowMap().FindRef(FName(TEXT("Marked"))))
		: nullptr;
	TestNotNull(TEXT("A leading byte-order mark does not hide the Name header"), ByteOrderMarkRow);
	if (ByteOrderMarkRow != nullptr)
	{
		TestEqual(TEXT("A leading byte-order mark does not hide other headers"), ByteOrderMarkRow->Quantity, 4);
	}

	const FString SparseEnumCsv = TEXT(
		"Name,Standing,Grade\n"
		"Authored,High,High\n"
		"FullName,EDirectiveUtilTestSparseRank::High,EDirectiveUtilTestSparseTier::High\n"
		"HighValue,10,20\n"
		"LowValue,1,2\n");
	UDataTable* SparseEnumTable = ImportWithHeader(FDirectiveUtilTestSparseEnumRow::StaticStruct(), SparseEnumCsv, ErrorMessage);
	TestNotNull(TEXT("Every accepted enumeration form imports"), SparseEnumTable);
	if (SparseEnumTable != nullptr)
	{
		TestEqual(TEXT("Every enumeration form row is imported"), SparseEnumTable->GetRowMap().Num(), 4);
		const TCHAR* const HighRowNames[] = { TEXT("Authored"), TEXT("FullName"), TEXT("HighValue") };
		for (const TCHAR* RowName : HighRowNames)
		{
			const FDirectiveUtilTestSparseEnumRow* HighRow = FindRowByName<const FDirectiveUtilTestSparseEnumRow>(SparseEnumTable, RowName);
			const bool bHighRowFound = HighRow != nullptr;
			TestTrue(FString::Printf(TEXT("Enumeration row %s is imported"), RowName), bHighRowFound);
			if (HighRow != nullptr)
			{
				TestEqual(FString::Printf(TEXT("Enumeration row %s sets the enum class to High"), RowName),
					HighRow->Standing, EDirectiveUtilTestSparseRank::High);
				TestEqual(FString::Printf(TEXT("Enumeration row %s sets the TEnumAsByte to High"), RowName),
					static_cast<uint8>(HighRow->Grade.GetValue()), static_cast<uint8>(EDirectiveUtilTestSparseTier::High));
			}
		}
		const FDirectiveUtilTestSparseEnumRow* LowRow = FindRowByName<const FDirectiveUtilTestSparseEnumRow>(SparseEnumTable, TEXT("LowValue"));
		TestNotNull(TEXT("The declared value 1 imports"), LowRow);
		if (LowRow != nullptr)
		{
			TestEqual(TEXT("The declared value 1 is Low rather than the entry at index 1"),
				LowRow->Standing, EDirectiveUtilTestSparseRank::Low);
			TestEqual(TEXT("The declared byte value 2 is Low rather than the entry at index 2"),
				static_cast<uint8>(LowRow->Grade.GetValue()), static_cast<uint8>(EDirectiveUtilTestSparseTier::Low));
		}

		FString SparseEnumExport;
		TestTrue(TEXT("Enumeration rows export"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				SparseEnumTable, EDirectiveUtilCsvDelimiter::Comma, true, SparseEnumExport, ExportError));
		FString SparseCell;
		TestTrue(TEXT("The enum class cell of a full-name import is exported"),
			FindExportedCell(SparseEnumExport, TEXT("FullName"), TEXT("Standing"), SparseCell));
		TestEqual(TEXT("An enum class exports its authored name"), SparseCell, FString(TEXT("High")));
		TestTrue(TEXT("The TEnumAsByte cell of a numeric import is exported"),
			FindExportedCell(SparseEnumExport, TEXT("HighValue"), TEXT("Grade"), SparseCell));
		TestEqual(TEXT("A TEnumAsByte exports its authored name"), SparseCell, FString(TEXT("High")));
		TestTrue(TEXT("The enum class cell of a numeric import is exported"),
			FindExportedCell(SparseEnumExport, TEXT("LowValue"), TEXT("Standing"), SparseCell));
		TestEqual(TEXT("An enum class value of 1 exports as Low"), SparseCell, FString(TEXT("Low")));

		FDirectiveUtilTestSparseEnumRow* MutableSparseRow = FindRowByName<FDirectiveUtilTestSparseEnumRow>(SparseEnumTable, TEXT("Authored"));
		const FEnumProperty* StandingProperty = FindFProperty<FEnumProperty>(FDirectiveUtilTestSparseEnumRow::StaticStruct(), TEXT("Standing"));
		const FByteProperty* GradeProperty = FindFProperty<FByteProperty>(FDirectiveUtilTestSparseEnumRow::StaticStruct(), TEXT("Grade"));
		TestNotNull(TEXT("The enum class property is reflected as an enum property"), StandingProperty);
		TestNotNull(TEXT("The TEnumAsByte property is reflected as a byte property"), GradeProperty);
		if (MutableSparseRow != nullptr && StandingProperty != nullptr && GradeProperty != nullptr)
		{
			const int64 HighStanding = static_cast<int64>(EDirectiveUtilTestSparseRank::High);
			const int64 HighGrade = static_cast<int64>(EDirectiveUtilTestSparseTier::High);
			const int64 UndeclaredValues[] = { 0, 5, 255 };
			for (const int64 UndeclaredValue : UndeclaredValues)
			{
				SetEnumPropertyValue(StandingProperty, MutableSparseRow, UndeclaredValue);
				FString UndeclaredExport(TEXT("stale"));
				TestFalse(FString::Printf(TEXT("An enum class holding undeclared value %lld fails export"), UndeclaredValue),
					UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
						SparseEnumTable, EDirectiveUtilCsvDelimiter::Comma, true, UndeclaredExport, ExportError));
				TestTrue(FString::Printf(TEXT("The undeclared enum class value %lld names its property"), UndeclaredValue),
					ExportError.Contains(TEXT("Standing")));
				TestTrue(FString::Printf(TEXT("A failed export of enum class value %lld clears the CSV output"), UndeclaredValue),
					UndeclaredExport.IsEmpty());
				SetEnumPropertyValue(StandingProperty, MutableSparseRow, HighStanding);

				SetByteEnumPropertyValue(GradeProperty, MutableSparseRow, UndeclaredValue);
				TestFalse(FString::Printf(TEXT("A TEnumAsByte holding undeclared value %lld fails export"), UndeclaredValue),
					UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
						SparseEnumTable, EDirectiveUtilCsvDelimiter::Comma, true, UndeclaredExport, ExportError));
				TestTrue(FString::Printf(TEXT("The undeclared TEnumAsByte value %lld names its property"), UndeclaredValue),
					ExportError.Contains(TEXT("Grade")));
				TestTrue(FString::Printf(TEXT("A failed export of TEnumAsByte value %lld clears the CSV output"), UndeclaredValue),
					UndeclaredExport.IsEmpty());
				SetByteEnumPropertyValue(GradeProperty, MutableSparseRow, HighGrade);
			}
			TestTrue(TEXT("Restoring declared enumeration values makes the table export again"),
				UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
					SparseEnumTable, EDirectiveUtilCsvDelimiter::Comma, true, SparseEnumExport, ExportError));
		}
	}

	const TCHAR* const RejectedStandingCells[] = {
		TEXT("0"), TEXT("2"), TEXT("-1"), TEXT("300"), TEXT("Medium"), TEXT("EDirectiveUtilTestSparseRank::Medium") };
	for (const TCHAR* RejectedCell : RejectedStandingCells)
	{
		const bool bRejected = ImportWithHeader(FDirectiveUtilTestSparseEnumRow::StaticStruct(),
			FString::Printf(TEXT("Name,Standing\nOne,%s\n"), RejectedCell), ErrorMessage) == nullptr;
		TestTrue(FString::Printf(TEXT("The enum class cell '%s' is rejected"), RejectedCell), bRejected);
		TestTrue(FString::Printf(TEXT("The rejected enum class cell '%s' names its property"), RejectedCell),
			ErrorMessage.Contains(TEXT("Standing"), ESearchCase::IgnoreCase));
	}
	const TCHAR* const RejectedGradeCells[] = {
		TEXT("0"), TEXT("5"), TEXT("-1"), TEXT("300"), TEXT("Medium"), TEXT("EDirectiveUtilTestSparseTier::Medium") };
	for (const TCHAR* RejectedCell : RejectedGradeCells)
	{
		const bool bRejected = ImportWithHeader(FDirectiveUtilTestSparseEnumRow::StaticStruct(),
			FString::Printf(TEXT("Name,Grade\nOne,%s\n"), RejectedCell), ErrorMessage) == nullptr;
		TestTrue(FString::Printf(TEXT("The TEnumAsByte cell '%s' is rejected"), RejectedCell), bRejected);
		TestTrue(FString::Printf(TEXT("The rejected TEnumAsByte cell '%s' names its property"), RejectedCell),
			ErrorMessage.Contains(TEXT("Grade"), ESearchCase::IgnoreCase));
	}

	const FString TextFormsCsv = TEXT(
		"Name,Description\n"
		"Loc,\"LOCTEXT(\"\"DirectiveTestKey\"\",\"\"Loc source\"\")\"\n"
		"Ns,\"NSLOCTEXT(\"\"DirectiveTests\"\",\"\"NsKey\"\",\"\"Ns source\"\")\"\n"
		"Inv,\"INVTEXT(\"\"Invariant source\"\")\"\n"
		"Plain,Plain words\n");
	UDataTable* TextFormsTable = ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(), TextFormsCsv, ErrorMessage);
	TestNotNull(TEXT("Text cells in every macro form import"), TextFormsTable);
	if (TextFormsTable != nullptr)
	{
		const FDirectiveUtilTestInventoryRow* LocRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TextFormsTable, TEXT("Loc"));
		const FDirectiveUtilTestInventoryRow* NsRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TextFormsTable, TEXT("Ns"));
		const FDirectiveUtilTestInventoryRow* InvRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TextFormsTable, TEXT("Inv"));
		const FDirectiveUtilTestInventoryRow* PlainRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TextFormsTable, TEXT("Plain"));
		TestNotNull(TEXT("The LOCTEXT row imports"), LocRow);
		TestNotNull(TEXT("The NSLOCTEXT row imports"), NsRow);
		TestNotNull(TEXT("The INVTEXT row imports"), InvRow);
		TestNotNull(TEXT("The plain text row imports"), PlainRow);
		if (LocRow != nullptr)
		{
			TestEqual(TEXT("A LOCTEXT cell imports its source string"), LocRow->Description.ToString(), FString(TEXT("Loc source")));
		}
		if (NsRow != nullptr)
		{
			TestEqual(TEXT("An NSLOCTEXT cell imports its source string"), NsRow->Description.ToString(), FString(TEXT("Ns source")));
			TestEqual(TEXT("An NSLOCTEXT cell keeps its namespace"),
				FTextInspector::GetNamespace(NsRow->Description).Get(FString()), FString(TEXT("DirectiveTests")));
			TestEqual(TEXT("An NSLOCTEXT cell keeps its key"),
				FTextInspector::GetKey(NsRow->Description).Get(FString()), FString(TEXT("NsKey")));
		}
		if (InvRow != nullptr)
		{
			TestEqual(TEXT("An INVTEXT cell imports its string"), InvRow->Description.ToString(), FString(TEXT("Invariant source")));
			TestTrue(TEXT("An INVTEXT cell imports culture-invariant text"), InvRow->Description.IsCultureInvariant());
		}
		if (PlainRow != nullptr)
		{
			TestEqual(TEXT("A plain cell imports as a literal string"), PlainRow->Description.ToString(), FString(TEXT("Plain words")));
		}
	}
	const UDataTable* StringTableCellTable = ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(),
		TEXT("Name,Description\nTable,\"LOCTABLE(\"\"DirectiveTestsTable\"\", \"\"Key\"\")\"\n"), ErrorMessage);
	const FDirectiveUtilTestInventoryRow* StringTableRow = StringTableCellTable != nullptr
		? FindRowByName<const FDirectiveUtilTestInventoryRow>(StringTableCellTable, TEXT("Table"))
		: nullptr;
	TestNotNull(TEXT("A LOCTABLE cell imports"), StringTableRow);
	if (StringTableRow != nullptr)
	{
		TestEqual(TEXT("A LOCTABLE cell imports as its literal text instead of a string table reference"),
			StringTableRow->Description.ToString(), FString(TEXT("LOCTABLE(\"DirectiveTestsTable\", \"Key\")")));
		TestFalse(TEXT("A LOCTABLE cell does not reference a string table"), StringTableRow->Description.IsFromStringTable());
	}
	TestNull(TEXT("A text macro followed by other text is rejected"),
		ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(),
			TEXT("Name,Description\nOne,\"INVTEXT(\"\"Invariant\"\") trailing\"\n"), ErrorMessage));
	TestTrue(TEXT("The rejected text names its property"), ErrorMessage.Contains(TEXT("Description"), ESearchCase::IgnoreCase));

	const FRejectedPropertyCase RejectedCases[] = {
		{ TEXT("An object reference"), FDirectiveUtilTestObjectReferenceRow::StaticStruct(), TEXT("ObjectReference"), TEXT("ObjectReference") },
		{ TEXT("A soft object pointer"), FDirectiveUtilTestSoftReferenceRow::StaticStruct(), TEXT("SoftReference"), TEXT("SoftReference") },
		{ TEXT("A set"), FDirectiveUtilTestSetRow::StaticStruct(), TEXT("Members"), TEXT("Members") },
		{ TEXT("A map"), FDirectiveUtilTestMapRow::StaticStruct(), TEXT("Lookup"), TEXT("Lookup") },
		{ TEXT("A dynamic delegate"), FDirectiveUtilTestDelegateRow::StaticStruct(), TEXT("Callback"), TEXT("Callback") },
		{ TEXT("A nested object reference"), FDirectiveUtilTestNestedObjectReferenceRow::StaticStruct(), TEXT("Payload.ObjectReference"), TEXT("Payload") },
		{ TEXT("A nested soft object pointer"), FDirectiveUtilTestNestedSoftReferenceRow::StaticStruct(), TEXT("Payload.SoftReference"), TEXT("Payload") },
		{ TEXT("A nested set"), FDirectiveUtilTestNestedSetRow::StaticStruct(), TEXT("Payload.Members"), TEXT("Payload") },
		{ TEXT("A nested map"), FDirectiveUtilTestNestedMapRow::StaticStruct(), TEXT("Payload.Lookup"), TEXT("Payload") },
		{ TEXT("A nested dynamic delegate"), FDirectiveUtilTestNestedDelegateRow::StaticStruct(), TEXT("Payload.Callback"), TEXT("Payload") },
	};
	for (const FRejectedPropertyCase& RejectedCase : RejectedCases)
	{
		const FString RejectedCsv = FString::Printf(TEXT("Name,%s\nOne,x\n"), RejectedCase.TopLevelColumn);
		const bool bImportRejected = ImportWithHeader(RejectedCase.RowStruct, RejectedCsv, ErrorMessage) == nullptr;
		TestTrue(FString::Printf(TEXT("%s rejects import"), RejectedCase.Description), bImportRejected);
		TestTrue(FString::Printf(TEXT("%s import error names %s"), RejectedCase.Description, RejectedCase.PropertyPath),
			ErrorMessage.Contains(RejectedCase.PropertyPath, ESearchCase::IgnoreCase));

		UDataTable* RejectedExportTable = NewObject<UDataTable>();
		RejectedExportTable->RowStruct = RejectedCase.RowStruct;
		FString RejectedExport(TEXT("stale"));
		TestFalse(FString::Printf(TEXT("%s rejects export"), RejectedCase.Description),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				RejectedExportTable, EDirectiveUtilCsvDelimiter::Comma, true, RejectedExport, ExportError));
		TestTrue(FString::Printf(TEXT("%s export error names %s"), RejectedCase.Description, RejectedCase.PropertyPath),
			ExportError.Contains(RejectedCase.PropertyPath, ESearchCase::IgnoreCase));
		TestTrue(FString::Printf(TEXT("%s clears the CSV output on export failure"), RejectedCase.Description),
			RejectedExport.IsEmpty());
	}

	const FInvalidRowKeyCase InvalidRowKeys[] = {
		{ TEXT("a comma"), TEXT("Bad,Key") },
		{ TEXT("a double quote"), TEXT("Bad\"Key") },
		{ TEXT("an apostrophe"), TEXT("Bad'Key") },
		{ TEXT("a tab"), TEXT("Bad\tKey") },
		{ TEXT("a line feed"), TEXT("Bad\nKey") },
		{ TEXT("a carriage return"), TEXT("Bad\rKey") },
		{ TEXT("a carriage return and line feed"), TEXT("Bad\r\nKey") },
	};
	for (const FInvalidRowKeyCase& KeyCase : InvalidRowKeys)
	{
		const FString KeyCsv = FString(TEXT("Name,Quantity\n")) + QuoteCsvCell(KeyCase.Key) + TEXT(",1\n");
		const bool bKeyRejected = ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(), KeyCsv, ErrorMessage) == nullptr;
		TestTrue(FString::Printf(TEXT("A row key containing %s is rejected"), KeyCase.Description), bKeyRejected);
		TestTrue(FString::Printf(TEXT("The rejected row key containing %s is reported for its row"), KeyCase.Description),
			ErrorMessage.StartsWith(TEXT("Row 2 has the row name")));
	}
	TestNull(TEXT("Row keys that differ only in letter case are rejected"),
		ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(),
			TEXT("Name,Quantity\nSword,1\nSWORD,2\n"), ErrorMessage));
	TestTrue(TEXT("Row keys that differ only in letter case are reported as a repeated row name"),
		ErrorMessage.StartsWith(TEXT("Row 3 repeats the row name")));
	TestNull(TEXT("A key that matches an earlier key after trimming is rejected"),
		ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(),
			TEXT("Name,Quantity\nSword,1\n\"  Sword  \",2\n"), ErrorMessage));
	TestTrue(TEXT("A key that matches after trimming is reported as a repeated row name"),
		ErrorMessage.StartsWith(TEXT("Row 3 repeats the row name")));
	UDataTable* TrimmedKeyTable = ImportWithHeader(FDirectiveUtilTestInventoryRow::StaticStruct(),
		TEXT("Name,Quantity\n\"  Padded  \",1\n\"\tTabbed\t\",2\n"), ErrorMessage);
	TestNotNull(TEXT("Keys with surrounding whitespace import"), TrimmedKeyTable);
	if (TrimmedKeyTable != nullptr)
	{
		TestEqual(TEXT("Both whitespace-padded keys import as rows"), TrimmedKeyTable->GetRowMap().Num(), 2);
		const FDirectiveUtilTestInventoryRow* PaddedRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TrimmedKeyTable, TEXT("Padded"));
		const FDirectiveUtilTestInventoryRow* TabbedRow = FindRowByName<const FDirectiveUtilTestInventoryRow>(TrimmedKeyTable, TEXT("Tabbed"));
		TestNotNull(TEXT("Surrounding spaces are trimmed from a row key"), PaddedRow);
		TestNotNull(TEXT("Surrounding tabs are trimmed from a row key"), TabbedRow);
		if (PaddedRow != nullptr)
		{
			TestEqual(TEXT("The trimmed spaced key keeps its row values"), PaddedRow->Quantity, 1);
		}
		if (TabbedRow != nullptr)
		{
			TestEqual(TEXT("The trimmed tabbed key keeps its row values"), TabbedRow->Quantity, 2);
		}
	}

	UDataTable* NameValueTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
		FDirectiveUtilTestNameCollisionRow::StaticStruct(), TEXT("Alpha\nBeta\n"),
		EDirectiveUtilCsvDelimiter::Comma, false, ErrorMessage);
	TestNotNull(TEXT("A row struct with a Name property imports without a header row"), NameValueTable);
	if (NameValueTable != nullptr)
	{
		TestEqual(TEXT("Each headerless row of a Name property struct imports"), NameValueTable->GetRowMap().Num(), 2);
		const FDirectiveUtilTestNameCollisionRow* AlphaRow = FindRowByName<const FDirectiveUtilTestNameCollisionRow>(NameValueTable, TEXT("Row_1"));
		const FDirectiveUtilTestNameCollisionRow* BetaRow = FindRowByName<const FDirectiveUtilTestNameCollisionRow>(NameValueTable, TEXT("Row_2"));
		TestNotNull(TEXT("The first Name property row has a generated key"), AlphaRow);
		TestNotNull(TEXT("The second Name property row has a generated key"), BetaRow);
		if (AlphaRow != nullptr)
		{
			TestEqual(TEXT("The first cell binds to the Name property"), AlphaRow->Name, FString(TEXT("Alpha")));
		}
		if (BetaRow != nullptr)
		{
			TestEqual(TEXT("The second cell binds to the Name property"), BetaRow->Name, FString(TEXT("Beta")));
		}

		FString NameValueExport;
		TestTrue(TEXT("A row struct with a Name property exports without a header row"),
			UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
				NameValueTable, EDirectiveUtilCsvDelimiter::Comma, false, NameValueExport, ExportError));
		TestEqual(TEXT("The Name property values export in generated key order"), NameValueExport, FString(TEXT("Alpha\nBeta\n")));
	}
	TestNull(TEXT("A header row import of a row struct with a Name property is rejected"),
		ImportWithHeader(FDirectiveUtilTestNameCollisionRow::StaticStruct(), TEXT("Name\nAlpha\n"), ErrorMessage));
	TestTrue(TEXT("The Name property collision is explained"),
		ErrorMessage.Contains(TEXT("declares a property named Name"), ESearchCase::CaseSensitive));

	UDataTable* TableStateTable = ImportWithHeader(FDirectiveUtilTestTableStateRow::StaticStruct(),
		TEXT("Name,Value\nFirst,1\nSecond,2\n"), ErrorMessage);
	TestNotNull(TEXT("A table of callback-recording rows is created"), TableStateTable);
	if (TableStateTable != nullptr)
	{
		const TCHAR* const CreatedRowNames[] = { TEXT("First"), TEXT("Second") };
		for (const TCHAR* RowName : CreatedRowNames)
		{
			const FDirectiveUtilTestTableStateRow* CreatedRow = FindRowByName<const FDirectiveUtilTestTableStateRow>(TableStateTable, RowName);
			const bool bCreatedRowFound = CreatedRow != nullptr;
			TestTrue(FString::Printf(TEXT("Created row %s exists"), RowName), bCreatedRowFound);
			if (CreatedRow != nullptr)
			{
				TestEqual(FString::Printf(TEXT("Created row %s saw an empty table during post-import"), RowName),
					CreatedRow->TableRowCountDuringImport, 0);
				TestEqual(FString::Printf(TEXT("Created row %s saw the destination table during post-import"), RowName),
					CreatedRow->TableNameDuringImport, TableStateTable->GetFName());
			}
		}

		TestTrue(TEXT("Replacing a table with three rows succeeds"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				TableStateTable, TEXT("Name,Value\nThird,3\nFourth,4\nFifth,5\n"),
				EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		TestEqual(TEXT("The replacement installs three rows"), TableStateTable->GetRowMap().Num(), 3);
		const TCHAR* const ReplacedRowNames[] = { TEXT("Third"), TEXT("Fourth"), TEXT("Fifth") };
		for (const TCHAR* RowName : ReplacedRowNames)
		{
			const FDirectiveUtilTestTableStateRow* ReplacedRow = FindRowByName<const FDirectiveUtilTestTableStateRow>(TableStateTable, RowName);
			const bool bReplacedRowFound = ReplacedRow != nullptr;
			TestTrue(FString::Printf(TEXT("Replacement row %s exists"), RowName), bReplacedRowFound);
			if (ReplacedRow != nullptr)
			{
				TestEqual(FString::Printf(TEXT("Replacement row %s saw the two previous rows during post-import"), RowName),
					ReplacedRow->TableRowCountDuringImport, 2);
				TestEqual(FString::Printf(TEXT("Replacement row %s saw the destination table during post-import"), RowName),
					ReplacedRow->TableNameDuringImport, TableStateTable->GetFName());
			}
		}

		TestTrue(TEXT("Replacing a table with one row succeeds"),
			UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
				TableStateTable, TEXT("Name,Value\nSixth,6\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
		const FDirectiveUtilTestTableStateRow* SixthRow = FindRowByName<const FDirectiveUtilTestTableStateRow>(TableStateTable, TEXT("Sixth"));
		TestNotNull(TEXT("The second replacement row exists"), SixthRow);
		if (SixthRow != nullptr)
		{
			TestEqual(TEXT("The second replacement saw the three previous rows during post-import"),
				SixthRow->TableRowCountDuringImport, 3);
		}
		TestEqual(TEXT("The second replacement leaves one row"), TableStateTable->GetRowMap().Num(), 1);
	}

	const UFunction* DiffFunction = UDirectiveUtilDataTableFunctionLibrary::StaticClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilDataTableFunctionLibrary, DiffDataTables));
	TestTrue(TEXT("Diff Data Tables is an impure node"),
		DiffFunction != nullptr && !DiffFunction->HasAnyFunctionFlags(FUNC_BlueprintPure));

#if WITH_EDITOR
	UUserDefinedEnum* TierEnum = Cast<UUserDefinedEnum>(FEnumEditorUtils::CreateUserDefinedEnum(
		GetTransientPackage(),
		MakeUniqueObjectName(GetTransientPackage(), UUserDefinedEnum::StaticClass(), FName(TEXT("DirectiveUtilTestTier"))),
		RF_Public | RF_Transient));
	UUserDefinedStruct* UserRowStruct = FStructureEditorUtils::CreateUserDefinedStruct(
		GetTransientPackage(),
		MakeUniqueObjectName(GetTransientPackage(), UUserDefinedStruct::StaticClass(), FName(TEXT("DirectiveUtilTestUserRow"))),
		RF_Public | RF_Transient);
	TestNotNull(TEXT("A transient Blueprint enumeration is created"), TierEnum);
	TestNotNull(TEXT("A transient Blueprint Structure is created"), UserRowStruct);
	if (TierEnum != nullptr && UserRowStruct != nullptr)
	{
		FEnumEditorUtils::AddNewEnumeratorForUserDefinedEnum(TierEnum);
		FEnumEditorUtils::AddNewEnumeratorForUserDefinedEnum(TierEnum);
		FEnumEditorUtils::SetEnumeratorDisplayName(TierEnum, 0, FText::FromString(TEXT("Bronze")));
		FEnumEditorUtils::SetEnumeratorDisplayName(TierEnum, 1, FText::FromString(TEXT("Gold")));

		FStructureEditorUtils::AddVariable(UserRowStruct, FEdGraphPinType(
			UEdGraphSchema_K2::PC_Byte, NAME_None, TierEnum, EPinContainerType::None, false, FEdGraphTerminalType()));
		FStructureEditorUtils::AddVariable(UserRowStruct, FEdGraphPinType(
			UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
		const TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(UserRowStruct);
		TestEqual(TEXT("The Blueprint Structure has three members"), Variables.Num(), 3);
		if (Variables.Num() == 3)
		{
			const FGuid ReadyGuid = Variables[0].VarGuid;
			const FGuid TierGuid = Variables[1].VarGuid;
			const FGuid RatioGuid = Variables[2].VarGuid;
			FStructureEditorUtils::RenameVariable(UserRowStruct, ReadyGuid, TEXT("Ready"));
			FStructureEditorUtils::RenameVariable(UserRowStruct, TierGuid, TEXT("Tier"));
			FStructureEditorUtils::RenameVariable(UserRowStruct, RatioGuid, TEXT("Ratio"));

			UDataTable* UserTable = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
				UserRowStruct, TEXT("Name,Ready,Tier,Ratio\nFirst,true,Gold,0.30000000000000004\n"),
				EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
			TestNotNull(TEXT("A Blueprint Structure row imports by authored names"), UserTable);
			FString UserExport;
			if (UserTable != nullptr)
			{
				TestTrue(TEXT("A Blueprint Structure table exports"),
					UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
						UserTable, EDirectiveUtilCsvDelimiter::Comma, true, UserExport, ExportError));
				TestTrue(TEXT("Blueprint Structure headers use authored names"),
					UserExport.Contains(TEXT("Ready")) && UserExport.Contains(TEXT("Tier")) && UserExport.Contains(TEXT("Ratio"))
					&& !UserExport.Contains(TEXT("MemberVar")));
				TestTrue(TEXT("A Blueprint enumeration exports its display name"), UserExport.Contains(TEXT("Gold")));
				TestTrue(TEXT("A Blueprint Structure double exports every digit"), UserExport.Contains(TEXT("0.30000000000000004")));
			}
			UDataTable* UserReimport = UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
				UserRowStruct, UserExport, EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage);
			FString UserReexport;
			if (UserReimport != nullptr)
			{
				UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
					UserReimport, EDirectiveUtilCsvDelimiter::Comma, true, UserReexport, ExportError);
			}
			TestEqual(TEXT("A Blueprint Structure table round-trips through CSV"), UserReexport, UserExport);
			TestNull(TEXT("The generated MAX entry of a Blueprint enumeration is rejected"),
				UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
					UserRowStruct, TEXT("Name,Tier\nFirst,2\n"), EDirectiveUtilCsvDelimiter::Comma, true, ErrorMessage));
			TestNull(TEXT("An undeclared numeric value of a Blueprint enumeration is rejected"),
				ImportWithHeader(UserRowStruct, TEXT("Name,Tier\nFirst,5\n"), ErrorMessage));

			const FString TierAuthoredName = TierEnum->GetAuthoredNameStringByIndex(1);
			const FString TierPlainName = TierEnum->GetNameStringByIndex(1);
			const FString TierFullName = TierEnum->GetNameByIndex(1).ToString();
			TestEqual(TEXT("The authored name of a Blueprint enumeration entry is its display name"),
				TierAuthoredName, FString(TEXT("Gold")));
			TestNotEqual(TEXT("The plain name of a Blueprint enumeration entry differs from its authored name"),
				TierPlainName, TierAuthoredName);
			TestNotEqual(TEXT("The full name of a Blueprint enumeration entry differs from its plain name"),
				TierFullName, TierPlainName);
			const FString TierFormsCsv = FString::Printf(
				TEXT("Name,Tier\nAuthored,%s\nPlain,%s\nFull,%s\nValue,%lld\n"),
				*TierAuthoredName, *TierPlainName, *TierFullName, TierEnum->GetValueByIndex(1));
			UDataTable* TierFormsTable = ImportWithHeader(UserRowStruct, TierFormsCsv, ErrorMessage);
			TestNotNull(TEXT("A Blueprint enumeration imports from its authored, plain, and full names and its value"), TierFormsTable);
			FString TierFormsExport;
			if (TierFormsTable != nullptr)
			{
				TestTrue(TEXT("The Blueprint enumeration forms table exports"),
					UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
						TierFormsTable, EDirectiveUtilCsvDelimiter::Comma, true, TierFormsExport, ExportError));
				const TCHAR* const TierFormRowNames[] = { TEXT("Authored"), TEXT("Plain"), TEXT("Full"), TEXT("Value") };
				for (const TCHAR* RowName : TierFormRowNames)
				{
					FString TierCell;
					const bool bTierCellFound = FindExportedCell(TierFormsExport, RowName, TEXT("Tier"), TierCell);
					TestTrue(FString::Printf(TEXT("The %s form row exports a Tier cell"), RowName), bTierCellFound);
					TestEqual(FString::Printf(TEXT("The %s form imports as the Gold entry"), RowName),
						TierCell, FString(TEXT("Gold")));
				}
			}
		}
	}
#endif

	return true;
}
