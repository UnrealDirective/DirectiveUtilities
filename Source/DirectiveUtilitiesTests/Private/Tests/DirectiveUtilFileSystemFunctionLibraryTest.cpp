// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString GetTestRoot()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("DirectiveUtilTests") / TEXT("FileSystem");
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilFileSystemFunctionLibraryTest,
	"DirectiveUtilities.FileSystemFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilFileSystemFunctionLibraryTest::RunTest(const FString& Parameters)
{
	IFileManager& FileManager = IFileManager::Get();
	const FString TestRoot = GetTestRoot();
	FileManager.DeleteDirectory(*TestRoot, false, true);
	const FString NestedDirectory = TestRoot / TEXT("Nested") / TEXT("Deeper");
	TestTrue(TEXT("The test directory is created"), FileManager.MakeDirectory(*NestedDirectory, true));

	const FString TextPath = NestedDirectory / TEXT("Notes.txt");
	const FString Contents = TEXT("first line\nsecond line\n\nfourth line");
	TestTrue(TEXT("Text writes into an existing directory"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(TextPath, Contents));
	TestFalse(TEXT("Text overwrite can be refused"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(TextPath, TEXT("nope"), true, false));

	FString ReadBack;
	TestTrue(TEXT("Text reads after writing"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(TextPath, ReadBack));
	TestEqual(TEXT("Text round trips"), ReadBack, Contents);
	FString AliasedPath = TextPath;
	TestTrue(TEXT("A text read supports aliased path and output"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(AliasedPath, AliasedPath));
	TestEqual(TEXT("The aliased text read is exact"), AliasedPath, Contents);

	TArray<FString> Lines;
	TestTrue(TEXT("Lines read with empty entries"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(TextPath, Lines, true));
	TestEqual(TEXT("Line splitting is exact"), Lines,
		TArray<FString>({ TEXT("first line"), TEXT("second line"), TEXT(""), TEXT("fourth line") }));

	const FString AppendPath = TestRoot / TEXT("Append.txt");
	TestTrue(TEXT("Append creates a missing file"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(AppendPath, TEXT("one")));
	TestTrue(TEXT("Append adds bytes without a separator"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(AppendPath, TEXT("two")));
	FString Appended;
	UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(AppendPath, Appended);
	TestEqual(TEXT("Appended content is exact"), Appended, FString(TEXT("onetwo")));

	const FString UnicodeAppendPath = TestRoot / TEXT("UnicodeAppend.txt");
	TestTrue(TEXT("A UTF-16 source file is created"),
		FFileHelper::SaveStringToFile(
			TEXT("wide"), *UnicodeAppendPath, FFileHelper::EEncodingOptions::ForceUnicode));
	TestTrue(TEXT("Appending converts supported source text to UTF-8"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(UnicodeAppendPath, TEXT(" text")));
	TestTrue(TEXT("Converted appended text reads"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(UnicodeAppendPath, Appended));
	TestEqual(TEXT("Converted appended text is exact"), Appended, FString(TEXT("wide text")));

	const TCHAR EmbeddedNullCharacters[] = { TEXT('a'), TEXT('\0'), TEXT('b') };
	const FString EmbeddedNull = FString::ConstructFromPtrSize(EmbeddedNullCharacters, 3);
	const FString EmbeddedNullPath = TestRoot / TEXT("EmbeddedNull.txt");
	TestTrue(TEXT("Text with an embedded null writes"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(EmbeddedNullPath, EmbeddedNull));
	TestTrue(TEXT("Text can append after an embedded null"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(EmbeddedNullPath, TEXT("c")));
	TestTrue(TEXT("Text with an embedded null reads"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(EmbeddedNullPath, Appended));
	TestEqual(TEXT("Embedded null text is not truncated"), Appended.Len(), 4);
	TestTrue(TEXT("The appended text position exists"), Appended.IsValidIndex(3));
	if (Appended.IsValidIndex(3))
	{
		TestEqual(TEXT("Text after the embedded null survives"), Appended[3], TEXT('c'));
	}

	const TArray<uint8> BinaryBytes = { 0x00, 0x01, 0xFE, 0xFF, 0x7F };
	const FString BinaryPath = TestRoot / TEXT("Data.bin");
	TestTrue(TEXT("Binary data writes"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(BinaryPath, BinaryBytes));
	TArray<uint8> BinaryReadBack;
	TestTrue(TEXT("Binary data reads"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, BinaryReadBack));
	TestEqual(TEXT("Binary data round trips"), BinaryReadBack, BinaryBytes);
	TestFalse(TEXT("Binary overwrite can be refused"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(
			BinaryPath, TArray<uint8>({ 0xAA }), true, false));
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, BinaryReadBack);
	TestEqual(TEXT("A refused binary write preserves the destination"), BinaryReadBack, BinaryBytes);

	const FString NoOverwriteCreatePath = TestRoot / TEXT("CreateWithoutOverwrite.txt");
	TestTrue(TEXT("No-overwrite mode can create a missing file"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(
			NoOverwriteCreatePath, TEXT("created"), true, false));
	TestFalse(TEXT("No-overwrite mode refuses the file after creation"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(
			NoOverwriteCreatePath, TEXT("changed"), true, false));

	const FString AtomicTextPath = TestRoot / TEXT("Atomic.txt");
	TestTrue(TEXT("Atomic text creates a file"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(AtomicTextPath, TEXT("old")));
	TestTrue(TEXT("Atomic text replaces a file"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(AtomicTextPath, TEXT("new")));
	TestFalse(TEXT("Atomic text can refuse replacement"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(AtomicTextPath, TEXT("nope"), true, false));
	UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(AtomicTextPath, ReadBack);
	TestEqual(TEXT("A refused atomic write preserves the destination"), ReadBack, FString(TEXT("new")));

	const FString AtomicBinaryPath = TestRoot / TEXT("Atomic.bin");
	TestTrue(TEXT("Atomic binary creates a file"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(AtomicBinaryPath, BinaryBytes));
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(AtomicBinaryPath, BinaryReadBack);
	TestEqual(TEXT("Atomic binary round trips"), BinaryReadBack, BinaryBytes);
	TestFalse(TEXT("Atomic binary can refuse replacement"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(
			AtomicBinaryPath, TArray<uint8>({ 0xAA }), true, false));
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(AtomicBinaryPath, BinaryReadBack);
	TestEqual(TEXT("A refused atomic binary write preserves the destination"), BinaryReadBack, BinaryBytes);

	TestTrue(TEXT("File size reports written bytes"),
		UDirectiveUtilFileSystemFunctionLibrary::GetFileSize(BinaryPath) == BinaryBytes.Num());
	FDateTime Timestamp;
	TestTrue(TEXT("File timestamp reads"),
		UDirectiveUtilFileSystemFunctionLibrary::GetFileTimeStamp(TextPath, Timestamp));

	const FString RelativePath = FString(TEXT("DirectiveUtilTests")) / TEXT("Relative.txt");
	TestTrue(TEXT("Saved-relative text writes"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(RelativePath, TEXT("relative")));
	const FString ExpectedAbsolute = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / RelativePath;
	TestTrue(TEXT("Saved-relative text lands under Saved"), FPaths::FileExists(ExpectedAbsolute));

	TestFalse(TEXT("Empty paths fail reads"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(FString(), ReadBack));
	const TCHAR EmbeddedNullPathCharacters[] = {
		TEXT('B'), TEXT('a'), TEXT('d'), TEXT('\0'), TEXT('.'), TEXT('t'), TEXT('x'), TEXT('t')
	};
	const FString EmbeddedNullInputPath = FString::ConstructFromPtrSize(
		EmbeddedNullPathCharacters, UE_ARRAY_COUNT(EmbeddedNullPathCharacters));
	ReadBack = TEXT("stale");
	TestFalse(TEXT("Paths with embedded null characters fail safely"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(EmbeddedNullInputPath, ReadBack));
	TestEqual(TEXT("A rejected path clears the text output"), ReadBack, FString());
	TestFalse(TEXT("Paths with embedded null characters cannot be written"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(EmbeddedNullInputPath, TEXT("data")));
	const FString MissingParentPath = TestRoot / TEXT("MissingParent") / TEXT("File.txt");
	TestFalse(TEXT("Directory creation can be refused"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(
			MissingParentPath, TEXT("data"), false, true));
	TestFalse(TEXT("A refused parent directory is not created"), FPaths::FileExists(MissingParentPath));
	TestEqual(TEXT("Missing file size is negative"),
		UDirectiveUtilFileSystemFunctionLibrary::GetFileSize(TestRoot / TEXT("Missing.bin")), static_cast<int64>(-1));
	Timestamp = FDateTime::MaxValue();
	TestFalse(TEXT("A missing timestamp fails"),
		UDirectiveUtilFileSystemFunctionLibrary::GetFileTimeStamp(TestRoot / TEXT("Missing.bin"), Timestamp));
	TestEqual(TEXT("A failed timestamp resets its output"), Timestamp, FDateTime());

	FileManager.DeleteDirectory(*TestRoot, false, true);
	return true;
}
