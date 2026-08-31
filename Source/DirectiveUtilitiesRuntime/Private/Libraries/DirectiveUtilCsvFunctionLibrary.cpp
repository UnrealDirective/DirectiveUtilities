// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"

namespace
{
	bool TryGetDelimiterCharacter(const EDirectiveUtilCsvDelimiter Delimiter, TCHAR& OutCharacter)
	{
		switch (Delimiter)
		{
			case EDirectiveUtilCsvDelimiter::Comma:
				OutCharacter = TEXT(',');
				return true;
			case EDirectiveUtilCsvDelimiter::Semicolon:
				OutCharacter = TEXT(';');
				return true;
			case EDirectiveUtilCsvDelimiter::Tab:
				OutCharacter = TEXT('\t');
				return true;
			default:
				OutCharacter = TEXT('\0');
				return false;
		}
	}

	bool CellNeedsQuotes(const FString& Cell, const TCHAR Separator)
	{
		return Cell.Contains(TEXT("\"")) || Cell.Contains(TEXT("\r")) || Cell.Contains(TEXT("\n"))
			|| Cell.Contains(FString::Chr(Separator));
	}

	void AppendQuotedCell(FString& OutText, const FString& Cell)
	{
		OutText += TEXT('"');
		FString Escaped = Cell.Replace(TEXT("\""), TEXT("\"\""));
		OutText += Escaped;
		OutText += TEXT('"');
	}

	ESearchCase::Type SearchCase(const bool bCaseSensitive)
	{
		return bCaseSensitive ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	}

	bool FindUniqueHeader(const FDirectiveUtilCsvDocument& Document, const FString& HeaderName,
		const bool bCaseSensitive, int32& OutColumnIndex)
	{
		OutColumnIndex = INDEX_NONE;
		if (HeaderName.IsEmpty() || Document.Rows.IsEmpty())
		{
			return false;
		}

		for (int32 ColumnIndex = 0; ColumnIndex < Document.Rows[0].Cells.Num(); ++ColumnIndex)
		{
			if (Document.Rows[0].Cells[ColumnIndex].Equals(HeaderName, SearchCase(bCaseSensitive)))
			{
				if (OutColumnIndex != INDEX_NONE)
				{
					OutColumnIndex = INDEX_NONE;
					return false;
				}
				OutColumnIndex = ColumnIndex;
			}
		}
		return OutColumnIndex != INDEX_NONE;
	}

	bool BuildKeyedRows(const FDirectiveUtilCsvDocument& Document, const FString& KeyHeader,
		const bool bCaseSensitive, TMap<FString, TPair<FString, const FDirectiveUtilCsvRow*>>& OutRows,
		FString& OutErrorMessage)
	{
		OutRows.Reset();
		int32 KeyColumn = INDEX_NONE;
		if (!FindUniqueHeader(Document, KeyHeader, bCaseSensitive, KeyColumn))
		{
			OutErrorMessage = FString::Printf(TEXT("Header '%s' is missing or repeated."), *KeyHeader);
			return false;
		}

		for (int32 RowIndex = 1; RowIndex < Document.Rows.Num(); ++RowIndex)
		{
			const FDirectiveUtilCsvRow& Row = Document.Rows[RowIndex];
			if (!Row.Cells.IsValidIndex(KeyColumn))
			{
				OutErrorMessage = FString::Printf(TEXT("Row %d has no value for key column '%s'."), RowIndex + 1, *KeyHeader);
				return false;
			}

			const FString Key = bCaseSensitive ? Row.Cells[KeyColumn] : Row.Cells[KeyColumn].ToLower();
			if (Key.IsEmpty())
			{
				OutErrorMessage = FString::Printf(TEXT("Row %d has an empty value for key column '%s'."), RowIndex + 1, *KeyHeader);
				return false;
			}
			if (OutRows.Contains(Key))
			{
				OutErrorMessage = FString::Printf(TEXT("Key '%s' appears more than once."), *Row.Cells[KeyColumn]);
				return false;
			}
			OutRows.Add(Key, TPair<FString, const FDirectiveUtilCsvRow*>(Row.Cells[KeyColumn], &Row));
		}
		return true;
	}
}

