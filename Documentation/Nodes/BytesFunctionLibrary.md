# Bytes Function Library

> Zlib and gzip compression, SHA-256 hashing, and HMAC-SHA256 signing for byte arrays. The engine keeps all of these behind C++ or, for SHA-256, behind a platform-gated implementation that asserts on Mac and Linux.

The existing [String library](StringFunctionLibrary.md) covers MD5, SHA-1, CRC32, Base64, and hex. This library adds the pieces needed to shrink save payloads and sign web-service requests.

Compression stores no header describing the original size. Keep the original byte count next to the compressed payload. Uncompressed input and output are limited to 1,000,000 bytes. Decompression requires the exact size, consumes the complete compressed stream, and fails without partial output when the payload, codec, size, or trailing data is invalid.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilBytesFunctionLibrary.h`

---

## Compress Bytes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Bytes`

```cpp
static bool CompressBytes(const TArray<uint8>& UncompressedData, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutCompressedData);
```

Compresses a byte array with the requested codec. Empty input produces a valid empty zlib or gzip stream. Incompressible input still succeeds and may produce more bytes than the input. Inputs above 1,000,000 bytes are rejected.

| Parameter | Type | Description |
|-----------|------|-------------|
| UncompressedData | `const TArray<uint8>&` | The bytes to compress. |
| Format | `const EDirectiveUtilCompressionFormat` | `Zlib` or `Gzip`. Both produce standard-format bytes other tools can read. |
| OutCompressedData | `TArray<uint8>&` | [out] The compressed bytes, or an empty array on failure. |

**Returns:** True when the data was compressed into a complete standard-format stream.

## Decompress Bytes
**Type:** Blueprint Callable &nbsp;|&nbsp; **Category:** `Directive Utilities|Bytes`

```cpp
static bool DecompressBytes(const TArray<uint8>& CompressedData, const int32 ExpectedUncompressedSize, const EDirectiveUtilCompressionFormat Format, TArray<uint8>& OutUncompressedData);
```

Decompresses a zlib or gzip stream, including one written by another tool such as Python's `zlib` and `gzip` modules. `Format` must match the stream: zlib bytes read as `Gzip`, or gzip bytes read as `Zlib`, fail. A stream whose CRC-32 (gzip) or Adler-32 (zlib) trailer does not match the data fails.

| Parameter | Type | Description |
|-----------|------|-------------|
| CompressedData | `const TArray<uint8>&` | The compressed bytes. |
| ExpectedUncompressedSize | `const int32` | The exact size of the original data in bytes, up to 1,000,000. |
| Format | `const EDirectiveUtilCompressionFormat` | The codec the data was compressed with. |
| OutUncompressedData | `TArray<uint8>&` | [out] The original bytes, or an empty array on failure. |

**Returns:** True when the complete payload was decompressed to exactly `ExpectedUncompressedSize` bytes. An empty compressed array, an expected size larger or smaller than the real output, a corrupt payload or checksum, trailing data, or output above the limit fails without partial output.

## SHA 256 Hash Bytes
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Bytes`

```cpp
static FString Sha256HashBytes(const TArray<uint8>& Bytes);
```

**Returns:** The FIPS 180-4 digest as 64 lowercase hex characters. Verified against the published NIST vectors by the automation tests.

## SHA 256 Hash String
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Bytes`

```cpp
static FString Sha256HashString(const FString& String);
```

The string is converted to UTF-8 bytes first.

**Returns:** The digest as lowercase hex.

## HMAC-SHA-256 Hex
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|Bytes`

```cpp
static FString HmacSha256Hex(const TArray<uint8>& Message, const TArray<uint8>& Key);
```

Computes the HMAC-SHA256 authentication code of a message for a secret key (RFC 2104). Use it to authenticate payloads such as web-service requests. Keys longer than 64 bytes are hashed first. A key of exactly 64 bytes is used as is. Shorter keys, including an empty key, are zero-padded to the block size.

| Parameter | Type | Description |
|-----------|------|-------------|
| Message | `const TArray<uint8>&` | The bytes to authenticate. |
| Key | `const TArray<uint8>&` | The shared secret. |

**Returns:** The authentication code as 64 lowercase hex characters.
