#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilTypes.generated.h"

/**
 * Provides a list of success types.
 */
UENUM(BlueprintType)
enum class EDirectiveUtilSuccessStatus : uint8
{
	Success,
	Failure,
};