FString UDirectiveUtilCsvFunctionLibrary::GetCsvDelimiterCharacter(const EDirectiveUtilCsvDelimiter Delimiter)
{
	TCHAR Character = TEXT('\0');
	return TryGetDelimiterCharacter(Delimiter, Character) ? FString::Chr(Character) : FString();
}

bool UDirectiveUtilCsvFunctionLibrary::ParseCsv(const FString& CsvText, const EDirectiveUtilCsvDelimiter Delimiter, FDirectiveUtilCsvDocument& OutDocument, FString& OutErrorMessage)
{
	const FString SourceText = CsvText;
	OutDocument = FDirectiveUtilCsvDocument();
	OutDocument.Delimiter = Delimiter;
	OutErrorMessage.Reset();

	FDirectiveUtilCsvDocument Parsed;
	Parsed.Delimiter = Delimiter;

	TCHAR Separator = TEXT('\0');
	if (!TryGetDelimiterCharacter(Delimiter, Separator))
	{
		OutErrorMessage = TEXT("The delimiter value is invalid.");
		return false;
	}

	enum class ECsvFieldState : uint8
	{
		FieldStart,
		Unquoted,
		Quoted,
		QuoteInQuoted
	};

	ECsvFieldState State = ECsvFieldState::FieldStart;
	bool bLastActionEndedRow = false;
	FDirectiveUtilCsvRow CurrentRow;
	FString CurrentCell;

	const int32 Length = SourceText.Len();
	int32 Index = 0;

	auto EndField = [&]()
	{
		CurrentRow.Cells.Add(MoveTemp(CurrentCell));
		CurrentCell.Reset();
		State = ECsvFieldState::FieldStart;
		bLastActionEndedRow = false;
	};
	auto EndRow = [&]()
	{
		EndField();
		Parsed.Rows.Add(MoveTemp(CurrentRow));
		CurrentRow = FDirectiveUtilCsvRow();
		bLastActionEndedRow = true;
	};
	auto IsAtBlankLine = [&]()
	{
		return State == ECsvFieldState::FieldStart && CurrentRow.Cells.Num() == 0 && CurrentCell.IsEmpty();
	};

	while (Index < Length)
	{
		const TCHAR Character = SourceText[Index];

		switch (State)
		{
			case ECsvFieldState::FieldStart:
			{
				if (Character == TEXT('"'))
				{
					State = ECsvFieldState::Quoted;
					bLastActionEndedRow = false;
					++Index;
				}
				else if (Character == Separator)
				{
					EndField();
					++Index;
				}
				else if (Character == TEXT('\r') || Character == TEXT('\n'))
				{
					if (!IsAtBlankLine())
					{
						EndRow();
					}
					else
					{
						bLastActionEndedRow = true;
					}
						if (Character == TEXT('\r') && Index + 1 < Length && SourceText[Index + 1] == TEXT('\n'))
					{
						++Index;
					}
					++Index;
				}
				else
				{
					CurrentCell += Character;
					State = ECsvFieldState::Unquoted;
					bLastActionEndedRow = false;
					++Index;
				}
				break;
			}
			case ECsvFieldState::Unquoted:
			{
				if (Character == Separator)
				{
					EndField();
				}
				else if (Character == TEXT('\r') || Character == TEXT('\n'))
				{
					EndRow();
					if (Character == TEXT('\r') && Index + 1 < Length && SourceText[Index + 1] == TEXT('\n'))
					{
						++Index;
					}
				}
				else
				{
					CurrentCell += Character;
					bLastActionEndedRow = false;
				}
				++Index;
				break;
			}
			case ECsvFieldState::Quoted:
			{
				if (Character == TEXT('"'))
				{
					State = ECsvFieldState::QuoteInQuoted;
				}
				else
				{
					CurrentCell += Character;
					bLastActionEndedRow = false;
				}
				++Index;
				break;
			}
			case ECsvFieldState::QuoteInQuoted:
			{
				if (Character == TEXT('"'))
				{
					CurrentCell += TEXT('"');
					State = ECsvFieldState::Quoted;
				}
				else if (Character == Separator)
				{
					EndField();
				}
				else if (Character == TEXT('\r') || Character == TEXT('\n'))
				{
					EndRow();
					if (Character == TEXT('\r') && Index + 1 < Length && SourceText[Index + 1] == TEXT('\n'))
					{
						++Index;
					}
				}
				else
				{
					OutErrorMessage = FString::Printf(TEXT("Unexpected character after closing quote at index %d."), Index);
					return false;
				}
				++Index;
				break;
			}
			default:
			{
				checkNoEntry();
			}
		}
	}

	if (State == ECsvFieldState::Quoted)
	{
		OutErrorMessage = TEXT("Reached the end of the input inside a quoted field.");
		return false;
	}

	if (Length > 0 && !bLastActionEndedRow)
	{
		EndRow();
	}

	OutDocument = MoveTemp(Parsed);
	return true;
}

