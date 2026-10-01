# Data Table Function Library

> Building a `UDataTable` from CSV text at runtime and exporting one back to CSV text. The engine's DataTable CSV Blueprint nodes are editor-only, and its runtime C++ entry point offers no delimiter choice and no headerless import.

Row values are matched to the row struct's properties by column name, case-insensitive. Blueprint Structure assets match on their authored field names rather than mangled reflected names. Blank lines are skipped rather than treated as rows with an empty key. A leading byte-order mark (U+FEFF) is ignored. Supported property types are Boolean, finite integer and floating-point numbers, enumerations, `FString`, `FName`, `FText`, and value-only struct properties written in engine text format such as `(X=1.0,Y=2.0,Z=3.0)`. Object references, dynamic arrays, fixed-size reflected arrays, sets, maps, and delegates are rejected at any nesting depth with an error naming the property. Structs that can hold object references outside reflected properties, such as `FInstancedStruct`, are also rejected. Runtime imports do not load assets through reflected object properties.

Enumeration cells accept authored names, plain internal names, `EnumType::Name` for `enum class` and namespaced enumerations, or a declared numeric value, and export the authored name. The `MAX` entry that Unreal adds to the end of every enumeration is not a declared value. Import rejects it by name or number, and export fails when a property holds it.

Floating-point cells export with the fewest significant digits that read back to the same `float` or `double`, so values such as `0.0000001`, `1e-30`, and `0.30000000000000004` survive a round trip. Whole numbers below 10^9 for `float` and 10^17 for `double` are written without an exponent. The same formatting applies to floating-point members inside struct cells.

`FText` cells export the display string, with no `NSLOCTEXT`, `LOCTEXT`, or `INVTEXT` wrapper, so the output is the same in the editor and in a packaged game. On import, a text cell written in one of those macro forms is read as that macro; any other text becomes the cell's literal string. Other engine text forms, such as `LOCTABLE`, are kept as literal text, so an import never references or loads a string table.

A struct cell lists members as `Name=Value` pairs. The outer parentheses are optional. Members are matched by reflected or authored name, case-insensitive. Members left out keep their defaults. An unknown member, a repeated member, or a value that is not valid for the member's type rejects the import. Exported struct cells write numbers and Booleans bare and write strings, names, text, and enumeration names in double quotes with backslash escapes. Structs that define their own text format, such as `FGuid`, use that format for import and export, and the import fails when the struct's parser rejects the text.

When a header row is used, the header must contain a column named `Name` (case-insensitive). The first such column supplies each row's `FName` key instead of binding to a property. A header row without a `Name` column is rejected. Leading and trailing whitespace around a key is trimmed. A key that contains a space, comma, double quote, apostrophe, tab, or line break is rejected rather than renamed, so `Iron Sword` fails instead of becoming `IronSword`. Two keys that differ only in letter case are the same `FName` and count as a repeated row name. A row struct that declares its own property named `Name` is rejected rather than having that column silently dropped; rename the property or import without a header row. Columns that match no property are errors, properties missing from the header keep their struct defaults, and every data row must have exactly as many cells as the header. Without a header row, cells bind by position to the struct's property order. Extra cells are rejected, missing trailing cells keep their defaults, and row keys are generated as `Row_1`, `Row_2`, and so on.

CSV text holds at most 1,000,000 rows, including the header row, matching the [CSV library](CsvFunctionLibrary.md#parse-csv) limit. Native row structs must derive from `FTableRowBase`. Blueprint Structure assets are also accepted. A row struct with no properties is accepted; each row then carries only its key.

Imports parse every row first, then call each native row's `OnPostDataImport` on the parsed row. A problem reported by any callback fails the call without changing the destination table. During those callbacks the table passed to the callback is the destination, which still holds its previous rows (none for a new table). The rows are then installed in one step, after which each row's `OnDataTableChanged` runs and the table's `OnDataTableChanged` delegate broadcasts once.

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

**Returns:** The new transient table, or null on failure. A header row without a `Name` column, duplicate or invalid row names, an empty `Name` cell, an unknown or repeated column binding, a row struct that declares its own `Name` property, a ragged headered row, extra positional cells, an unsupported property, a malformed value, or a row post-import problem rejects the whole import with a message naming the problem.

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

Exports a table to CSV text. With a header row, the output starts with a `Name` column followed by one column per property in declaration order, named as authored. Each data row starts with its row key. Without a header row, data rows contain only positional property values and can be imported with `bHasHeaderRow` false; generated keys replace the original keys on that path. Rows are sorted alphabetically by row name. Unsupported properties, non-finite numeric values, and enumeration values outside the declared entries fail the whole export, including for a table with no rows. With a header row, a row struct that declares its own property named `Name` fails because that column would collide with the row keys; export it without a header row.

| Parameter | Type | Description |
|-----------|------|-------------|
| Table | `const UDataTable*` | The table to export. Must have a row struct set. |
| Delimiter | `const EDirectiveUtilCsvDelimiter` | The field separator to write. |
| bIncludeHeaderRow | `const bool` | When true, the first output row holds column names and each data row starts with its Name key. |
| OutCsvText | `FString&` | [out] The serialized text, or an empty string on failure. |
| OutErrorMessage | `FString&` | [out] A description of the first problem found, or an empty string on success. |

**Returns:** True when the table was exported.

## Replace Data Table From CSV
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|DataTable`

```cpp
static bool ReplaceDataTableFromCsv(UDataTable* Table, const FString& CsvText, const EDirectiveUtilCsvDelimiter Delimiter, const bool bHasHeaderRow, FString& OutErrorMessage);
```

Parses and validates the complete replacement and runs row post-import callbacks on the parsed rows before changing the destination. A parse error, validation error, or callback problem returns false without changing the table or broadcasting `OnDataTableChanged`. A successful call replaces every row while retaining the destination object and row struct, then broadcasts `OnDataTableChanged` once. Composite Data Tables are rejected because their rows are derived from parent tables.

The table changes in place. When `Table` is a Data Table asset, every user of that loaded asset sees the new rows. In Play In Editor the change outlives the session, and saving the asset writes it to disk. To keep an asset unchanged, create a transient table with Create Data Table From CSV and replace that one instead.

A successful call frees every previous row. Row pointers taken from `FindRow` or `GetRowMap` before the call are invalid afterward; look rows up again.

## Diff Data Tables
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|DataTable`

```cpp
static bool DiffDataTables(const UDataTable* Before, const UDataTable* After, TArray<FName>& OutAddedRows, TArray<FName>& OutRemovedRows, TArray<FName>& OutChangedRows, FString& OutErrorMessage);
```

Returns sorted added, removed, and changed row names. Both tables must use the same valid row struct. Row comparison uses the reflected struct rather than raw padding bytes. The three output arrays must be different variables. The node is impure, so it runs once per execution rather than once for each connected output pin.
