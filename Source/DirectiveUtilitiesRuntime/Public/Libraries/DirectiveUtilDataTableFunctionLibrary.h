// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilCsvTypes.h"
#include "DirectiveUtilDataTableFunctionLibrary.generated.h"

class UDataTable;
class UScriptStruct;
struct FTableRowBase;

/**
 * Runtime DataTable utilities: building a UDataTable from CSV text at runtime and exporting
 * one back to CSV text. The engine's DataTable CSV Blueprint nodes are editor-only; its
 * runtime C++ entry point offers no delimiter choice and no headerless import.
 *
 * Row values are matched to the row struct's properties by column name, case-insensitive.
 * Blueprint Structure assets match on their authored field names rather than the mangled
 * reflected names. Enumeration cells import from an authored name, plain internal name,
 * EnumType::Name form, or declared numeric value, and export as the authored name.
 * The generated MAX entry of an enumeration is rejected on import and export.
 * Supported property types: Boolean, integer and floating-point numbers, enumerations,
 * FString, FName, FText, and value-only struct properties written in engine text format
 * such as `(X=1.0,Y=2.0,Z=3.0)`. Object references, arrays, sets, maps, and delegates are
 * rejected at any nesting depth. Fixed-size reflected arrays are also rejected, as are structs
 * that can hold object references outside reflected properties, such as FInstancedStruct.
 *
 * Floating-point values export with the fewest digits that read back to the same value,
 * including members of nested structs. FText values export as their display string, with no
 * NSLOCTEXT, LOCTEXT, or INVTEXT wrapper. On import, a text cell written in one of those macro
 * forms is read as that macro. Any other text, including engine forms such as LOCTABLE, is kept
 * as the literal string, so an import never loads a string table. A struct cell must name only members of the struct, and every member
 * value must be valid for its type. Structs with their own native text format, such as FGuid,
 * use the engine's text import and fail when the engine reports a problem.
 *
 * Row post-import callbacks run on every parsed row before any row is installed, so a
 * reported problem fails the call without changing the table. The rows are then installed
 * together, and row table-change callbacks and the table's OnDataTableChanged delegate run
 * once.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilDataTableFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Creates a transient UDataTable from CSV text using the supplied row struct.
	 *
	 * When `bHasHeaderRow` is `true`, row 0 names the columns and the header must contain a
	 * column named "Name" (case-insensitive). That column supplies each row's FName key instead
	 * of binding to a property. Leading and trailing whitespace around a key is trimmed. A key
	 * that contains a space, comma, quote, apostrophe, tab, or line break is rejected rather
	 * than renamed. A row struct that declares its own property named `Name` is rejected
	 * rather than having that column silently dropped. Columns that match no property are an error; properties missing
	 * from the header keep their struct defaults. Every data row must have the same number of
	 * cells as the header. When `bHasHeaderRow` is `false`, every cell of every row binds by
	 * column position to the struct's property order, extra cells are rejected, and missing
	 * trailing cells keep their defaults. Row keys are generated as `Row_1`, `Row_2`, and so on.
	 * Blank lines are skipped rather than treated as rows with an empty key. A row struct with
	 * no properties is accepted.
	 *
	 * @param RowStruct The reflected struct describing each table row. Native rows must derive from FTableRowBase; Blueprint Structure assets are also accepted.
	 * @param CsvText The CSV text to import.
	 * @param Delimiter The field separator used by the text.
	 * @param bHasHeaderRow Whether the first row names columns.
	 * @param OutErrorMessage Receives a description of the first problem found, or an empty string on success.
	 * @return The new transient table with zero or more rows, or null on failure. The caller owns the returned object like any other Blueprint object reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|DataTable")
	static UDataTable* CreateDataTableFromCsv(
		UScriptStruct* RowStruct,
		const FString& CsvText,
		const EDirectiveUtilCsvDelimiter Delimiter,
		const bool bHasHeaderRow,
		FString& OutErrorMessage);

	/**
	 * Exports a UDataTable to CSV text. The output starts with a `Name` column followed by
	 * one column per property of the row struct, in property declaration order, named as
	 * authored. Without a header row, each row contains only positional property values so
	 * the output can be imported without a header. Rows appear sorted alphabetically by row
	 * name. An unsupported property type fails the whole export, including on a table with no
	 * rows, rather than writing a header for a column that cannot be exported. With a header
	 * row, a row struct that declares its own property named `Name` fails because that column
	 * would collide with the row keys.
	 *
	 * @param Table The table to export. Must have a row struct set.
	 * @param Delimiter The field separator to write.
	 * @param bIncludeHeaderRow When `true`, the first output row holds the column names and each data row starts with its Name key.
	 * @param OutCsvText Receives the serialized text, or an empty string on failure.
	 * @param OutErrorMessage Receives a description of the first problem found, or an empty string on success.
	 * @return `true` when the table was exported.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|DataTable")
	static bool ExportDataTableToCsv(
		const UDataTable* Table,
		const EDirectiveUtilCsvDelimiter Delimiter,
		const bool bIncludeHeaderRow,
		FString& OutCsvText,
		FString& OutErrorMessage);

	/**
	 * Replaces every row of an existing table from CSV. Input is fully parsed and validated,
	 * and row post-import callbacks run on the parsed rows, before the table changes. Any
	 * problem fails the call without changing the table or broadcasting OnDataTableChanged.
	 * During those callbacks the table still holds its previous rows. Composite Data Tables are
	 * rejected because their rows are derived from parent tables.
	 *
	 * The table is changed in place. When it is a Data Table asset, the change applies to the
	 * loaded asset that every user of the table shares. In Play In Editor the change remains
	 * after the session ends and is written to disk if the asset is saved. Replace a transient
	 * copy from Create Data Table From CSV when the change must not reach the asset.
	 *
	 * @note A successful call frees every previous row, so row pointers returned by FindRow
	 * or GetRowMap before the call are no longer valid.
	 *
	 * @param Table The table to replace. It must have a row struct and must not be composite.
	 * @param CsvText The CSV text to import.
	 * @param Delimiter The field separator used by the text.
	 * @param bHasHeaderRow Whether the first row names columns.
	 * @param OutErrorMessage Receives the first validation or import problem, or an empty string on success.
	 * @return `true` when all replacement rows were installed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|DataTable")
	static bool ReplaceDataTableFromCsv(
		UDataTable* Table,
		const FString& CsvText,
		const EDirectiveUtilCsvDelimiter Delimiter,
		const bool bHasHeaderRow,
		FString& OutErrorMessage);

	/**
	 * Compares two tables that use the same row struct. Row names found in only one table are
	 * added or removed; rows with the same name are compared through the struct definition.
	 * All output arrays are sorted by row name.
	 *
	 * @param Before The earlier table.
	 * @param After The later table.
	 * @param OutAddedRows Receives row names found only in After.
	 * @param OutRemovedRows Receives row names found only in Before.
	 * @param OutChangedRows Receives shared row names whose values differ.
	 * @param OutErrorMessage Receives a validation problem, or an empty string on success.
	 * @return `true` when both tables were valid and comparable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|DataTable")
	static bool DiffDataTables(
		const UDataTable* Before,
		const UDataTable* After,
		TArray<FName>& OutAddedRows,
		TArray<FName>& OutRemovedRows,
		TArray<FName>& OutChangedRows,
		FString& OutErrorMessage);
};