void UDirectiveUtilCsvFunctionLibrary::WriteCsv(const FDirectiveUtilCsvDocument& Document, FString& OutCsvText)
{
	TCHAR Separator = TEXT('\0');
	if (!TryGetDelimiterCharacter(Document.Delimiter, Separator))
	{
		OutCsvText.Reset();
		return;
	}

	FString CsvText;
	for (const FDirectiveUtilCsvRow& Row : Document.Rows)
	{
		if (Row.Cells.IsEmpty() || (Row.Cells.Num() == 1 && Row.Cells[0].IsEmpty()))
		{
			AppendQuotedCell(CsvText, FString());
			CsvText += TEXT('\n');
			continue;
		}
		for (int32 CellIndex = 0; CellIndex < Row.Cells.Num(); ++CellIndex)
		{
			if (CellIndex > 0)
			{
				CsvText += Separator;
			}
			const FString& Cell = Row.Cells[CellIndex];
			if (CellNeedsQuotes(Cell, Separator))
			{
				AppendQuotedCell(CsvText, Cell);
			}
			else
			{
				CsvText += Cell;
			}
		}
		CsvText += TEXT('\n');
	}
	OutCsvText = MoveTemp(CsvText);
}

int32 UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(const FDirectiveUtilCsvDocument& Document)
{
	return Document.Rows.Num();
}

int32 UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex)
{
	if (!Document.Rows.IsValidIndex(RowIndex))
	{
		return -1;
	}
	return Document.Rows[RowIndex].Cells.Num();
}

bool UDirectiveUtilCsvFunctionLibrary::GetCsvCell(const FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, FString& OutCell)
{
	if (!Document.Rows.IsValidIndex(RowIndex) || !Document.Rows[RowIndex].Cells.IsValidIndex(ColumnIndex))
	{
		OutCell.Reset();
		return false;
	}
	const FString Cell = Document.Rows[RowIndex].Cells[ColumnIndex];
	OutCell = Cell;
	return true;
}

bool UDirectiveUtilCsvFunctionLibrary::SetCsvCell(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex, const int32 ColumnIndex, const FString& Value)
{
	if (RowIndex < 0 || ColumnIndex < 0 || !Document.Rows.IsValidIndex(RowIndex))
	{
		return false;
	}

	TArray<FString>& Cells = Document.Rows[RowIndex].Cells;
	const FString ValueCopy = Value;
	if (!Cells.IsValidIndex(ColumnIndex))
	{
		if (ColumnIndex > Cells.Num())
		{
			return false;
		}
		Cells.SetNum(ColumnIndex + 1);
	}
	Cells[ColumnIndex] = ValueCopy;
	return true;
}

int32 UDirectiveUtilCsvFunctionLibrary::AddCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const TArray<FString>& Cells)
{
	FDirectiveUtilCsvRow Row;
	if (Cells.IsEmpty())
	{
		Row.Cells.AddDefaulted();
	}
	else
	{
		Row.Cells = Cells;
	}
	Document.Rows.Add(MoveTemp(Row));
	return Document.Rows.Num() - 1;
}

bool UDirectiveUtilCsvFunctionLibrary::RemoveCsvRow(UPARAM(ref) FDirectiveUtilCsvDocument& Document, const int32 RowIndex)
{
	if (!Document.Rows.IsValidIndex(RowIndex))
	{
		return false;
	}
	Document.Rows.RemoveAt(RowIndex);
	return true;
}

bool UDirectiveUtilCsvFunctionLibrary::FindCsvColumn(const FDirectiveUtilCsvDocument& Document,
	const FString& HeaderName, int32& OutColumnIndex, const bool bCaseSensitive)
{
	return FindUniqueHeader(Document, HeaderName, bCaseSensitive, OutColumnIndex);
}

