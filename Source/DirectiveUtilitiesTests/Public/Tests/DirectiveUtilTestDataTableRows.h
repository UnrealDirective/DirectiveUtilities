// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Engine/DataTable.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
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

/** Enum class with sparse values, so a declared numeric value differs from its index. */
UENUM()
enum class EDirectiveUtilTestSparseRank : uint8
{
	Low = 1,
	High = 10,
};

/** Namespaced enumeration with sparse values for the TEnumAsByte import path. */
UENUM()
namespace EDirectiveUtilTestSparseTier
{
	enum Type : uint8
	{
		Low = 2,
		High = 20,
	};
}

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

/** Row struct with no properties, for imports that carry only row keys. */
USTRUCT()
struct FDirectiveUtilTestEmptyRow : public FTableRowBase
{
	GENERATED_BODY()
};

/** Row struct exercising floating-point round trips at the top level and inside a nested struct. */
USTRUCT()
struct FDirectiveUtilTestPrecisionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	float Single = 0.0f;

	UPROPERTY()
	double Double = 0.0;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;
};

/** Nested struct mixing quoted and bare member values for struct text round trips. */
USTRUCT()
struct FDirectiveUtilTestNestedValue
{
	GENERATED_BODY()

	UPROPERTY()
	FString Label;

	UPROPERTY()
	FText Caption;

	UPROPERTY()
	EDirectiveUtilTestRarity Rarity = EDirectiveUtilTestRarity::Common;

	UPROPERTY()
	float Scale = 0.0f;
};

/** Row struct exercising a nested property-text struct and a struct with its own native text format. */
USTRUCT()
struct FDirectiveUtilTestStructTextRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestNestedValue Nested;

	UPROPERTY()
	FGuid Id;
};

/** Row struct whose post-import callback reports a problem for negative values. */
USTRUCT()
struct FDirectiveUtilTestRejectingRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;

	virtual void OnPostDataImport(const UDataTable*, const FName RowName, TArray<FString>& OutCollectedImportProblems) override
	{
		if (Value < 0)
		{
			OutCollectedImportProblems.Add(FString::Printf(TEXT("Row %s has a negative value."), *RowName.ToString()));
		}
	}
};

/** Row struct holding a struct that can reference objects outside reflection. */
USTRUCT()
struct FDirectiveUtilTestInstancedStructRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FInstancedStruct Payload;
};

/** Row struct with an enum class property and a TEnumAsByte property, both with sparse values. */
USTRUCT()
struct FDirectiveUtilTestSparseEnumRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	EDirectiveUtilTestSparseRank Standing = EDirectiveUtilTestSparseRank::Low;

	UPROPERTY()
	TEnumAsByte<EDirectiveUtilTestSparseTier::Type> Grade = EDirectiveUtilTestSparseTier::Low;
};

/** Row struct whose post-import callback records the destination table it was given. */
USTRUCT()
struct FDirectiveUtilTestTableStateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;

	UPROPERTY()
	int32 TableRowCountDuringImport = -1;

	UPROPERTY()
	FName TableNameDuringImport = NAME_None;

	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName, TArray<FString>&) override
	{
		TableRowCountDuringImport = InDataTable != nullptr ? InDataTable->GetRowMap().Num() : -1;
		TableNameDuringImport = InDataTable != nullptr ? InDataTable->GetFName() : NAME_None;
	}
};

/** Row struct holding an object reference. */
USTRUCT()
struct FDirectiveUtilTestObjectReferenceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UObject> ObjectReference = nullptr;
};

/** Row struct holding a soft object pointer. */
USTRUCT()
struct FDirectiveUtilTestSoftReferenceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UObject> SoftReference = nullptr;
};

/** Row struct holding a set. */
USTRUCT()
struct FDirectiveUtilTestSetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	TSet<int32> Members;
};

/** Row struct holding a map. */
USTRUCT()
struct FDirectiveUtilTestMapRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, int32> Lookup;
};

DECLARE_DYNAMIC_DELEGATE(FDirectiveUtilTestRowDelegate);

/** Row struct holding a dynamic delegate. */
USTRUCT()
struct FDirectiveUtilTestDelegateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestRowDelegate Callback;
};

/** Row struct nesting an object reference row inside a struct member. */
USTRUCT()
struct FDirectiveUtilTestNestedObjectReferenceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestObjectReferenceRow Payload;
};

/** Row struct nesting a soft object pointer row inside a struct member. */
USTRUCT()
struct FDirectiveUtilTestNestedSoftReferenceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestSoftReferenceRow Payload;
};

/** Row struct nesting a set row inside a struct member. */
USTRUCT()
struct FDirectiveUtilTestNestedSetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestSetRow Payload;
};

/** Row struct nesting a map row inside a struct member. */
USTRUCT()
struct FDirectiveUtilTestNestedMapRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestMapRow Payload;
};

/** Row struct nesting a dynamic delegate row inside a struct member. */
USTRUCT()
struct FDirectiveUtilTestNestedDelegateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	FDirectiveUtilTestDelegateRow Payload;
};
