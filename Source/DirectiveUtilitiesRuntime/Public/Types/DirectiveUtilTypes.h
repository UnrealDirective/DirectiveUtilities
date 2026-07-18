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

UENUM(BlueprintType)
enum class EDirectiveUtilWorldType : uint8
{
	Unknown,
	None,
	Game,
	Editor,
	PlayInEditor UMETA(DisplayName = "Play In Editor"),
	EditorPreview UMETA(DisplayName = "Editor Preview"),
	GamePreview UMETA(DisplayName = "Game Preview"),
	GameRPC UMETA(DisplayName = "Game RPC"),
	Inactive,
};

UENUM(BlueprintType)
enum class EDirectiveUtilBuildConfiguration : uint8
{
	Unknown,
	Debug,
	DebugGame UMETA(DisplayName = "Debug Game"),
	Development,
	Shipping,
	Test,
};

UENUM(BlueprintType)
enum class EDirectiveUtilBuildTargetType : uint8
{
	Unknown,
	Game,
	Server,
	Client,
	Editor,
	Program,
};