bool UDirectiveUtilCsvFunctionLibrary::GetCsvCellByHeader(const FDirectiveUtilCsvDocument& Document,
	const int32 RowIndex, const FString& HeaderName, FString& OutCell, const bool bCaseSensitive)
{
	int32 ColumnIndex = INDEX_NONE;
	if (!FindUniqueHeader(Document, HeaderName, bCaseSensitive, ColumnIndex))
	{
		OutCell.Reset();
		return false;
	}
	return GetCsvCell(Document, RowIndex, ColumnIndex, OutCell);
}

bool UDirectiveUtilCsvFunctionLibrary::SetCsvCellByHeader(FDirectiveUtilCsvDocument& Document,
	const int32 RowIndex, const FString& HeaderName, const FString& Value, const bool bCaseSensitive)
{
	int32 ColumnIndex = INDEX_NONE;
	return FindUniqueHeader(Document, HeaderName, bCaseSensitive, ColumnIndex)
		&& SetCsvCell(Document, RowIndex, ColumnIndex, Value);
}

bool UDirectiveUtilCsvFunctionLibrary::FindCsvRowByKey(const FDirectiveUtilCsvDocument& Document,
	const FString& KeyHeader, const FString& KeyValue, int32& OutRowIndex, const bool bCaseSensitive)
{
	OutRowIndex = INDEX_NONE;
	int32 KeyColumn = INDEX_NONE;
	if (KeyValue.IsEmpty() || !FindUniqueHeader(Document, KeyHeader, bCaseSensitive, KeyColumn))
	{
		return false;
	}

	for (int32 RowIndex = 1; RowIndex < Document.Rows.Num(); ++RowIndex)
	{
		const TArray<FString>& Cells = Document.Rows[RowIndex].Cells;
		if (Cells.IsValidIndex(KeyColumn) && Cells[KeyColumn].Equals(KeyValue, SearchCase(bCaseSensitive)))
		{
			if (OutRowIndex != INDEX_NONE)
			{
				OutRowIndex = INDEX_NONE;
				return false;
			}
			OutRowIndex = RowIndex;
		}
	}
	return OutRowIndex != INDEX_NONE;
}

bool UDirectiveUtilCsvFunctionLibrary::UpsertCsvRowByKey(FDirectiveUtilCsvDocument& Document,
	const FString& KeyHeader, const FString& KeyValue, const TMap<FString, FString>& ValuesByHeader,
	int32& OutRowIndex, FString& OutErrorMessage, const bool bCaseSensitive)
{
	const FString KeyHeaderCopy = KeyHeader;
	const FString KeyValueCopy = KeyValue;
	OutRowIndex = INDEX_NONE;
	OutErrorMessage.Reset();
	if (KeyValueCopy.IsEmpty())
	{
		OutErrorMessage = TEXT("The key value is empty.");
		return false;
	}

	int32 KeyColumn = INDEX_NONE;
	if (!FindUniqueHeader(Document, KeyHeaderCopy, bCaseSensitive, KeyColumn))
	{
		OutErrorMessage = FString::Printf(TEXT("Header '%s' is missing or repeated."), *KeyHeaderCopy);
		return false;
	}

	TMap<int32, FString> ValuesByColumn;
	TSet<int32> ResolvedColumns;
	for (const TPair<FString, FString>& Value : ValuesByHeader)
	{
		int32 ColumnIndex = INDEX_NONE;
		if (!FindUniqueHeader(Document, Value.Key, bCaseSensitive, ColumnIndex))
		{
			OutErrorMessage = FString::Printf(TEXT("Header '%s' is missing or repeated."), *Value.Key);
			return false;
		}
		if (ResolvedColumns.Contains(ColumnIndex))
		{
			OutErrorMessage = FString::Printf(TEXT("More than one input header resolves to column %d."), ColumnIndex);
			return false;
		}
		ResolvedColumns.Add(ColumnIndex);
		if (ColumnIndex != KeyColumn)
		{
			ValuesByColumn.Add(ColumnIndex, Value.Value);
		}
	}

	TMap<FString, TPair<FString, const FDirectiveUtilCsvRow*>> ExistingRows;
	if (!BuildKeyedRows(Document, KeyHeaderCopy, bCaseSensitive, ExistingRows, OutErrorMessage))
	{
		return false;
	}

	for (int32 RowIndex = 1; RowIndex < Document.Rows.Num(); ++RowIndex)
	{
		const TArray<FString>& Cells = Document.Rows[RowIndex].Cells;
		if (Cells.IsValidIndex(KeyColumn) && Cells[KeyColumn].Equals(KeyValueCopy, SearchCase(bCaseSensitive)))
		{
			OutRowIndex = RowIndex;
			break;
		}
	}

	if (OutRowIndex == INDEX_NONE)
	{
		FDirectiveUtilCsvRow Row;
		Row.Cells.SetNum(Document.Rows[0].Cells.Num());
		Row.Cells[KeyColumn] = KeyValueCopy;
		OutRowIndex = Document.Rows.Add(MoveTemp(Row));
	}

	TArray<FString>& Cells = Document.Rows[OutRowIndex].Cells;
	if (Cells.Num() < Document.Rows[0].Cells.Num())
	{
		Cells.SetNum(Document.Rows[0].Cells.Num());
	}
	for (const TPair<int32, FString>& Value : ValuesByColumn)
	{
		Cells[Value.Key] = Value.Value;
	}
	Cells[KeyColumn] = KeyValueCopy;
	return true;
}

