// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DirectiveUtilTestDataTableRows.generated.h"

/** Rarity values covering both the native enum and byte-backed enum import paths. */
UENUM()
enum class EDirectiveUtilTestRarity : uint8
{
	Common,
	Rare,
	Legendary,
};

UENUM()
enum EDirectiveUtilTestLegacyRarity : uint8
{
	LegacyCommon,
	LegacyRare,
};

/** Row struct exercising numeric, Boolean, string, enumeration, and text imports for runtime DataTable tests. */
USTRUCT()
struct FDirectiveUtilTestInventoryRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Quantity = 0;

	UPROPERTY()
	float Weight = 0.0f;

	UPROPERTY()
	bool bEnchanted = false;

	UPROPERTY()
	FString Label;

	// Declared after a heap-allocating member so a write at the row base address corrupts it.
	UPROPERTY()
	EDirectiveUtilTestRarity Rarity = EDirectiveUtilTestRarity::Common;

	UPROPERTY()
	FText Description;
};

/** Row struct exercising struct-property text format for runtime DataTable tests. */
USTRUCT()
struct FDirectiveUtilTestWaypointRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FName WaypointName;
};

USTRUCT()
struct FDirectiveUtilTestNumericRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	int8 Signed8 = 0;

	UPROPERTY()
	int16 Signed16 = 0;

	UPROPERTY()
	int64 Signed64 = 0;

	UPROPERTY()
	uint8 Unsigned8 = 0;

	UPROPERTY()
	uint16 Unsigned16 = 0;

	UPROPERTY()
	uint32 Unsigned32 = 0;

	UPROPERTY()
	uint64 Unsigned64 = 0;

	UPROPERTY()
	float Single = 0.0f;

	UPROPERTY()
	double Double = 0.0;

	UPROPERTY()
	TEnumAsByte<EDirectiveUtilTestLegacyRarity> LegacyRarity = LegacyCommon;
};

USTRUCT()
struct FDirectiveUtilTestNestedCollection
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> Values;
};

USTRUCT()
struct FDirectiveUtilTestNestedCollectionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestNestedCollection Payload;
};

USTRUCT()
struct FDirectiveUtilTestNameCollisionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FDirectiveUtilTestFixedArrayRow : public FTableRowBase
{
	GENERATED_BODY()

	FDirectiveUtilTestFixedArrayRow()
	{
		FixedValues[0] = 0;
		FixedValues[1] = 0;
	}

	UPROPERTY()
	int32 FixedValues[2];
};

USTRUCT()
struct FDirectiveUtilTestLifecycleRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;

	UPROPERTY()
	bool bPostImported = false;

	UPROPERTY()
	bool bTableChanged = false;

	virtual void OnPostDataImport(const UDataTable*, const FName, TArray<FString>&) override
	{
		bPostImported = true;
	}

	virtual void OnDataTableChanged(const UDataTable*, const FName) override
	{
		bTableChanged = true;
	}
};
