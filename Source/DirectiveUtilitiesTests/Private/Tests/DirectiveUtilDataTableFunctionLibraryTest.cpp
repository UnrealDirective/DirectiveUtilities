// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilDataTableFunctionLibrary.h"
#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"
#include "Engine/CompositeDataTable.h"
#include "Misc/AutomationTest.h"
#include "Tests/DirectiveUtilTestDataTableRows.h"

#include <limits>

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
		TestTrue(TEXT("FText export keeps a structured text literal"), TextHistoryExport.Contains(TEXT("NSLOCTEXT")));
	}

	FString Exported;
	FString ExportError;
	TestTrue(TEXT("Export writes the table back to CSV"),
		UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(Table, EDirectiveUtilCsvDelimiter::Comma, true, Exported, ExportError));
	TestTrue(TEXT("Export starts with the header"),
		Exported.StartsWith(TEXT("Name,Quantity,Weight,bEnchanted,Label,Rarity,Description\n")));
	TestTrue(TEXT("Export keeps imported values"), Exported.Contains(TEXT("Sword,3,")));
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
		TestTrue(TEXT("Struct properties export in engine text format"), WaypointExport.Contains(TEXT("X=")));
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

	return true;
}
