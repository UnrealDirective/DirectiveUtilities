// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilCsvTypes.generated.h"

/** The field separator used by a CSV document. */
UENUM(BlueprintType)
enum class EDirectiveUtilCsvDelimiter : uint8
{
	Comma UMETA(DisplayName = "Comma"),
	Semicolon UMETA(DisplayName = "Semicolon"),
	Tab UMETA(DisplayName = "Tab"),
};

/** One row of a CSV document. Cells keep their parsed text without quotes or separators. */
USTRUCT(BlueprintType)
struct FDirectiveUtilCsvRow
{
	GENERATED_BODY()

	/** The cells of the row, in column order. Rows may have different lengths when the input is ragged. Empty cell arrays serialize as one empty cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSV")
	TArray<FString> Cells;
};

/**
 * A parsed CSV document held as rows of cells. The delimiter is stored so a round-trip
 * through Write CSV keeps the original separator unless another one is requested.
 */
USTRUCT(BlueprintType)
struct FDirectiveUtilCsvDocument
{
	GENERATED_BODY()

	/** The rows of the document, in input order. Empty documents have no rows. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSV")
	TArray<FDirectiveUtilCsvRow> Rows;

	/** The delimiter the document was parsed with, and the default for writing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CSV")
	EDirectiveUtilCsvDelimiter Delimiter = EDirectiveUtilCsvDelimiter::Comma;
};

USTRUCT(BlueprintType)
struct FDirectiveUtilCsvDiff
{
	GENERATED_BODY()

	/** Keys found only in the later document, sorted ascending. */
	UPROPERTY(BlueprintReadOnly, Category = "CSV")
	TArray<FString> AddedKeys;

	/** Keys found only in the earlier document, sorted ascending. */
	UPROPERTY(BlueprintReadOnly, Category = "CSV")
	TArray<FString> RemovedKeys;

	/** Keys whose non-key cells differ, sorted ascending. */
	UPROPERTY(BlueprintReadOnly, Category = "CSV")
	TArray<FString> ChangedKeys;
};
