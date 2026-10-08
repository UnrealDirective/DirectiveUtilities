// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilBytesFunctionLibrary.h"
#include "DirectiveUtilSha256.h"
#include "zlib.h"

namespace
{
	bool GetWindowBits(const EDirectiveUtilCompressionFormat Format, int32& OutWindowBits)
	{
		switch (Format)
		{
			case EDirectiveUtilCompressionFormat::Zlib:
				OutWindowBits = MAX_WBITS;
				return true;
			case EDirectiveUtilCompressionFormat::Gzip:
				OutWindowBits = MAX_WBITS + 16;
				return true;
			default:
				return false;
		}
	}

	void FinalizeSha256(FDirectiveUtilSha256& Hasher, uint8 (&OutDigest)[32])
	{
		Hasher.Final(OutDigest);
	}

	void ComputeSha256(const TArray<uint8>& Bytes, uint8 (&OutDigest)[32])
	{
		FDirectiveUtilSha256 Hasher;
		Hasher.Update(Bytes.GetData(), Bytes.Num());
		FinalizeSha256(Hasher, OutDigest);
	}
}

bool UDirectiveUtilBytesFunctionLibrary::CompressBytes(const TArray<uint8>& UncompressedData, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutCompressedData)
{
	int32 WindowBits = 0;
	if (!GetWindowBits(Format, WindowBits)
		|| UncompressedData.Num() > MaximumDecompressedByteCount)
	{
		OutCompressedData.Reset();
		return false;
	}

	z_stream Stream = {};
	if (deflateInit2(&Stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, WindowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		OutCompressedData.Reset();
		return false;
	}

	const uLong Bound = deflateBound(&Stream, static_cast<uLong>(UncompressedData.Num()));
	if (Bound == 0 || Bound > static_cast<uLong>(MAX_int32))
	{
		deflateEnd(&Stream);
		OutCompressedData.Reset();
		return false;
	}

	TArray<uint8> CompressedBuffer;
	CompressedBuffer.SetNumUninitialized(static_cast<int32>(Bound));
	Stream.next_in = UncompressedData.IsEmpty()
		? Z_NULL
		: const_cast<Bytef*>(reinterpret_cast<const Bytef*>(UncompressedData.GetData()));
	Stream.avail_in = static_cast<uInt>(UncompressedData.Num());
	Stream.next_out = reinterpret_cast<Bytef*>(CompressedBuffer.GetData());
	Stream.avail_out = static_cast<uInt>(CompressedBuffer.Num());

	const int32 Result = deflate(&Stream, Z_FINISH);
	const uLong CompressedSize = Stream.total_out;
	const uInt RemainingInput = Stream.avail_in;
	const int32 EndResult = deflateEnd(&Stream);
	const bool bSucceeded = Result == Z_STREAM_END
		&& RemainingInput == 0
		&& CompressedSize <= Bound
		&& EndResult == Z_OK;
	if (!bSucceeded)
	{
		OutCompressedData.Reset();
		return false;
	}

	CompressedBuffer.SetNum(static_cast<int32>(CompressedSize), EAllowShrinking::No);
	OutCompressedData = MoveTemp(CompressedBuffer);
	return true;
}

bool UDirectiveUtilBytesFunctionLibrary::DecompressBytes(const TArray<uint8>& CompressedData, const int32 ExpectedUncompressedSize, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutUncompressedData)
{
	int32 WindowBits = 0;
	if (!GetWindowBits(Format, WindowBits))
	{
		OutUncompressedData.Reset();
		return false;
	}
	if (CompressedData.IsEmpty())
	{
		OutUncompressedData.Reset();
		return false;
	}
	if (ExpectedUncompressedSize < 0 || ExpectedUncompressedSize > MaximumDecompressedByteCount)
	{
		OutUncompressedData.Reset();
		return false;
	}

	TArray<uint8> UncompressedBuffer;
	UncompressedBuffer.SetNumUninitialized(ExpectedUncompressedSize);
	uint8 EmptyOutput = 0;

	z_stream Stream = {};
	Stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(CompressedData.GetData()));
	Stream.avail_in = static_cast<uInt>(CompressedData.Num());
	Stream.next_out = ExpectedUncompressedSize == 0
		? &EmptyOutput
		: reinterpret_cast<Bytef*>(UncompressedBuffer.GetData());
	Stream.avail_out = ExpectedUncompressedSize == 0 ? 1 : static_cast<uInt>(UncompressedBuffer.Num());

	if (inflateInit2(&Stream, WindowBits) != Z_OK)
	{
		OutUncompressedData.Reset();
		return false;
	}

	const int32 Result = inflate(&Stream, Z_FINISH);
	const uLong ConsumedSize = Stream.total_in;
	const uLong UncompressedSize = Stream.total_out;
	const int32 EndResult = inflateEnd(&Stream);
	const bool bSucceeded = Result == Z_STREAM_END
		&& UncompressedSize == static_cast<uLong>(ExpectedUncompressedSize)
		&& ConsumedSize == static_cast<uLong>(CompressedData.Num())
		&& EndResult == Z_OK;
	if (!bSucceeded)
	{
		OutUncompressedData.Reset();
		return false;
	}

	OutUncompressedData = MoveTemp(UncompressedBuffer);
	return true;
}

