// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/DirectiveUtilCsvTypes.h"
#include "DirectiveUtilCsvFunctionLibrary.generated.h"

/**
 * CSV utilities for Blueprints: parsing text into rows and cells, serializing rows back to
 * CSV text, and querying or editing a parsed document. Parsing supports RFC-style quoted
 * fields, embedded separators and newlines, and doubled quotes. For compatibility with common
 * exporters, a quote inside an unquoted field is kept as text. Writing quotes a field only when
 * it contains the delimiter, a quote, or a newline.
 *
 * The library works on strings. Pair it with the FileSystem library to read and write .csv files.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilCsvFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr int32 MaximumRowCount = 1000000;

	/**
	 * Parses CSV text into a document. Newlines inside quoted fields are supported; CR, LF,
	 * and CRLF are accepted as row terminators. A quote is only special at the start of a
	 * field; inside an unquoted field it is kept as literal content. A quoted field must end
	 * at a separator or a row terminator, so trailing content after the closing quote fails.
	 * Blank lines are skipped rather than parsed as a row holding one empty cell. A leading
	 * byte-order mark (U+FEFF) is ignored. Text with more than MaximumRowCount rows, counting
	 * the header row, fails.
	 *
	 * @param CsvText The text to parse.
	 * @param Delimiter The field separator.
	 * @param OutDocument Receives the parsed rows, or an empty document on failure. Stores the delimiter for later writing.
	 * @param OutErrorMessage Receives a description of the first problem found, or an empty string on success.
	 * @return `true` when the text was parsed. Empty input parses to an empty document.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool ParseCsv(const FString& CsvText, const EDirectiveUtilCsvDelimiter Delimiter, FDirectiveUtilCsvDocument& OutDocument, FString& OutErrorMessage);

	/**
	 * Serializes a document to CSV text using its stored delimiter. Cells within a row are
	 * separated by the delimiter, with no trailing delimiter; rows are terminated with LF.
	 * A row with no cells is written as one quoted empty cell. A cell is quoted when it holds the
	 * delimiter, a double quote, or a line break, or starts with U+FEFF.
	 *
	 * @param Document The document to serialize.
	 * @param OutCsvText Receives the serialized text.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static void WriteCsv(const FDirectiveUtilCsvDocument& Document, FString& OutCsvText);

	/**
	 * Returns the field separator character for a delimiter value.
	 *
	 * @param Delimiter The delimiter value.
	 * @return The separator character as a single-character string, or an empty string for an invalid value.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static FString GetCsvDelimiterCharacter(const EDirectiveUtilCsvDelimiter Delimiter);

	/**
	 * Returns the number of rows in a CSV document, including any header row.
	 *
	 * @param Document The document to query.
	 * @return The row count.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static int32 GetCsvRowCount(const FDirectiveUtilCsvDocument& Document);

	/**
	 * Returns the number of cells in one row of a CSV document.
	 *
	 * @param Document The document to query.
	 * @param RowIndex Zero-based row index.
	 * @return The cell count of that row, or -1 when the row index is out of range.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static int32 GetCsvColumnCount(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex);

	/**
	 * Reads one cell of a CSV document.
	 *
	 * @param Document The document to read.
	 * @param RowIndex Zero-based row index.
	 * @param ColumnIndex Zero-based column index.
	 * @param OutCell Receives the cell text, or an empty string when the indices are out of range.
	 * @return `true` when both indices were in range. A missing cell within an existing row returns `false`.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static bool GetCsvCell(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, FString& OutCell);

	/**
	 * Replaces one cell of a CSV document. A row grows by one empty cell when the column index
	 * is exactly the current row length; indices past that are rejected rather than filling a gap.
	 *
	 * @param Document The document to edit.
	 * @param RowIndex Zero-based row index.
	 * @param ColumnIndex Zero-based column index.
	 * @param Value The new cell text.
	 * @return `true` when the cell was set. Returns `false` when either index is negative, the row index is out of range, or the column index is more than one past the end of the row.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool SetCsvCell(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, const FString& Value);

	/**
	 * Appends a row to a CSV document.
	 *
	 * @param Document The document to edit.
	 * @param Cells The cells of the new row, in column order. An empty array creates one empty cell.
	 * @return The zero-based index of the new row.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static int32 AddCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const TArray<FString>& Cells);

	/**
	 * Removes one row from a CSV document. Later rows shift down by one.
	 *
	 * @param Document The document to edit.
	 * @param RowIndex Zero-based row index.
	 * @return `true` when the row was removed. Returns `false` when the index is out of range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool RemoveCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex);

	/**
	 * Finds a uniquely named column in the first row.
	 *
	 * @param Document The document whose first row contains headers.
	 * @param HeaderName The header to find.
	 * @param OutColumnIndex Receives the zero-based column index, or -1 on failure.
	 * @param bCaseSensitive Whether header matching distinguishes letter case.
	 * @return `true` when exactly one matching header exists.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static bool FindCsvColumn(const FDirectiveUtilCsvDocument& Document, const FString& HeaderName,
		int32& OutColumnIndex, const bool bCaseSensitive = false);

	/**
	 * Reads a cell by the header in the first row.
	 *
	 * @param Document The document to read.
	 * @param RowIndex Zero-based row index, including the header row.
	 * @param HeaderName The unique header naming the column.
	 * @param OutCell Receives the cell text, or an empty string on failure.
	 * @param bCaseSensitive Whether header matching distinguishes letter case.
	 * @return `true` when the header and cell both exist.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Csv")
	static bool GetCsvCellByHeader(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex,
		const FString& HeaderName, FString& OutCell, const bool bCaseSensitive = false);

	/**
	 * Writes a cell by the header in the first row.
	 *
	 * @param Document The document to edit.
	 * @param RowIndex Zero-based row index, including the header row.
	 * @param HeaderName The unique header naming the column.
	 * @param Value The new cell text.
	 * @param bCaseSensitive Whether header matching distinguishes letter case.
	 * @return `true` when the header and row exist and the cell was set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool SetCsvCellByHeader(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex,
		const FString& HeaderName, const FString& Value, const bool bCaseSensitive = false);

	/**
	 * Finds a unique data row by a named key column. The first row is treated as headers.
	 *
	 * @param Document The document to search.
	 * @param KeyHeader The unique header naming the key column.
	 * @param KeyValue The non-empty key to find.
	 * @param OutRowIndex Receives the zero-based row index, or -1 on failure.
	 * @param bCaseSensitive Whether header and key matching distinguish letter case. When `true`, keys such as `Apple` and `apple` are different rows.
	 * @return `true` when exactly one data row has the key.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool FindCsvRowByKey(const FDirectiveUtilCsvDocument& Document, const FString& KeyHeader,
		const FString& KeyValue, int32& OutRowIndex, const bool bCaseSensitive = false);

	/**
	 * Updates a unique keyed row or appends one when the key is absent. All existing keys and
	 * input headers are validated before the document changes. A value supplied for the key
	 * column is ignored in favor of KeyValue.
	 *
	 * @param Document The document to edit. The first row is treated as headers.
	 * @param KeyHeader The unique header naming the key column.
	 * @param KeyValue The non-empty key to update or add.
	 * @param ValuesByHeader Values for uniquely resolved columns. Map keys ignore letter case, so one call cannot set headers that differ only in case.
	 * @param OutRowIndex Receives the updated or appended row index, or -1 on failure.
	 * @param OutErrorMessage Receives the validation error, or an empty string on success.
	 * @param bCaseSensitive Whether header and key matching distinguish letter case. When `false`, keys that differ only in case count as repeated keys and fail validation.
	 * @return `true` when the row was updated or added.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool UpsertCsvRowByKey(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const FString& KeyHeader,
		const FString& KeyValue, const TMap<FString, FString>& ValuesByHeader, int32& OutRowIndex,
		FString& OutErrorMessage, const bool bCaseSensitive = false);

	/**
	 * Removes a unique data row by a named key column.
	 *
	 * @param Document The document to edit. The first row is treated as headers.
	 * @param KeyHeader The unique header naming the key column.
	 * @param KeyValue The non-empty key to remove.
	 * @param bCaseSensitive Whether header and key matching distinguish letter case.
	 * @return `true` when exactly one matching row was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool RemoveCsvRowByKey(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const FString& KeyHeader,
		const FString& KeyValue, const bool bCaseSensitive = false);

	/**
	 * Checks the header row for required and repeated names.
	 *
	 * @param Document The document to validate.
	 * @param RequiredHeaders Headers that must appear at least once.
	 * @param OutMissingHeaders Receives required headers that were not found.
	 * @param OutDuplicateHeaders Receives names that appear more than once.
	 * @param bCaseSensitive Whether header matching distinguishes letter case.
	 * @return `true` when no required header is missing and no header is repeated. The two output arrays must be different variables.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool ValidateCsvHeaders(const FDirectiveUtilCsvDocument& Document,
		const TArray<FString>& RequiredHeaders, TArray<FString>& OutMissingHeaders,
		TArray<FString>& OutDuplicateHeaders, const bool bCaseSensitive = false);

	/**
	 * Checks that every data row has the same number of cells as the first row.
	 *
	 * @param Document The document to validate.
	 * @param OutInvalidRowIndices Receives zero-based indices of ragged data rows.
	 * @return `true` for an empty document or a rectangular document.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool ValidateCsvShape(const FDirectiveUtilCsvDocument& Document, TArray<int32>& OutInvalidRowIndices);

	/**
	 * Compares two documents by a unique key column. Header order must match. Key changes are
	 * reported as a removal and an addition; non-key cell changes are reported as changed keys.
	 * Cell values always compare with letter case, so `red` to `Red` is a change.
	 *
	 * @param Before The earlier document.
	 * @param After The later document.
	 * @param KeyHeader The unique header naming the key column in both documents.
	 * @param OutDiff Receives added, removed, and changed keys, or empty arrays on failure. Each array is sorted ignoring case, with keys that differ only in case ordered by character code.
	 * @param OutErrorMessage Receives the validation error, or an empty string on success.
	 * @param bCaseSensitive Whether header and key matching distinguish letter case.
	 * @return `true` when both documents were valid and comparable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Csv")
	static bool DiffCsvByKey(const FDirectiveUtilCsvDocument& Before, const FDirectiveUtilCsvDocument& After,
		const FString& KeyHeader, FDirectiveUtilCsvDiff& OutDiff, FString& OutErrorMessage,
		const bool bCaseSensitive = false);
};
