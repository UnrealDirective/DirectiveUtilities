# CSV Function Library

> Parsing text into rows and cells, serializing rows back to CSV text, and querying or editing a parsed document.

Parsing supports RFC-style quoted fields: quoted fields may contain separators and newlines, and quotes are escaped by doubling. For compatibility with common exporters, a quote inside an unquoted field remains literal text. A quoted field must end at a separator or row terminator, so trailing content after the closing quote fails. Blank lines are skipped rather than parsed as one empty cell. Writing quotes a field only when it contains the delimiter, a quote, or a newline. CR, LF, and CRLF are accepted as row terminators.

The library works on strings. Pair it with the [File System library](FileSystemFunctionLibrary.md) to read and write `.csv` files.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilCsvFunctionLibrary.h`

**Types:** `EDirectiveUtilCsvDelimiter` (Comma, Semicolon, Tab), `FDirectiveUtilCsvRow`, `FDirectiveUtilCsvDocument` in `Source/DirectiveUtilitiesRuntime/Public/Types/DirectiveUtilCsvTypes.h`. A document stores its rows plus the delimiter it was parsed with, so writing keeps the original separator.

---

## Parse CSV
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static bool ParseCsv(const FString& CsvText, const EDirectiveUtilCsvDelimiter Delimiter, FDirectiveUtilCsvDocument& OutDocument, FString& OutErrorMessage);
```

Parses CSV text into a document.

| Parameter | Type | Description |
|-----------|------|-------------|
| CsvText | `const FString&` | The text to parse. |
| Delimiter | `const EDirectiveUtilCsvDelimiter` | The field separator. |
| OutDocument | `FDirectiveUtilCsvDocument&` | [out] The parsed rows, or an empty document on failure. Stores the delimiter. |
| OutErrorMessage | `FString&` | [out] A description of the first problem found, or an empty string on success. |

**Returns:** True when the text was parsed. Empty input parses to an empty document.

## Write CSV
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static void WriteCsv(const FDirectiveUtilCsvDocument& Document, FString& OutCsvText);
```

Serializes a document using its stored delimiter. Rows are terminated with LF; every cell after the first is preceded by the delimiter, and no delimiter trails the last cell. A row with no cells is written as one quoted empty cell so reparsing keeps the row.

## Get CSV Delimiter Character
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static FString GetCsvDelimiterCharacter(const EDirectiveUtilCsvDelimiter Delimiter);
```

**Returns:** The separator character as a string (`,` / `;` / tab), or an empty string for an invalid enum value.

## Get CSV Row Count
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static int32 GetCsvRowCount(const FDirectiveUtilCsvDocument& Document);
```

**Returns:** The number of rows, including any header row.

## Get CSV Column Count
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static int32 GetCsvColumnCount(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex);
```

Rows may have different lengths when the input is ragged.

**Returns:** The cell count of that row, or -1 when the row index is out of range.

## Get CSV Cell
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static bool GetCsvCell(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, FString& OutCell);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| RowIndex | `const int32` | Zero-based row index. |
| ColumnIndex | `const int32` | Zero-based column index. |
| OutCell | `FString&` | [out] The cell text, or an empty string when out of range. |

**Returns:** True when both indices were in range. A missing cell within an existing row returns false.

## Set CSV Cell
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static bool SetCsvCell(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, const FString& Value);
```

Replaces one cell. A row grows by one empty cell when the column index is exactly the current length; indices past that are rejected rather than filling a gap.

**Returns:** True when the cell was set. False when either index is negative, the row index is out of range, or the column index is more than one past the end of the row.

## Add CSV Row
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static int32 AddCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const TArray<FString>& Cells);
```

Appends a row. An empty `Cells` array creates a row containing one empty cell.

**Returns:** The zero-based index of the new row.

## Remove CSV Row
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Csv`

```cpp
static bool RemoveCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex);
```

Removes one row; later rows shift down by one.

**Returns:** True when the row was removed.

## Header and keyed-row operations

```cpp
static bool FindCsvColumn(const FDirectiveUtilCsvDocument& Document, const FString& HeaderName, int32& OutColumnIndex, const bool bCaseSensitive = false);
static bool GetCsvCellByHeader(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const FString& HeaderName, FString& OutCell, const bool bCaseSensitive = false);
static bool SetCsvCellByHeader(FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const FString& HeaderName, const FString& Value, const bool bCaseSensitive = false);
static bool FindCsvRowByKey(const FDirectiveUtilCsvDocument& Document, const FString& KeyHeader, const FString& KeyValue, int32& OutRowIndex, const bool bCaseSensitive = false);
static bool UpsertCsvRowByKey(FDirectiveUtilCsvDocument& Document, const FString& KeyHeader, const FString& KeyValue, const TMap<FString, FString>& ValuesByHeader, int32& OutRowIndex, FString& OutErrorMessage, const bool bCaseSensitive = false);
static bool RemoveCsvRowByKey(FDirectiveUtilCsvDocument& Document, const FString& KeyHeader, const FString& KeyValue, const bool bCaseSensitive = false);
```

Row 0 supplies the headers. Header names and keys ignore case by default. A repeated matching header or key is ambiguous and fails instead of choosing one. Upsert updates values by header or appends a rectangular row when the key is missing.

## Validate CSV Headers / Validate CSV Shape

```cpp
static bool ValidateCsvHeaders(const FDirectiveUtilCsvDocument& Document, const TArray<FString>& RequiredHeaders, TArray<FString>& OutMissingHeaders, TArray<FString>& OutDuplicateHeaders, const bool bCaseSensitive = false);
static bool ValidateCsvShape(const FDirectiveUtilCsvDocument& Document, TArray<int32>& OutInvalidRowIndices);
```

Header validation reports missing required names and repeated names. Its two output arrays must be different variables. Shape validation reports each zero-based data-row index whose cell count differs from the header row.

## Diff CSV By Key

```cpp
static bool DiffCsvByKey(const FDirectiveUtilCsvDocument& Before, const FDirectiveUtilCsvDocument& After, const FString& KeyHeader, FDirectiveUtilCsvDiff& OutDiff, FString& OutErrorMessage, const bool bCaseSensitive = false);
```

Returns sorted added, removed, and changed keys. Both documents must have the same headers in the same order and unique, non-empty keys. Case-insensitive comparisons retain the key spelling from the source document in each result.
