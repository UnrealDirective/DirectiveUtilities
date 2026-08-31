// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilCsvFunctionLibraryTest,
	"DirectiveUtilities.CsvFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilCsvFunctionLibraryTest::RunTest(const FString& Parameters)
{
	FDirectiveUtilCsvDocument Document;
	FString ErrorMessage;

	TestTrue(TEXT("Simple CSV parses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b,c\n1,2,3"), EDirectiveUtilCsvDelimiter::Comma, Document, ErrorMessage));
	TestEqual(TEXT("Two rows parsed"), UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(Document), 2);
	TestEqual(TEXT("The delimiter is stored on the document"), Document.Delimiter, EDirectiveUtilCsvDelimiter::Comma);
	FString Cell;
	TestTrue(TEXT("Cell (0,0) reads back"), UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 0, 0, Cell));
	TestEqual(TEXT("Cell (0,0) is 'a'"), Cell, FString(TEXT("a")));
	TestTrue(TEXT("Cell (1,2) reads back"), UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 1, 2, Cell));
	TestEqual(TEXT("Cell (1,2) is '3'"), Cell, FString(TEXT("3")));
	TestTrue(TEXT("A cell may alias its output"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 0, 0, Document.Rows[0].Cells[0]));
	TestEqual(TEXT("The aliased cell remains intact"), Document.Rows[0].Cells[0], FString(TEXT("a")));

	FString AliasedCsvText(TEXT("left,right"));
	FDirectiveUtilCsvDocument AliasedDocument;
	TestTrue(TEXT("CSV text may alias the error output"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(
			AliasedCsvText, EDirectiveUtilCsvDelimiter::Comma, AliasedDocument, AliasedCsvText));
	TestEqual(TEXT("A successful aliased parse clears the error"), AliasedCsvText, FString());
	AliasedCsvText = TEXT("\"open");
	TestFalse(TEXT("An aliased failing parse still reports its error"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(
			AliasedCsvText, EDirectiveUtilCsvDelimiter::Comma, AliasedDocument, AliasedCsvText));
	TestTrue(TEXT("The aliased parse error is populated"), !AliasedCsvText.IsEmpty());

	FDirectiveUtilCsvDocument InvalidDelimiterDocument;
	FString InvalidDelimiterError;
	TestFalse(TEXT("An invalid delimiter value is rejected"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(
			TEXT("a,b"), static_cast<EDirectiveUtilCsvDelimiter>(255), InvalidDelimiterDocument, InvalidDelimiterError));
	TestEqual(TEXT("An invalid delimiter returns no rows"), InvalidDelimiterDocument.Rows.Num(), 0);
	TestTrue(TEXT("An invalid delimiter explains the failure"), !InvalidDelimiterError.IsEmpty());
	TestEqual(TEXT("An invalid delimiter has no character"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvDelimiterCharacter(static_cast<EDirectiveUtilCsvDelimiter>(255)), FString());

	TestTrue(TEXT("Empty input parses to an empty document"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(FString(), EDirectiveUtilCsvDelimiter::Comma, Document, ErrorMessage));
	TestEqual(TEXT("Empty input has no rows"), UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(Document), 0);
	TestEqual(TEXT("Empty input reports no error"), ErrorMessage, FString());

	const FString Quoted = TEXT("\"name, with comma\",\"line\nbreak\",\"say \"\"hi\"\"\"");
	TestTrue(TEXT("RFC 4180 quoting parses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(Quoted, EDirectiveUtilCsvDelimiter::Comma, Document, ErrorMessage));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 0, 0, Cell);
	TestEqual(TEXT("A quoted comma stays inside one cell"), Cell, FString(TEXT("name, with comma")));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 0, 1, Cell);
	TestEqual(TEXT("A newline stays inside one cell"), Cell, FString(TEXT("line\nbreak")));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Document, 0, 2, Cell);
	TestEqual(TEXT("Doubled quotes become one quote"), Cell, FString(TEXT("say \"hi\"")));

	FDirectiveUtilCsvDocument CrlfDocument;
	TestTrue(TEXT("CRLF terminators parse"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b\r\nc,d\r\n"), EDirectiveUtilCsvDelimiter::Comma, CrlfDocument, ErrorMessage));
	TestEqual(TEXT("CRLF produces two rows"), UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(CrlfDocument), 2);

	FDirectiveUtilCsvDocument TabDocument;
	TestTrue(TEXT("Tab delimiter parses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a\tb\nc\td"), EDirectiveUtilCsvDelimiter::Tab, TabDocument, ErrorMessage));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(TabDocument, 1, 1, Cell);
	TestEqual(TEXT("Tab-delimited second column reads back"), Cell, FString(TEXT("d")));
	TestEqual(TEXT("The tab delimiter character"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvDelimiterCharacter(EDirectiveUtilCsvDelimiter::Tab), FString(TEXT("\t")));
	TestEqual(TEXT("The semicolon delimiter character"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvDelimiterCharacter(EDirectiveUtilCsvDelimiter::Semicolon), FString(TEXT(";")));

	FDirectiveUtilCsvDocument Unterminated;
	TestFalse(TEXT("An unclosed quoted field fails"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("\"open"), EDirectiveUtilCsvDelimiter::Comma, Unterminated, ErrorMessage));
	TestTrue(TEXT("The failure names the problem"), !ErrorMessage.IsEmpty());

	FDirectiveUtilCsvDocument Partial;
	TestFalse(TEXT("A failure past the first row is still a failure"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b\nc,d\n\"e\"f"), EDirectiveUtilCsvDelimiter::Comma, Partial, ErrorMessage));
	TestEqual(TEXT("A failed parse leaves no partial rows"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(Partial), 0);

	FDirectiveUtilCsvDocument StrayQuote;
	TestFalse(TEXT("Content after a closing quote fails"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("\"a\"b"), EDirectiveUtilCsvDelimiter::Comma, StrayQuote, ErrorMessage));
	FDirectiveUtilCsvDocument UnquotedQuote;
	TestTrue(TEXT("A quote inside an unquoted field is content"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a\"b"), EDirectiveUtilCsvDelimiter::Comma, UnquotedQuote, ErrorMessage));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(UnquotedQuote, 0, 0, Cell);
	TestEqual(TEXT("The embedded quote survives"), Cell, FString(TEXT("a\"b")));

	FDirectiveUtilCsvDocument BlankLines;
	TestTrue(TEXT("Blank lines parse"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b\n\nc,d\n\n"), EDirectiveUtilCsvDelimiter::Comma, BlankLines, ErrorMessage));
	TestEqual(TEXT("Blank lines contribute no rows"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(BlankLines), 2);
	FDirectiveUtilCsvDocument EmptyCellRow;
	TestTrue(TEXT("A row of empty cells still parses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b\n,\n"), EDirectiveUtilCsvDelimiter::Comma, EmptyCellRow, ErrorMessage));
	TestEqual(TEXT("A comma-only line is a real row"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(EmptyCellRow), 2);

	FDirectiveUtilCsvDocument DefaultDocument;
	FString DefaultSerialized;
	UDirectiveUtilCsvFunctionLibrary::AddCsvRow(DefaultDocument, { TEXT("a"), TEXT("b") });
	UDirectiveUtilCsvFunctionLibrary::WriteCsv(DefaultDocument, DefaultSerialized);
	TestEqual(TEXT("A default-constructed document writes comma-separated"),
		DefaultSerialized, FString(TEXT("a,b\n")));
	TestTrue(TEXT("A write output may alias a document cell"),
		DefaultDocument.Rows[0].Cells.IsValidIndex(0));
	UDirectiveUtilCsvFunctionLibrary::WriteCsv(
		DefaultDocument, DefaultDocument.Rows[0].Cells[0]);
	TestEqual(TEXT("The aliased write output is complete"),
		DefaultDocument.Rows[0].Cells[0], FString(TEXT("a,b\n")));

	FDirectiveUtilCsvDocument Ragged;
	TestTrue(TEXT("Ragged rows parse"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("a,b,c\nd"), EDirectiveUtilCsvDelimiter::Comma, Ragged, ErrorMessage));
	TestEqual(TEXT("Row 0 holds three cells"), UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(Ragged, 0), 3);
	TestEqual(TEXT("Row 1 holds one cell"), UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(Ragged, 1), 1);
	TestEqual(TEXT("Out-of-range row columns report -1"), UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(Ragged, 9), -1);
	FString MissingCell;
	TestFalse(TEXT("A missing cell within an existing row fails the read"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Ragged, 1, 2, MissingCell));
	TestEqual(TEXT("A missing cell reads as empty"), MissingCell, FString());
	TestFalse(TEXT("A missing row fails the read"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Ragged, 7, 0, MissingCell));

	FDirectiveUtilCsvDocument RoundTrip;
	UDirectiveUtilCsvFunctionLibrary::AddCsvRow(RoundTrip, { TEXT("plain"), TEXT("with,comma"), TEXT("with\"quote"), TEXT("") });
	UDirectiveUtilCsvFunctionLibrary::AddCsvRow(RoundTrip, { TEXT("3.14"), TEXT(""), TEXT("tail") });
	RoundTrip.Delimiter = EDirectiveUtilCsvDelimiter::Comma;

	FString Serialized;
	UDirectiveUtilCsvFunctionLibrary::WriteCsv(RoundTrip, Serialized);
	TestEqual(TEXT("Write quotes only special fields"),
		Serialized,
		FString(TEXT("plain,\"with,comma\",\"with\"\"quote\",\n3.14,,tail\n")));

	FDirectiveUtilCsvDocument Reparsed;
	FString ReparseError;
	TestTrue(TEXT("Written text reparses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(Serialized, EDirectiveUtilCsvDelimiter::Comma, Reparsed, ReparseError));
	TestEqual(TEXT("Round-trip keeps both rows"), UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(Reparsed), 2);
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Reparsed, 0, 2, Cell);
	TestEqual(TEXT("Round-trip preserves the quoted content"), Cell, FString(TEXT("with\"quote")));

	FDirectiveUtilCsvDocument Editable;
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(TEXT("x,y"), EDirectiveUtilCsvDelimiter::Comma, Editable, ReparseError);
	TestTrue(TEXT("SetCsvCell writes an existing cell"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, 0, 1, TEXT("z")));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Editable, 0, 1, Cell);
	TestEqual(TEXT("The edited value sticks"), Cell, FString(TEXT("z")));
	TestTrue(TEXT("SetCsvCell appends one cell past the end"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, 0, 2, TEXT("far")));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCell(Editable, 0, 2, Cell);
	TestEqual(TEXT("The appended cell stores the requested value"), Cell, FString(TEXT("far")));
	TestFalse(TEXT("SetCsvCell refuses to leave a gap"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, 0, 9, TEXT("no")));
	TestEqual(TEXT("The rejected write did not grow the row"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(Editable, 0), 3);
	TestFalse(TEXT("SetCsvCell rejects out-of-range rows"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, 5, 0, TEXT("no")));
	TestFalse(TEXT("SetCsvCell rejects negative indices"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, -1, 0, TEXT("no")));
	TestTrue(TEXT("SetCsvCell accepts a value from the row it grows"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCell(Editable, 0, 3, Editable.Rows[0].Cells[0]));
	TestEqual(TEXT("The aliased appended value survives growth"), Editable.Rows[0].Cells[3], FString(TEXT("x")));

	const int32 AddedIndex = UDirectiveUtilCsvFunctionLibrary::AddCsvRow(Editable, { TEXT("new") });
	TestEqual(TEXT("AddCsvRow returns the new row index"), AddedIndex, 1);
	TestTrue(TEXT("RemoveCsvRow deletes an existing row"),
		UDirectiveUtilCsvFunctionLibrary::RemoveCsvRow(Editable, 1));
	TestEqual(TEXT("Removal shrinks the document"), UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(Editable), 1);
	TestFalse(TEXT("RemoveCsvRow rejects out-of-range rows"),
		UDirectiveUtilCsvFunctionLibrary::RemoveCsvRow(Editable, 1));

	FDirectiveUtilCsvDocument EmptyRowDocument;
	TestEqual(TEXT("Adding no cells creates the first row"),
		UDirectiveUtilCsvFunctionLibrary::AddCsvRow(EmptyRowDocument, {}), 0);
	TestEqual(TEXT("A row added without cells contains one empty cell"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvColumnCount(EmptyRowDocument, 0), 1);
	FString EmptyRowText;
	UDirectiveUtilCsvFunctionLibrary::WriteCsv(EmptyRowDocument, EmptyRowText);
	TestEqual(TEXT("The empty row has an explicit CSV representation"), EmptyRowText, FString(TEXT("\"\"\n")));
	FDirectiveUtilCsvDocument ReparsedEmptyRow;
	TestTrue(TEXT("The explicit empty row reparses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(
			EmptyRowText, EDirectiveUtilCsvDelimiter::Comma, ReparsedEmptyRow, ReparseError));
	TestEqual(TEXT("Reparsing keeps the empty row"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvRowCount(ReparsedEmptyRow), 1);

	FDirectiveUtilCsvDocument Keyed;
	TestTrue(TEXT("A keyed document parses"),
		UDirectiveUtilCsvFunctionLibrary::ParseCsv(
			TEXT("Id,Name,Score\nPlayerOne,Ada,10\nPlayerTwo,Lin,20\n"),
			EDirectiveUtilCsvDelimiter::Comma, Keyed, ErrorMessage));
	int32 ColumnIndex = INDEX_NONE;
	TestTrue(TEXT("Header lookup ignores case by default"),
		UDirectiveUtilCsvFunctionLibrary::FindCsvColumn(Keyed, TEXT("score"), ColumnIndex, false));
	TestEqual(TEXT("Header lookup returns the column index"), ColumnIndex, 2);
	TestTrue(TEXT("A cell reads by header"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCellByHeader(Keyed, 1, TEXT("Name"), Cell, false));
	TestEqual(TEXT("The named cell reads back"), Cell, FString(TEXT("Ada")));
	Cell = TEXT("stale");
	TestFalse(TEXT("A missing header fails the cell read"),
		UDirectiveUtilCsvFunctionLibrary::GetCsvCellByHeader(Keyed, 1, TEXT("Missing"), Cell, false));
	TestEqual(TEXT("A missing header clears the cell output"), Cell, FString());
	TestTrue(TEXT("A cell writes by header"),
		UDirectiveUtilCsvFunctionLibrary::SetCsvCellByHeader(Keyed, 1, TEXT("Score"), TEXT("11"), false));

	int32 RowIndex = INDEX_NONE;
	TestTrue(TEXT("A row can be found by key"),
		UDirectiveUtilCsvFunctionLibrary::FindCsvRowByKey(Keyed, TEXT("Id"), TEXT("playerone"), RowIndex, false));
	TestEqual(TEXT("Key lookup returns the data row index"), RowIndex, 1);
	TMap<FString, FString> UpdatedValues;
	UpdatedValues.Add(TEXT("Id"), TEXT("WrongKey"));
	UpdatedValues.Add(TEXT("Name"), TEXT("Ada Lovelace"));
	UpdatedValues.Add(TEXT("Score"), TEXT("12"));
	TestTrue(TEXT("Upsert updates an existing row"),
		UDirectiveUtilCsvFunctionLibrary::UpsertCsvRowByKey(
			Keyed, TEXT("Id"), TEXT("PlayerOne"), UpdatedValues, RowIndex, ErrorMessage, false));
	TestEqual(TEXT("Upsert preserves the existing row index"), RowIndex, 1);
	UDirectiveUtilCsvFunctionLibrary::GetCsvCellByHeader(Keyed, RowIndex, TEXT("Name"), Cell, false);
	TestEqual(TEXT("Upsert writes named values"), Cell, FString(TEXT("Ada Lovelace")));
	UDirectiveUtilCsvFunctionLibrary::GetCsvCellByHeader(Keyed, RowIndex, TEXT("Id"), Cell, false);
	TestEqual(TEXT("The key input remains authoritative"), Cell, FString(TEXT("PlayerOne")));
	FString AliasedUpsertHeader(TEXT("Id"));
	TestTrue(TEXT("The upsert key header may alias the error output"),
		UDirectiveUtilCsvFunctionLibrary::UpsertCsvRowByKey(
			Keyed, AliasedUpsertHeader, TEXT("PlayerOne"), UpdatedValues,
			RowIndex, AliasedUpsertHeader, false));
	TestEqual(TEXT("A successful aliased upsert clears the error"), AliasedUpsertHeader, FString());

	TMap<FString, FString> NewValues;
	NewValues.Add(TEXT("Name"), TEXT("Grace"));
	NewValues.Add(TEXT("Score"), TEXT("30"));
	TestTrue(TEXT("Upsert adds a missing row"),
		UDirectiveUtilCsvFunctionLibrary::UpsertCsvRowByKey(
			Keyed, TEXT("Id"), TEXT("PlayerThree"), NewValues, RowIndex, ErrorMessage, false));
	TestEqual(TEXT("The new row is appended"), RowIndex, 3);
	TestTrue(TEXT("A row removes by key"),
		UDirectiveUtilCsvFunctionLibrary::RemoveCsvRowByKey(Keyed, TEXT("Id"), TEXT("PlayerTwo"), false));
	TestEqual(TEXT("Removing by key shrinks the document"), Keyed.Rows.Num(), 3);

	TArray<FString> MissingHeaders;
	TArray<FString> DuplicateHeaders;
	TestTrue(TEXT("Required headers validate"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvHeaders(
			Keyed, { TEXT("id"), TEXT("name") }, MissingHeaders, DuplicateHeaders, false));
	TestFalse(TEXT("A missing required header fails validation"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvHeaders(
			Keyed, { TEXT("Missing") }, MissingHeaders, DuplicateHeaders, false));
	TestEqual(TEXT("The missing header is reported"), MissingHeaders, TArray<FString>({ TEXT("Missing") }));

	FDirectiveUtilCsvDocument DuplicateHeadersDocument;
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(
		TEXT("Id,Name,name\nOne,A,B"), EDirectiveUtilCsvDelimiter::Comma, DuplicateHeadersDocument, ErrorMessage);
	TestFalse(TEXT("Duplicate headers fail validation"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvHeaders(
			DuplicateHeadersDocument, {}, MissingHeaders, DuplicateHeaders, false));
	TestEqual(TEXT("The duplicate header is reported once"), DuplicateHeaders.Num(), 1);
	TestFalse(TEXT("A repeated header cannot be resolved"),
		UDirectiveUtilCsvFunctionLibrary::FindCsvColumn(DuplicateHeadersDocument, TEXT("name"), ColumnIndex, false));
	TestFalse(TEXT("Header validation rejects aliased outputs"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvHeaders(
			DuplicateHeadersDocument, {}, MissingHeaders, MissingHeaders, false));
	TestEqual(TEXT("Aliased header outputs are cleared"), MissingHeaders.Num(), 0);

	TArray<int32> InvalidRows;
	TestFalse(TEXT("A ragged document fails shape validation"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvShape(Ragged, InvalidRows));
	TestEqual(TEXT("Shape validation reports zero-based row indices"), InvalidRows, TArray<int32>({ 1 }));
	TestTrue(TEXT("A rectangular document passes shape validation"),
		UDirectiveUtilCsvFunctionLibrary::ValidateCsvShape(Keyed, InvalidRows));

	FDirectiveUtilCsvDocument BeforeDiff;
	FDirectiveUtilCsvDocument AfterDiff;
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(
		TEXT("Id,Value\nKeep,1\nChange,2\nRemoveMe,3"), EDirectiveUtilCsvDelimiter::Comma, BeforeDiff, ErrorMessage);
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(
		TEXT("ID,VALUE\nkeep,1\nCHANGE,4\nAddMe,5"), EDirectiveUtilCsvDelimiter::Comma, AfterDiff, ErrorMessage);
	FDirectiveUtilCsvDiff Diff;
	TestTrue(TEXT("Keyed documents diff"),
		UDirectiveUtilCsvFunctionLibrary::DiffCsvByKey(
			BeforeDiff, AfterDiff, TEXT("Id"), Diff, ErrorMessage, false));
	TestEqual(TEXT("Diff reports added keys with source casing"), Diff.AddedKeys, TArray<FString>({ TEXT("AddMe") }));
	TestEqual(TEXT("Diff reports removed keys with source casing"), Diff.RemovedKeys, TArray<FString>({ TEXT("RemoveMe") }));
	TestEqual(TEXT("Diff reports changed keys with current casing"), Diff.ChangedKeys, TArray<FString>({ TEXT("CHANGE") }));
	FString AliasedKeyHeader(TEXT("Id"));
	TestTrue(TEXT("The diff key header may alias the error output"),
		UDirectiveUtilCsvFunctionLibrary::DiffCsvByKey(
			BeforeDiff, AfterDiff, AliasedKeyHeader, Diff, AliasedKeyHeader, false));
	TestEqual(TEXT("A successful aliased diff clears the error"), AliasedKeyHeader, FString());

	FDirectiveUtilCsvDocument DuplicateKeys;
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(
		TEXT("Id,Value\nOne,1\none,2"), EDirectiveUtilCsvDelimiter::Comma, DuplicateKeys, ErrorMessage);
	TestFalse(TEXT("Duplicate keys reject a diff"),
		UDirectiveUtilCsvFunctionLibrary::DiffCsvByKey(
			DuplicateKeys, AfterDiff, TEXT("Id"), Diff, ErrorMessage, false));
	const int32 DuplicateRowCount = DuplicateKeys.Rows.Num();
	TMap<FString, FString> DuplicateUpdate;
	DuplicateUpdate.Add(TEXT("Value"), TEXT("3"));
	TestFalse(TEXT("Duplicate keys reject an upsert"),
		UDirectiveUtilCsvFunctionLibrary::UpsertCsvRowByKey(
			DuplicateKeys, TEXT("Id"), TEXT("One"), DuplicateUpdate, RowIndex, ErrorMessage, false));
	TestEqual(TEXT("A rejected upsert leaves every row in place"), DuplicateKeys.Rows.Num(), DuplicateRowCount);

	FDirectiveUtilCsvDocument DifferentHeaders;
	UDirectiveUtilCsvFunctionLibrary::ParseCsv(
		TEXT("Id,Other\nOne,1"), EDirectiveUtilCsvDelimiter::Comma, DifferentHeaders, ErrorMessage);
	TestFalse(TEXT("Different schemas reject a diff"),
		UDirectiveUtilCsvFunctionLibrary::DiffCsvByKey(
			BeforeDiff, DifferentHeaders, TEXT("Id"), Diff, ErrorMessage, false));

	return true;
}
