// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DirectiveUtilBytesFunctionLibrary.generated.h"

/** The lossless compression codecs available to byte-array helpers. */
UENUM(BlueprintType)
enum class EDirectiveUtilCompressionFormat : uint8
{
	Zlib UMETA(DisplayName = "Zlib", Tooltip = "Produces zlib-format bytes that other tools can read."),
	Gzip UMETA(DisplayName = "Gzip", Tooltip = "Produces gzip-format bytes that other tools can read."),
};

/**
 * Byte-array utilities for Blueprints: zlib and gzip compression, SHA-256 hashing, and
 * HMAC-SHA256 signing. The engine keeps all of these behind C++. The existing string
 * library covers MD5, SHA-1, CRC32, Base64, and hex.
 *
 * Compression stores no header describing the original size. Pair Compress Bytes with
 * Decompress Bytes by keeping the original byte count next to the compressed payload,
 * because decompression needs it up front.
 */
UCLASS()
class DIRECTIVEUTILITIESRUNTIME_API UDirectiveUtilBytesFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr int32 MaximumDecompressedByteCount = 1000000;

	/**
	 * Compresses a byte array with the requested codec. Inputs above
	 * MaximumDecompressedByteCount are rejected so every successful result can be restored by
	 * Decompress Bytes. Empty input produces a valid empty zlib or gzip stream.
	 *
	 * @param UncompressedData The bytes to compress.
	 * @param Format The codec to use.
	 * @param OutCompressedData Receives the compressed bytes, or an empty array on failure.
	 * @return `true` when the data was compressed. Incompressible input still succeeds; the result may be larger than the input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Bytes")
	static bool CompressBytes(const TArray<uint8>& UncompressedData, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutCompressedData);

	/**
	 * Decompresses bytes produced by Compress Bytes with the same codec.
	 *
	 * @param CompressedData The compressed bytes.
	 * @param ExpectedUncompressedSize The exact size of the original data in bytes. Values above MaximumDecompressedByteCount are rejected.
	 * @param Format The codec the data was compressed with.
	 * @param OutUncompressedData Receives the original bytes, or an empty array on failure.
	 * @return `true` when the data was decompressed to exactly `ExpectedUncompressedSize` bytes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Directive Utilities|Bytes")
	static bool DecompressBytes(const TArray<uint8>& CompressedData, const int32 ExpectedUncompressedSize, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutUncompressedData);

	/**
	 * Returns the SHA-256 digest of a byte array as 64 lowercase hex characters.
	 *
	 * @param Bytes The bytes to hash.
	 * @return The digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Bytes")
	static FString Sha256HashBytes(const TArray<uint8>& Bytes);

	/**
	 * Returns the SHA-256 digest of a string as 64 lowercase hex characters. The string is
	 * converted to UTF-8 bytes first.
	 *
	 * @param String The string to hash.
	 * @return The digest as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Bytes")
	static FString Sha256HashString(const FString& String);

	/**
	 * Returns the HMAC-SHA256 authentication code of a message for a secret key as 64
	 * lowercase hex characters. Use it to prove that a payload came from someone holding
	 * the same key, such as when signing web-service requests.
	 *
	 * @param Message The bytes to authenticate.
	 * @param Key The shared secret. Keys longer than 64 bytes are hashed down first.
	 * @return The authentication code as lowercase hex.
	 */
	UFUNCTION(BlueprintPure, Category = "Directive Utilities|Bytes")
	static FString HmacSha256Hex(const TArray<uint8>& Message, const TArray<uint8>& Key);
};
