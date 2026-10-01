// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

namespace DirectiveUtil
{
	/**
	 * Resolves a caller-supplied path to an absolute path. Absolute paths pass through unchanged;
	 * relative paths resolve against the project Saved directory.
	 */
	FString ResolveRuntimePath(const FString& Path);

	/**
	 * Converts a UTC timestamp to local wall-clock time using the timezone rules in effect at that instant.
	 * Returns the input unchanged when platform time routines fail.
	 */
	FDateTime UtcToLocal(const FDateTime& UtcTimestamp);

	/**
	 * Converts local wall-clock time to a UTC timestamp using the timezone rules in effect at that instant.
	 * Returns the input unchanged when platform time routines fail.
	 */
	FDateTime LocalToUtc(const FDateTime& LocalDateTime);

	/** Converts like UtcToLocal. Returns false and leaves the output untouched when the conversion fails. */
	bool TryUtcToLocal(const FDateTime& UtcTimestamp, FDateTime& OutLocalDateTime);

	/** Converts like LocalToUtc. Returns false and leaves the output untouched when the conversion fails. */
	bool TryLocalToUtc(const FDateTime& LocalDateTime, FDateTime& OutUtcDateTime);
}
