// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "Containers/StringConv.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

#if PLATFORM_MAC && WITH_EDITOR
#include <unistd.h>
#endif

namespace
{
	FString GetTestRoot()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("DirectiveUtilTests") / TEXT("FileSystem");
	}

	int32 CountTemporaryFiles(const FString& Directory)
	{
		TArray<FString> TemporaryFiles;
		IFileManager::Get().FindFiles(TemporaryFiles, *(Directory / TEXT("*.tmp-*")), true, false);
		return TemporaryFiles.Num();
	}

	// Packaged Mac games run in the App Sandbox, which cannot create or attach disk images.
#if PLATFORM_MAC && WITH_EDITOR
	bool RunHdiutil(const FString& Arguments, FString& OutOutput)
	{
		int32 ReturnCode = INDEX_NONE;
		FString StandardOutput;
		FString StandardError;
		const bool bLaunched = FPlatformProcess::ExecProcess(
			TEXT("/usr/bin/hdiutil"), *Arguments, &ReturnCode, &StandardOutput, &StandardError);
		OutOutput = StandardOutput + StandardError;
		return bLaunched && ReturnCode == 0;
	}

	class FTemporaryFatVolume
	{
	public:
		explicit FTemporaryFatVolume(FAutomationTestBase& InTest)
			: Test(InTest)
			, Root(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("DirectiveUtilTests")
				/ (TEXT("FatVolume-") + FGuid::NewGuid().ToString(EGuidFormats::Digits)))
			, ImagePath(Root / TEXT("Volume.dmg"))
			, MountPath(Root / TEXT("Mount"))
		{
		}

		~FTemporaryFatVolume()
		{
			Release();
		}

		const FString& GetMountPath() const
		{
			return MountPath;
		}

		bool Attach()
		{
			FString Output;
			if (!IFileManager::Get().MakeDirectory(*MountPath, true))
			{
				Test.AddError(TEXT("The FAT volume mount directory could not be created."));
				return false;
			}
			if (!RunHdiutil(FString::Printf(TEXT("create -size 8m -fs \"MS-DOS FAT16\" -volname UDFATTEST \"%s\""),
				*ImagePath), Output))
			{
				Test.AddError(FString::Printf(TEXT("hdiutil could not create the FAT disk image: %s"), *Output));
				return false;
			}
			if (!RunHdiutil(FString::Printf(TEXT("attach -nobrowse -mountpoint \"%s\" \"%s\""), *MountPath, *ImagePath), Output))
			{
				Test.AddError(FString::Printf(TEXT("hdiutil could not mount the FAT disk image: %s"), *Output));
				return false;
			}
			bAttached = true;
			return true;
		}

	private:
		void Release()
		{
			if (bAttached)
			{
				FString Output;
				const FString DetachArguments = FString::Printf(TEXT("detach \"%s\""), *MountPath);
				const FString ForceDetachArguments = FString::Printf(TEXT("detach -force \"%s\""), *MountPath);
				if (!RunHdiutil(DetachArguments, Output) && !RunHdiutil(ForceDetachArguments, Output))
				{
					Test.AddError(FString::Printf(TEXT("hdiutil could not detach the FAT volume, so it was left in place: %s"), *Output));
					return;
				}
				bAttached = false;
			}
			IFileManager::Get().DeleteDirectory(*Root, false, true);
		}

		FAutomationTestBase& Test;
		FString Root;
		FString ImagePath;
		FString MountPath;
		bool bAttached = false;
	};

	bool CanCreateHardLink(const FString& ExistingPath, const FString& LinkPath)
	{
		const FTCHARToUTF8 ExistingUtf8(*ExistingPath);
		const FTCHARToUTF8 LinkUtf8(*LinkPath);
		return link(ExistingUtf8.Get(), LinkUtf8.Get()) == 0;
	}

	void TestAtomicWritesWithoutHardLinks(FAutomationTestBase& Test)
	{
		FTemporaryFatVolume Volume(Test);
		if (!Volume.Attach())
		{
			return;
		}

		const FString& MountPath = Volume.GetMountPath();
		const FString ProbePath = MountPath / TEXT("Probe.bin");
		const FString ProbeLinkPath = MountPath / TEXT("ProbeLink.bin");
		Test.TestTrue(TEXT("The FAT volume accepts a plain write"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(ProbePath, TArray<uint8>({ 1 })));
		Test.TestFalse(TEXT("The FAT volume refuses hard links"), CanCreateHardLink(ProbePath, ProbeLinkPath));
		IFileManager::Get().Delete(*ProbePath, false, true, true);
		IFileManager::Get().Delete(*ProbeLinkPath, false, true, true);

		const TArray<uint8> FirstBytes = { 0x00, 0x01, 0xFE, 0xFF, 0x7F };
		const TArray<uint8> SecondBytes = { 0xAA, 0xBB };
		const FString BinaryPath = MountPath / TEXT("Created.bin");
		TArray<uint8> ReadBack;
		Test.TestTrue(TEXT("A no-overwrite atomic binary write creates a file without hard links"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(BinaryPath, FirstBytes, true, false));
		UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, ReadBack);
		Test.TestEqual(TEXT("The created binary file holds the exact bytes"), ReadBack, FirstBytes);
		Test.TestFalse(TEXT("A second no-overwrite atomic binary write fails without hard links"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(BinaryPath, SecondBytes, true, false));
		UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, ReadBack);
		Test.TestEqual(TEXT("A refused no-overwrite write leaves the binary file unchanged"), ReadBack, FirstBytes);
		Test.TestTrue(TEXT("An overwriting atomic binary write replaces the file"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(BinaryPath, SecondBytes, true, true));
		UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BinaryPath, ReadBack);
		Test.TestEqual(TEXT("The overwritten binary file holds the new bytes"), ReadBack, SecondBytes);

		const FString TextPath = MountPath / TEXT("Created.txt");
		FString TextReadBack;
		Test.TestTrue(TEXT("A no-overwrite atomic text write creates a file without hard links"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(TextPath, TEXT("created"), true, false));
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(TextPath, TextReadBack);
		Test.TestEqual(TEXT("The created text file holds the exact text"), TextReadBack, FString(TEXT("created")));
		Test.TestFalse(TEXT("A second no-overwrite atomic text write fails without hard links"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(TextPath, TEXT("changed"), true, false));
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(TextPath, TextReadBack);
		Test.TestEqual(TEXT("A refused no-overwrite write leaves the text file unchanged"), TextReadBack, FString(TEXT("created")));
		Test.TestTrue(TEXT("An overwriting atomic text write replaces the file"),
			UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(TextPath, TEXT("replaced"), true, true));
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(TextPath, TextReadBack);
		Test.TestEqual(TEXT("The overwritten text file holds the new text"), TextReadBack, FString(TEXT("replaced")));

		Test.TestEqual(TEXT("Atomic writes without hard links leave no temporary files"), CountTemporaryFiles(MountPath), 0);
	}
#endif
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

	const FString MixedEndingsPath = TestRoot / TEXT("MixedEndings.txt");
	TestTrue(TEXT("Mixed line endings write"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(MixedEndingsPath, TEXT("a\r\nb\rc\n\n \r\nd\r\n")));
	TestTrue(TEXT("Mixed line endings read with empty entries"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(MixedEndingsPath, Lines, true));
	TestEqual(TEXT("CRLF, CR, and LF each end one line"), Lines,
		TArray<FString>({ TEXT("a"), TEXT("b"), TEXT("c"), TEXT(""), TEXT(" "), TEXT("d") }));
	TestTrue(TEXT("Mixed line endings read without empty entries"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(MixedEndingsPath, Lines, false));
	TestEqual(TEXT("Excluding empty lines keeps whitespace lines"), Lines,
		TArray<FString>({ TEXT("a"), TEXT("b"), TEXT("c"), TEXT(" "), TEXT("d") }));

	const FString NewlineOnlyPath = TestRoot / TEXT("NewlineOnly.txt");
	UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(NewlineOnlyPath, TEXT("\n"));
	TestTrue(TEXT("A newline-only file reads as lines"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(NewlineOnlyPath, Lines, true));
	TestEqual(TEXT("A newline-only file has one empty line"), Lines, TArray<FString>({ TEXT("") }));
	UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(NewlineOnlyPath, Lines, false);
	TestEqual(TEXT("A newline-only file has no lines when empty lines are excluded"), Lines.Num(), 0);
	const FString EmptyTextPath = TestRoot / TEXT("Empty.txt");
	UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(EmptyTextPath, FString());
	TestTrue(TEXT("An empty file reads as lines"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(EmptyTextPath, Lines, true));
	TestEqual(TEXT("An empty file has no lines"), Lines.Num(), 0);

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
	TestTrue(TEXT("Appending to a UTF-16 file succeeds"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(UnicodeAppendPath, TEXT(" text")));
	TestEqual(TEXT("Appending to a UTF-16 file adds UTF-16 code units"),
		UDirectiveUtilFileSystemFunctionLibrary::GetFileSize(UnicodeAppendPath), static_cast<int64>(2 + 9 * 2));
	TestTrue(TEXT("UTF-16 appended text reads"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(UnicodeAppendPath, Appended));
	TestEqual(TEXT("UTF-16 appended text is exact"), Appended, FString(TEXT("wide text")));

	const TArray<uint8> Latin1Bytes = { 'c', 'a', 'f', 0xE9 };
	const FString Latin1AppendPath = TestRoot / TEXT("Latin1Append.txt");
	TestTrue(TEXT("A Latin-1 source file is created"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(Latin1AppendPath, Latin1Bytes));
	TestTrue(TEXT("Appending to a Latin-1 file succeeds"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(Latin1AppendPath, TEXT("!")));
	TArray<uint8> Latin1ReadBack;
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(Latin1AppendPath, Latin1ReadBack);
	TestEqual(TEXT("Appending keeps the original Latin-1 bytes"), Latin1ReadBack,
		TArray<uint8>({ 'c', 'a', 'f', 0xE9, '!' }));
	TestFalse(TEXT("Append refuses to create a missing parent when asked not to"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(
			TestRoot / TEXT("MissingAppendParent") / TEXT("Append.txt"), TEXT("data"), false));
	const FString CreatedParentAppendPath = TestRoot / TEXT("CreatedAppendParent") / TEXT("Append.txt");
	TestTrue(TEXT("Append creates a missing parent when asked to"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(CreatedParentAppendPath, TEXT("data"), true));
	TestTrue(TEXT("Append into a created parent stores the text"),
		UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(CreatedParentAppendPath, Appended)
		&& Appended == TEXT("data"));

	const FString BigEndianAppendPath = TestRoot / TEXT("BigEndianAppend.txt");
	TestTrue(TEXT("A big-endian UTF-16 source file is created"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(BigEndianAppendPath, { 0xFE, 0xFF, 0x00, 'h', 0x00, 'i' }));
	TestTrue(TEXT("Appending to a big-endian UTF-16 file succeeds"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(BigEndianAppendPath, TEXT("!")));
	TArray<uint8> BigEndianReadBack;
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(BigEndianAppendPath, BigEndianReadBack);
	TestEqual(TEXT("Big-endian UTF-16 text is appended in the same byte order"), BigEndianReadBack,
		TArray<uint8>({ 0xFE, 0xFF, 0x00, 'h', 0x00, 'i', 0x00, '!' }));

	const TArray<uint8> OddUtf16Bytes = { 0xFF, 0xFE, 'h', 0x00, 'i' };
	const FString OddUtf16AppendPath = TestRoot / TEXT("OddUtf16Append.txt");
	UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(OddUtf16AppendPath, OddUtf16Bytes);
	TestFalse(TEXT("Appending to a UTF-16 file with an odd byte count fails"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(OddUtf16AppendPath, TEXT("!")));
	TArray<uint8> OddUtf16ReadBack;
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(OddUtf16AppendPath, OddUtf16ReadBack);
	TestEqual(TEXT("A refused UTF-16 append leaves the file unchanged"), OddUtf16ReadBack, OddUtf16Bytes);

	const TArray<uint8> Utf32Bytes = { 0xFF, 0xFE, 0x00, 0x00, 'h', 0x00, 0x00, 0x00 };
	const FString Utf32AppendPath = TestRoot / TEXT("Utf32Append.txt");
	UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(Utf32AppendPath, Utf32Bytes);
	TestFalse(TEXT("Appending to a UTF-32 file fails"),
		UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(Utf32AppendPath, TEXT("!")));
	TArray<uint8> Utf32ReadBack;
	UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(Utf32AppendPath, Utf32ReadBack);
	TestEqual(TEXT("A refused UTF-32 append leaves the file unchanged"), Utf32ReadBack, Utf32Bytes);

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
	TestEqual(TEXT("Atomic writes leave no temporary files"), CountTemporaryFiles(TestRoot), 0);
	const FString DirectoryDestination = TestRoot / TEXT("DirectoryDestination");
	TestTrue(TEXT("A directory destination is created"), FileManager.MakeDirectory(*DirectoryDestination, true));
	TestFalse(TEXT("An atomic write cannot replace a directory"),
		UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(DirectoryDestination, TEXT("data")));
	TestTrue(TEXT("A failed atomic install keeps the directory"), FileManager.DirectoryExists(*DirectoryDestination));
	TestEqual(TEXT("A failed atomic install removes its temporary file"), CountTemporaryFiles(TestRoot), 0);

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

	const FString RelativePath = FString(TEXT("DirectiveUtilTests")) / TEXT("FileSystem") / TEXT("Relative.txt");
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

#if PLATFORM_MAC && WITH_EDITOR
	TestAtomicWritesWithoutHardLinks(*this);
#endif

	FileManager.DeleteDirectory(*TestRoot, false, true);
	return true;
}
