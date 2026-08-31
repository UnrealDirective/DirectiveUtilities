# Data Table Function Library

> Building a `UDataTable` from CSV text at runtime and exporting one back to CSV text. The engine's DataTable CSV Blueprint nodes are editor-only, and its runtime C++ entry point offers no delimiter choice and no headerless import.

Row values are matched to the row struct's properties by column name, case-insensitive. Blueprint Structure assets match on their authored field names rather than mangled reflected names. Blank lines are skipped rather than treated as rows with an empty key. Supported property types are Boolean, finite integer and floating-point numbers, enumerations, `FString`, `FName`, `FText`, and value-only struct properties written in engine text format such as `(X=1.0,Y=2.0,Z=3.0)`. Enumeration cells accept authored names, plain internal names, `EnumType::Name`, or a declared numeric value, and export the authored name. Object references, dynamic arrays, fixed-size reflected arrays, sets, maps, and delegates are rejected at any nesting depth with an error naming the property. Runtime imports do not load assets through reflected object properties.

When a header row is used, the first column named `Name` (case-insensitive) supplies each row's `FName` key instead of binding to a property. A row struct that declares its own property named `Name` is rejected rather than having that column silently dropped; rename the property or import without a header row. Columns that match no property are errors, properties missing from the header keep their struct defaults, and every data row must have exactly as many cells as the header. Without a header row, cells bind by position to the struct's property order. Extra cells are rejected, missing trailing cells keep their defaults, and row keys are generated as `Row_1`, `Row_2`, and so on.

Native row structs must derive from `FTableRowBase`. Blueprint Structure assets are also accepted. Validated imports are installed through Unreal's DataTable importer, so row post-import and table-change callbacks run.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilDataTableFunctionLibrary.h`

---

## Create Data Table From CSV
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|DataTable`

```cpp
static UDataTable* CreateDataTableFromCsv(
	UScriptStruct* RowStruct,
	const FString& CsvText,
	const EDirectiveUtilCsvDelimiter Delimiter,
	const bool bHasHeaderRow,
	FString& OutErrorMessage);
```

Creates a transient table with zero or more rows.

| Parameter | Type | Description |
|-----------|------|-------------|
| RowStruct | `UScriptStruct*` | A native `FTableRowBase` row struct or Blueprint Structure asset. |
| CsvText | `const FString&` | The CSV text to import. |
| Delimiter | `const EDirectiveUtilCsvDelimiter` | The field separator used by the text. |
| bHasHeaderRow | `const bool` | Whether the first row names columns. |
| OutErrorMessage | `FString&` | [out] A description of the first problem found, or an empty string on success. |

**Returns:** The new transient table, or null on failure. Duplicate or invalid row names, an empty `Name` cell, an unknown or repeated column binding, a row struct that declares its own `Name` property, a ragged headered row, extra positional cells, an unsupported property, or a malformed value rejects the whole import with a message naming the problem.

## Export Data Table To CSV
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|DataTable`

```cpp
static bool ExportDataTableToCsv(
	const UDataTable* Table,
	const EDirectiveUtilCsvDelimiter Delimiter,
	const bool bIncludeHeaderRow,
	FString& OutCsvText,
	FString& OutErrorMessage);
```

Exports a table to CSV text. With a header row, the output starts with a `Name` column followed by one column per property in declaration order, named as authored. Each data row starts with its row key. Without a header row, data rows contain only positional property values and can be imported with `bHasHeaderRow` false; generated keys replace the original keys on that path. Rows are sorted alphabetically by row name. Unsupported properties and non-finite numeric values fail the whole export, including for a table with no rows.

| Parameter | Type | Description |
|-----------|------|-------------|
| Table | `const UDataTable*` | The table to export. Must have a row struct set. |
| Delimiter | `const EDirectiveUtilCsvDelimiter` | The field separator to write. |
| bIncludeHeaderRow | `const bool` | When true, the first output row holds column names and each data row starts with its Name key. |
| OutCsvText | `FString&` | [out] The serialized text, or an empty string on failure. |
| OutErrorMessage | `FString&` | [out] A description of the first problem found, or an empty string on success. |

**Returns:** True when the table was exported.

## Replace Data Table From CSV

```cpp
static bool ReplaceDataTableFromCsv(UDataTable* Table, const FString& CsvText, const EDirectiveUtilCsvDelimiter Delimiter, const bool bHasHeaderRow, FString& OutErrorMessage);
```

Parses and validates the complete replacement before changing the destination. The validated rows are installed through Unreal's DataTable importer so import callbacks run on the target table. If the importer or a callback reports a problem, the original table is restored. A successful call replaces every row while retaining the destination object and row struct. Composite Data Tables are rejected because their rows are derived from parent tables.

## Diff Data Tables

```cpp
static bool DiffDataTables(const UDataTable* Before, const UDataTable* After, TArray<FName>& OutAddedRows, TArray<FName>& OutRemovedRows, TArray<FName>& OutChangedRows, FString& OutErrorMessage);
```

Returns sorted added, removed, and changed row names. Both tables must use the same valid row struct. Row comparison uses the reflected struct rather than raw padding bytes. The three output arrays must be different variables.
