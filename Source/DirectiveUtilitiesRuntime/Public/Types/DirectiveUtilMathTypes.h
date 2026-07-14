// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "DirectiveUtilMathTypes.generated.h"

/**
 * Easing curves not provided by the engine's built-in Ease node (EEasingFunc): the classic Penner Back, Elastic and Bounce curves.
 */
UENUM(BlueprintType)
enum class EDirectiveUtilEaseType : uint8
{
	BackIn UMETA(DisplayName = "Back In", Tooltip="Overshoots slightly at the start before easing in."),
	BackOut UMETA(DisplayName = "Back Out", Tooltip="Overshoots slightly past the end before settling."),
	BackInOut UMETA(DisplayName = "Back In Out", Tooltip="Overshoots at both the start and the end."),
	ElasticIn UMETA(DisplayName = "Elastic In", Tooltip="Oscillates with increasing amplitude before easing in."),
	ElasticOut UMETA(DisplayName = "Elastic Out", Tooltip="Oscillates with decreasing amplitude after the end."),
	ElasticInOut UMETA(DisplayName = "Elastic In Out", Tooltip="Oscillates at both the start and the end."),
	BounceIn UMETA(DisplayName = "Bounce In", Tooltip="Bounces with increasing energy before easing in."),
	BounceOut UMETA(DisplayName = "Bounce Out", Tooltip="Bounces with decreasing energy after the end."),
	BounceInOut UMETA(DisplayName = "Bounce In Out", Tooltip="Bounces at both the start and the end."),
};
