// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

/**
 * FIPS 180-4 SHA-256 over streamed input. The engine's GetSHA256Signature is gated to
 * platforms that ship their own implementation and asserts elsewhere, so the plugin carries
 * its own. Correctness is pinned by the automation tests against the published vectors.
 */
class FDirectiveUtilSha256
{
public:

	FDirectiveUtilSha256();

	/** Feeds more bytes into the digest. */
	void Update(const uint8* Data, int64 Length);

	/** Completes the digest and writes exactly 32 bytes. The instance must not be reused afterward. */
	void Final(uint8 OutDigest[32]);

private:

	void ProcessBlock(const uint8 Block[64]);

	uint32 State[8];
	uint64 ByteCount;
	uint8 Pending[64];
	int32 PendingLength;
};
