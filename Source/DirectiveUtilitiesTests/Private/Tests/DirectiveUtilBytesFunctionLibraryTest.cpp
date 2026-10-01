// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilBytesFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilBytesFunctionLibraryTest,
	"DirectiveUtilities.BytesFunctionLibraryTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilBytesFunctionLibraryTest::RunTest(const FString& Parameters)
{
	const TArray<uint8> Empty;
	TestEqual(TEXT("SHA-256 of empty input matches the known digest"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(Empty),
		FString(TEXT("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")));

	const TArray<uint8> Abc = { 'a', 'b', 'c' };
	TestEqual(TEXT("SHA-256 of 'abc' matches the NIST vector"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(Abc),
		FString(TEXT("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")));

	// 55, 56, and 64 bytes straddle the point where the length field forces an extra block.
	const TCHAR* PaddingDigests[] = {
		TEXT("9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318"),
		TEXT("b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a"),
		TEXT("ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb"),
	};
	const int32 PaddingLengths[] = { 55, 56, 64 };
	for (int32 Case = 0; Case < UE_ARRAY_COUNT(PaddingLengths); ++Case)
	{
		TArray<uint8> Block;
		Block.Init(static_cast<uint8>('a'), PaddingLengths[Case]);
		TestEqual(*FString::Printf(TEXT("SHA-256 padding boundary at %d bytes"), PaddingLengths[Case]),
			UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(Block), FString(PaddingDigests[Case]));
	}

	TestEqual(TEXT("SHA-256 of a string hashes its UTF-8 bytes"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashString(TEXT("abc")),
		FString(TEXT("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")));
	TestEqual(TEXT("SHA-256 of an empty string matches the empty digest"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashString(FString()),
		FString(TEXT("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")));
	const TCHAR EmbeddedNullCharacters[] = { TEXT('a'), TEXT('\0'), TEXT('b') };
	const FString StringWithNull = FString::ConstructFromPtrSize(EmbeddedNullCharacters, 3);
	TestEqual(TEXT("SHA-256 hashes embedded null bytes"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashString(StringWithNull),
		FString(TEXT("59b271ae1bbcb1d31d41929817f4b16fb439eb4f31520b5ad1d5ce98920a7138")));

	TArray<uint8> LongInput;
	LongInput.SetNumUninitialized(1000);
	for (int32 Index = 0; Index < LongInput.Num(); ++Index)
	{
		LongInput[Index] = static_cast<uint8>(Index % 251);
	}
	const FString FirstHash = UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(LongInput);
	TestEqual(TEXT("SHA-256 output is 64 hex characters"), FirstHash.Len(), 64);

	TArray<uint8> Mutated = LongInput;
	Mutated[500] ^= 0xFF;
	TestNotEqual(TEXT("Changing one bit changes the digest"),
		UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(Mutated), FirstHash);

	const TArray<uint8> Key = { 'k', 'e', 'y' };
	TArray<uint8> Message;
	const FString FoxMessage = TEXT("The quick brown fox jumps over the lazy dog");
	for (const TCHAR Character : FoxMessage)
	{
		Message.Add(static_cast<uint8>(Character));
	}
	TestEqual(TEXT("HMAC-SHA256 matches the reference key/fox digest"),
		UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(Message, Key),
		FString(TEXT("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8")));

	TArray<uint8> Rfc4231Key;
	Rfc4231Key.Init(0x0b, 20);
	TArray<uint8> HiThere;
	for (const TCHAR Character : FString(TEXT("Hi There")))
	{
		HiThere.Add(static_cast<uint8>(Character));
	}
	TestEqual(TEXT("HMAC-SHA256 matches RFC 4231 case 1"),
		UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(HiThere, Rfc4231Key),
		FString(TEXT("b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7")));

	// Case 6 uses a 131-byte key, which must be reduced by hashing rather than truncated.
	TArray<uint8> OversizedKey;
	OversizedKey.Init(0xaa, 131);
	TArray<uint8> LargerThanBlock;
	for (const TCHAR Character : FString(TEXT("Test Using Larger Than Block-Size Key - Hash Key First")))
	{
		LargerThanBlock.Add(static_cast<uint8>(Character));
	}
	TestEqual(TEXT("HMAC-SHA256 matches RFC 4231 case 6"),
		UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(LargerThanBlock, OversizedKey),
		FString(TEXT("60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54")));

	TestEqual(TEXT("HMAC-SHA256 accepts an empty key and message"),
		UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(Empty, Empty),
		FString(TEXT("b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad")));

	TArray<uint8> BlockSizedKey;
	for (int32 Index = 0; Index < 64; ++Index)
	{
		BlockSizedKey.Add(static_cast<uint8>(Index));
	}
	const TArray<uint8> ReferenceMessage = {
		'H', 'e', 'l', 'l', 'o', ',', ' ', 'D', 'i', 'r', 'e', 'c', 't', 'i', 'v', 'e', ' ',
		'U', 't', 'i', 'l', 'i', 't', 'i', 'e', 's', '!' };
	// Computed with Python's hmac module; a 64-byte key fills the block exactly and is used without hashing.
	TestEqual(TEXT("HMAC-SHA256 matches a reference digest for a block-sized key"),
		UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(ReferenceMessage, BlockSizedKey),
		FString(TEXT("5ff68dcabfbacd294703358e475843f1b5e5840814955ef13009ef1c30f3079d")));

	// Produced by Python's zlib.compress and gzip.compress(mtime=0) at level 9 from ReferenceMessage.
	const TArray<uint8> ExternalZlib = {
		0x78, 0xda, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x70, 0xc9, 0x2c, 0x4a, 0x4d, 0x2e, 0xc9,
		0x2c, 0x4b, 0x55, 0x08, 0x2d, 0xc9, 0xcc, 0xc9, 0x2c, 0xc9, 0x4c, 0x2d, 0x56, 0x04, 0x00, 0x87,
		0xe0, 0x09, 0xdd };
	const TArray<uint8> ExternalGzip = {
		0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7,
		0x51, 0x70, 0xc9, 0x2c, 0x4a, 0x4d, 0x2e, 0xc9, 0x2c, 0x4b, 0x55, 0x08, 0x2d, 0xc9, 0xcc, 0xc9,
		0x2c, 0xc9, 0x4c, 0x2d, 0x56, 0x04, 0x00, 0x49, 0xb6, 0xa0, 0xcd, 0x1b, 0x00, 0x00, 0x00 };
	TArray<uint8> ExternalOutput;
	TestTrue(TEXT("An externally produced zlib stream decompresses"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ExternalZlib, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Zlib, ExternalOutput));
	TestEqual(TEXT("The external zlib stream restores the reference bytes"), ExternalOutput, ReferenceMessage);
	TestTrue(TEXT("An externally produced gzip stream decompresses"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ExternalGzip, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Gzip, ExternalOutput));
	TestEqual(TEXT("The external gzip stream restores the reference bytes"), ExternalOutput, ReferenceMessage);
	TestFalse(TEXT("A zlib stream decoded as gzip fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ExternalZlib, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Gzip, ExternalOutput));
	TestEqual(TEXT("A format mismatch leaves no output"), ExternalOutput.Num(), 0);
	TestFalse(TEXT("A gzip stream decoded as zlib fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ExternalGzip, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Zlib, ExternalOutput));
	TestFalse(TEXT("An expected size smaller than the stream output fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ExternalGzip, ReferenceMessage.Num() - 1, EDirectiveUtilCompressionFormat::Gzip, ExternalOutput));
	TestEqual(TEXT("A short expected size leaves no output"), ExternalOutput.Num(), 0);
	TArray<uint8> CorruptCrcGzip = ExternalGzip;
	CorruptCrcGzip[CorruptCrcGzip.Num() - 8] ^= 0x01;
	TestFalse(TEXT("A gzip trailer with a wrong CRC fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			CorruptCrcGzip, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Gzip, ExternalOutput));
	TestEqual(TEXT("A CRC failure leaves no output"), ExternalOutput.Num(), 0);
	TArray<uint8> CorruptAdlerZlib = ExternalZlib;
	CorruptAdlerZlib.Last() ^= 0x01;
	TestFalse(TEXT("A zlib trailer with a wrong Adler-32 fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			CorruptAdlerZlib, ReferenceMessage.Num(), EDirectiveUtilCompressionFormat::Zlib, ExternalOutput));

	TArray<uint8> OwnGzip;
	TestTrue(TEXT("Gzip compression succeeds for the reference bytes"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(ReferenceMessage, EDirectiveUtilCompressionFormat::Gzip, OwnGzip));
	TestTrue(TEXT("Gzip output starts with the gzip magic bytes and deflate method"),
		OwnGzip.Num() >= 18 && OwnGzip[0] == 0x1f && OwnGzip[1] == 0x8b && OwnGzip[2] == 0x08);

	const FString RepetitiveText = TEXT("Directive Utilities Directive Utilities Directive Utilities Directive Utilities");
	TArray<uint8> RepetitiveBytes;
	for (const TCHAR Character : RepetitiveText)
	{
		RepetitiveBytes.Add(static_cast<uint8>(Character));
	}

	for (const EDirectiveUtilCompressionFormat Format :
		{ EDirectiveUtilCompressionFormat::Zlib, EDirectiveUtilCompressionFormat::Gzip })
	{
		TArray<uint8> Compressed;
		TestTrue(TEXT("Compression succeeds"),
			UDirectiveUtilBytesFunctionLibrary::CompressBytes(RepetitiveBytes, Format, Compressed));
		TestTrue(TEXT("Repetitive text shrinks"), Compressed.Num() > 0 && Compressed.Num() < RepetitiveBytes.Num());

		TArray<uint8> Decompressed;
		TestTrue(TEXT("Decompression restores the original size"),
			UDirectiveUtilBytesFunctionLibrary::DecompressBytes(Compressed, RepetitiveBytes.Num(), Format, Decompressed));
		TestEqual(TEXT("Decompression restores the exact bytes"), Decompressed, RepetitiveBytes);

		TArray<uint8> Aliased = RepetitiveBytes;
		TestTrue(TEXT("Compression supports aliased input and output"),
			UDirectiveUtilBytesFunctionLibrary::CompressBytes(Aliased, Format, Aliased));
		TestTrue(TEXT("Decompression supports aliased input and output"),
			UDirectiveUtilBytesFunctionLibrary::DecompressBytes(Aliased, RepetitiveBytes.Num(), Format, Aliased));
		TestEqual(TEXT("Aliased compression round-trips"), Aliased, RepetitiveBytes);

	}

	TArray<uint8> Incompressible;
	Incompressible.SetNumUninitialized(64);
	for (int32 Index = 0; Index < Incompressible.Num(); ++Index)
	{
		Incompressible[Index] = static_cast<uint8>((Index * 37 + 11) & 0xFF);
	}
	TArray<uint8> CompressedIncompressible;
	TestTrue(TEXT("Incompressible input still compresses successfully"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(Incompressible, EDirectiveUtilCompressionFormat::Zlib, CompressedIncompressible));
	TArray<uint8> Restored;
	TestTrue(TEXT("Incompressible input round-trips"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(CompressedIncompressible, Incompressible.Num(), EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestEqual(TEXT("The incompressible bytes are identical"), Restored, Incompressible);

	TArray<uint8> Nothing;
	TArray<uint8> NothingOut;
	TestTrue(TEXT("Empty input compresses"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(Nothing, EDirectiveUtilCompressionFormat::Zlib, NothingOut));
	TestTrue(TEXT("Compressed empty input is a real zlib stream"), NothingOut.Num() > 0);
	TestTrue(TEXT("A compressed empty stream decompresses"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(NothingOut, 0, EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestEqual(TEXT("The decompressed empty stream has no bytes"), Restored.Num(), 0);
	TestFalse(TEXT("An empty compressed payload is rejected"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(Nothing, 0, EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestFalse(TEXT("An empty stream with a nonzero expected size fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(NothingOut, 10, EDirectiveUtilCompressionFormat::Zlib, Restored));

	TArray<uint8> Garbage;
	Garbage.Init(0x5A, 32);
	TestFalse(TEXT("Decompressing a malformed payload fails cleanly"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(Garbage, 64, EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestEqual(TEXT("Malformed decompression leaves no output"), Restored.Num(), 0);

	TArray<uint8> ValidCompressed;
	UDirectiveUtilBytesFunctionLibrary::CompressBytes(Incompressible, EDirectiveUtilCompressionFormat::Zlib, ValidCompressed);
	TestFalse(TEXT("A mismatched uncompressed size fails"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(ValidCompressed, Incompressible.Num() + 16, EDirectiveUtilCompressionFormat::Zlib, Restored));
	ValidCompressed.Add(0x00);
	TestFalse(TEXT("Trailing compressed bytes are rejected"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(ValidCompressed, Incompressible.Num(), EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestFalse(TEXT("Decompression rejects an output above the allocation limit"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			ValidCompressed,
			UDirectiveUtilBytesFunctionLibrary::MaximumDecompressedByteCount + 1,
			EDirectiveUtilCompressionFormat::Zlib,
			Restored));

	TArray<uint8> MaximumInput;
	MaximumInput.Init(0xA5, UDirectiveUtilBytesFunctionLibrary::MaximumDecompressedByteCount);
	TArray<uint8> MaximumCompressed;
	TestTrue(TEXT("Compression accepts the exact size limit"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(
			MaximumInput, EDirectiveUtilCompressionFormat::Zlib, MaximumCompressed));
	TestTrue(TEXT("The exact size limit decompresses"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(
			MaximumCompressed, MaximumInput.Num(), EDirectiveUtilCompressionFormat::Zlib, Restored));
	TestEqual(TEXT("The exact size limit round-trips"), Restored, MaximumInput);
	MaximumInput.Add(0xA5);
	TestFalse(TEXT("Compression rejects input above the round-trip limit"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(
			MaximumInput, EDirectiveUtilCompressionFormat::Zlib, MaximumCompressed));

	const EDirectiveUtilCompressionFormat InvalidFormat = static_cast<EDirectiveUtilCompressionFormat>(255);
	TestFalse(TEXT("Compression rejects an invalid codec"),
		UDirectiveUtilBytesFunctionLibrary::CompressBytes(Incompressible, InvalidFormat, MaximumCompressed));
	TestFalse(TEXT("Decompression rejects an invalid codec"),
		UDirectiveUtilBytesFunctionLibrary::DecompressBytes(CompressedIncompressible, Incompressible.Num(), InvalidFormat, Restored));

	return true;
}