bool UDirectiveUtilCsvFunctionLibrary::RemoveCsvRowByKey(FDirectiveUtilCsvDocument& Document,
	const FString& KeyHeader, const FString& KeyValue, const bool bCaseSensitive)
{
	int32 RowIndex = INDEX_NONE;
	return FindCsvRowByKey(Document, KeyHeader, KeyValue, RowIndex, bCaseSensitive)
		&& RemoveCsvRow(Document, RowIndex);
}

bool UDirectiveUtilCsvFunctionLibrary::ValidateCsvHeaders(const FDirectiveUtilCsvDocument& Document,
	const TArray<FString>& RequiredHeaders, TArray<FString>& OutMissingHeaders,
	TArray<FString>& OutDuplicateHeaders, const bool bCaseSensitive)
{
	if (&OutMissingHeaders == &OutDuplicateHeaders)
	{
		OutMissingHeaders.Reset();
		return false;
	}

	const TArray<FString> RequiredHeadersCopy = RequiredHeaders;
	TArray<FString> MissingHeaders;
	TArray<FString> DuplicateHeaders;
	if (Document.Rows.IsEmpty())
	{
		OutMissingHeaders = RequiredHeadersCopy;
		OutDuplicateHeaders.Reset();
		return RequiredHeadersCopy.IsEmpty();
	}

	const TArray<FString>& Headers = Document.Rows[0].Cells;
	for (int32 HeaderIndex = 0; HeaderIndex < Headers.Num(); ++HeaderIndex)
	{
		for (int32 OtherIndex = HeaderIndex + 1; OtherIndex < Headers.Num(); ++OtherIndex)
		{
			if (Headers[HeaderIndex].Equals(Headers[OtherIndex], SearchCase(bCaseSensitive))
				&& !DuplicateHeaders.ContainsByPredicate([&](const FString& Existing)
				{
					return Existing.Equals(Headers[HeaderIndex], SearchCase(bCaseSensitive));
				}))
			{
				DuplicateHeaders.Add(Headers[HeaderIndex]);
			}
		}
	}

	for (const FString& RequiredHeader : RequiredHeadersCopy)
	{
		if (!Headers.ContainsByPredicate([&](const FString& Header)
		{
			return Header.Equals(RequiredHeader, SearchCase(bCaseSensitive));
		}))
		{
			MissingHeaders.Add(RequiredHeader);
		}
	}
	const bool bValid = MissingHeaders.IsEmpty() && DuplicateHeaders.IsEmpty();
	OutMissingHeaders = MoveTemp(MissingHeaders);
	OutDuplicateHeaders = MoveTemp(DuplicateHeaders);
	return bValid;
}

bool UDirectiveUtilCsvFunctionLibrary::ValidateCsvShape(const FDirectiveUtilCsvDocument& Document,
	TArray<int32>& OutInvalidRowIndices)
{
	OutInvalidRowIndices.Reset();
	if (Document.Rows.IsEmpty())
	{
		return true;
	}

	const int32 ExpectedColumns = Document.Rows[0].Cells.Num();
	for (int32 RowIndex = 1; RowIndex < Document.Rows.Num(); ++RowIndex)
	{
		if (Document.Rows[RowIndex].Cells.Num() != ExpectedColumns)
		{
			OutInvalidRowIndices.Add(RowIndex);
		}
	}
	return OutInvalidRowIndices.IsEmpty();
}