FString UDirectiveUtilBytesFunctionLibrary::Sha256HashBytes(const TArray<uint8>& Bytes)
{
	uint8 Digest[32];
	ComputeSha256(Bytes, Digest);
	return BytesToHexLower(Digest, UE_ARRAY_COUNT(Digest));
}

FString UDirectiveUtilBytesFunctionLibrary::Sha256HashString(const FString& String)
{
	const FTCHARToUTF8 Utf8(*String, String.Len());
	FDirectiveUtilSha256 Hasher;
	Hasher.Update(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	uint8 Digest[32];
	FinalizeSha256(Hasher, Digest);
	return BytesToHexLower(Digest, UE_ARRAY_COUNT(Digest));
}

FString UDirectiveUtilBytesFunctionLibrary::HmacSha256Hex(const TArray<uint8>& Message, const TArray<uint8>& Key)
{
	constexpr int32 BlockSize = 64;

	TArray<uint8> KeyBlock;
	KeyBlock.SetNumZeroed(BlockSize);

	if (Key.Num() > BlockSize)
	{
		uint8 HashedKey[32];
		ComputeSha256(Key, HashedKey);
		FMemory::Memcpy(KeyBlock.GetData(), HashedKey, UE_ARRAY_COUNT(HashedKey));
	}
	else
	{
		if (!Key.IsEmpty())
		{
			FMemory::Memcpy(KeyBlock.GetData(), Key.GetData(), Key.Num());
		}
	}

	TArray<uint8> InnerPad;
	TArray<uint8> OuterPad;
	InnerPad.SetNumUninitialized(BlockSize);
	OuterPad.SetNumUninitialized(BlockSize);
	for (int32 Index = 0; Index < BlockSize; ++Index)
	{
		InnerPad[Index] = KeyBlock[Index] ^ 0x36;
		OuterPad[Index] = KeyBlock[Index] ^ 0x5c;
	}

	uint8 InnerDigest[32];
	FDirectiveUtilSha256 InnerHasher;
	InnerHasher.Update(InnerPad.GetData(), InnerPad.Num());
	InnerHasher.Update(Message.GetData(), Message.Num());
	FinalizeSha256(InnerHasher, InnerDigest);

	uint8 OuterDigest[32];
	FDirectiveUtilSha256 OuterHasher;
	OuterHasher.Update(OuterPad.GetData(), OuterPad.Num());
	OuterHasher.Update(InnerDigest, UE_ARRAY_COUNT(InnerDigest));
	FinalizeSha256(OuterHasher, OuterDigest);

	return BytesToHexLower(OuterDigest, UE_ARRAY_COUNT(OuterDigest));
}
