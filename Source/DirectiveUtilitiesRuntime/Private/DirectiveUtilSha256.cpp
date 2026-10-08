// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "DirectiveUtilSha256.h"

namespace
{
	constexpr uint32 RoundConstants[64] =
	{
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	uint32 RotateRight(uint32 Value, uint32 Count)
	{
		return (Value >> Count) | (Value << (32 - Count));
	}
}

FDirectiveUtilSha256::FDirectiveUtilSha256()
	: ByteCount(0)
	, PendingLength(0)
{
	State[0] = 0x6a09e667;
	State[1] = 0xbb67ae85;
	State[2] = 0x3c6ef372;
	State[3] = 0xa54ff53a;
	State[4] = 0x510e527f;
	State[5] = 0x9b05688c;
	State[6] = 0x1f83d9ab;
	State[7] = 0x5be0cd19;
}

void FDirectiveUtilSha256::Update(const uint8* Data, int64 Length)
{
	if (Data == nullptr || Length <= 0)
	{
		return;
	}

	ByteCount += static_cast<uint64>(Length);

	while (Length > 0)
	{
		const int32 CopyLength = FMath::Min<int64>(Length, 64 - PendingLength);
		FMemory::Memcpy(Pending + PendingLength, Data, static_cast<size_t>(CopyLength));
		PendingLength += CopyLength;
		Data += CopyLength;
		Length -= CopyLength;

		if (PendingLength == 64)
		{
			ProcessBlock(Pending);
			PendingLength = 0;
		}
	}
}

void FDirectiveUtilSha256::Final(uint8 OutDigest[32])
{
	const uint64 BitCount = ByteCount * 8;

	const uint8 EndOfMessage = 0x80;
	Update(&EndOfMessage, 1);

	static const uint8 Zeros[64] = { 0 };
	while (PendingLength != 56)
	{
		Update(Zeros, 1);
	}

	uint8 LengthBytes[8];
	for (int32 Index = 0; Index < 8; ++Index)
	{
		LengthBytes[Index] = static_cast<uint8>(BitCount >> (56 - Index * 8));
	}
	Update(LengthBytes, 8);

	for (int32 Index = 0; Index < 8; ++Index)
	{
		OutDigest[Index * 4 + 0] = static_cast<uint8>(State[Index] >> 24);
		OutDigest[Index * 4 + 1] = static_cast<uint8>(State[Index] >> 16);
		OutDigest[Index * 4 + 2] = static_cast<uint8>(State[Index] >> 8);
		OutDigest[Index * 4 + 3] = static_cast<uint8>(State[Index]);
	}
}

void FDirectiveUtilSha256::ProcessBlock(const uint8 Block[64])
{
	uint32 Schedule[64];
	for (int32 Index = 0; Index < 16; ++Index)
	{
		Schedule[Index] =
			(static_cast<uint32>(Block[Index * 4 + 0]) << 24) |
			(static_cast<uint32>(Block[Index * 4 + 1]) << 16) |
			(static_cast<uint32>(Block[Index * 4 + 2]) << 8) |
			static_cast<uint32>(Block[Index * 4 + 3]);
	}
	for (int32 Index = 16; Index < 64; ++Index)
	{
		const uint32 S0 = RotateRight(Schedule[Index - 15], 7) ^ RotateRight(Schedule[Index - 15], 18) ^ (Schedule[Index - 15] >> 3);
		const uint32 S1 = RotateRight(Schedule[Index - 2], 17) ^ RotateRight(Schedule[Index - 2], 19) ^ (Schedule[Index - 2] >> 10);
		Schedule[Index] = Schedule[Index - 16] + S0 + Schedule[Index - 7] + S1;
	}

	uint32 A = State[0];
	uint32 B = State[1];
	uint32 C = State[2];
	uint32 D = State[3];
	uint32 E = State[4];
	uint32 F = State[5];
	uint32 G = State[6];
	uint32 H = State[7];

	for (int32 Index = 0; Index < 64; ++Index)
	{
		const uint32 S1 = RotateRight(E, 6) ^ RotateRight(E, 11) ^ RotateRight(E, 25);
		const uint32 Choice = (E & F) ^ (~E & G);
		const uint32 Temp1 = H + S1 + Choice + RoundConstants[Index] + Schedule[Index];
		const uint32 S0 = RotateRight(A, 2) ^ RotateRight(A, 13) ^ RotateRight(A, 22);
		const uint32 Majority = (A & B) ^ (A & C) ^ (B & C);
		const uint32 Temp2 = S0 + Majority;

		H = G;
		G = F;
		F = E;
		E = D + Temp1;
		D = C;
		C = B;
		B = A;
		A = Temp1 + Temp2;
	}

	State[0] += A;
	State[1] += B;
	State[2] += C;
	State[3] += D;
	State[4] += E;
	State[5] += F;
	State[6] += G;
	State[7] += H;
}