bool UDirectiveUtilCsvFunctionLibrary::DiffCsvByKey(const FDirectiveUtilCsvDocument& Before,
	const FDirectiveUtilCsvDocument& After, const FString& KeyHeader, FDirectiveUtilCsvDiff& OutDiff,
	FString& OutErrorMessage, const bool bCaseSensitive)
{
	const FString KeyHeaderCopy = KeyHeader;
	FDirectiveUtilCsvDiff Diff;
	FString ErrorMessage;
	if (Before.Rows.IsEmpty() || After.Rows.IsEmpty())
	{
		OutDiff = FDirectiveUtilCsvDiff();
		OutErrorMessage = TEXT("Both documents must contain a header row.");
		return false;
	}
	if (Before.Rows[0].Cells.Num() != After.Rows[0].Cells.Num())
	{
		OutDiff = FDirectiveUtilCsvDiff();
		OutErrorMessage = TEXT("The documents have different headers.");
		return false;
	}
	for (int32 ColumnIndex = 0; ColumnIndex < Before.Rows[0].Cells.Num(); ++ColumnIndex)
	{
		if (!Before.Rows[0].Cells[ColumnIndex].Equals(
			After.Rows[0].Cells[ColumnIndex], SearchCase(bCaseSensitive)))
		{
			OutDiff = FDirectiveUtilCsvDiff();
			OutErrorMessage = TEXT("The documents have different headers.");
			return false;
		}
	}
	int32 KeyColumn = INDEX_NONE;
	if (!FindUniqueHeader(Before, KeyHeaderCopy, bCaseSensitive, KeyColumn))
	{
		OutDiff = FDirectiveUtilCsvDiff();
		OutErrorMessage = FString::Printf(TEXT("Header '%s' is missing or repeated."), *KeyHeaderCopy);
		return false;
	}

	TMap<FString, TPair<FString, const FDirectiveUtilCsvRow*>> BeforeRows;
	TMap<FString, TPair<FString, const FDirectiveUtilCsvRow*>> AfterRows;
	if (!BuildKeyedRows(Before, KeyHeaderCopy, bCaseSensitive, BeforeRows, ErrorMessage)
		|| !BuildKeyedRows(After, KeyHeaderCopy, bCaseSensitive, AfterRows, ErrorMessage))
	{
		OutDiff = FDirectiveUtilCsvDiff();
		OutErrorMessage = MoveTemp(ErrorMessage);
		return false;
	}

	for (const TPair<FString, TPair<FString, const FDirectiveUtilCsvRow*>>& Row : BeforeRows)
	{
		const TPair<FString, const FDirectiveUtilCsvRow*>* AfterRow = AfterRows.Find(Row.Key);
		if (AfterRow == nullptr)
		{
			Diff.RemovedKeys.Add(Row.Value.Key);
		}
		else
		{
			const TArray<FString>& BeforeCells = Row.Value.Value->Cells;
			const TArray<FString>& AfterCells = AfterRow->Value->Cells;
			bool bChanged = BeforeCells.Num() != AfterCells.Num();
			for (int32 ColumnIndex = 0; !bChanged && ColumnIndex < BeforeCells.Num(); ++ColumnIndex)
			{
				if (ColumnIndex != KeyColumn && BeforeCells[ColumnIndex] != AfterCells[ColumnIndex])
				{
					bChanged = true;
				}
			}
			if (bChanged)
			{
				Diff.ChangedKeys.Add(AfterRow->Key);
			}
		}
	}
	for (const TPair<FString, TPair<FString, const FDirectiveUtilCsvRow*>>& Row : AfterRows)
	{
		if (!BeforeRows.Contains(Row.Key))
		{
			Diff.AddedKeys.Add(Row.Value.Key);
		}
	}
	Diff.AddedKeys.Sort();
	Diff.RemovedKeys.Sort();
	Diff.ChangedKeys.Sort();
	OutDiff = MoveTemp(Diff);
	OutErrorMessage.Reset();
	return true;
}
