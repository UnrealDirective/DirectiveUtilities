#!/usr/bin/env bash

set -euo pipefail

SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_ROOT/../../.." && pwd)"

require_symbol() {
	local symbol="$1"
	local file="$2"
	if ! grep -Eq "static .*${symbol}\\(" "$REPOSITORY_ROOT/$file"; then
		echo "Missing local-data API: $symbol" >&2
		exit 1
	fi
}

for symbol in WriteTextFileAtomic WriteBinaryFileAtomic; do
	require_symbol "$symbol" "Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
done

for symbol in ReadTextFileAsync ReadBinaryFileAsync WriteTextFileAsync WriteBinaryFileAsync WatchFile; do
	require_symbol "$symbol" "Source/DirectiveUtilitiesRuntime/Public/Tasks/DirectiveUtilTask_FileSystem.h"
done

if grep -Eq '"DirectoryWatcher"' "$REPOSITORY_ROOT/Source/DirectiveUtilitiesRuntime/DirectiveUtilitiesRuntime.Build.cs"; then
	echo "Runtime file watching must not depend on Unreal's developer-only DirectoryWatcher module" >&2
	exit 1
fi

for symbol in FindCsvColumn UpsertCsvRowByKey ValidateCsvHeaders ValidateCsvShape DiffCsvByKey; do
	require_symbol "$symbol" "Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilCsvFunctionLibrary.h"
done

for symbol in ReplaceDataTableFromCsv DiffDataTables; do
	require_symbol "$symbol" "Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilDataTableFunctionLibrary.h"
done

for symbol in HasConfigKey ReadConfigVector WriteConfigVector ReadConfigColor WriteConfigColor; do
	require_symbol "$symbol" "Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilConfigFunctionLibrary.h"
done

if grep -Eq "static bool (FileExists|DirectoryExists|CreateDirectory|DeleteFile|DeleteDirectory|CopyFile|MoveFile|FindFiles|FindDirectories)\\(" \
	"$REPOSITORY_ROOT/Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilFileSystemFunctionLibrary.h"; then
	echo "File System library duplicates UE 5.8 Blueprint file-management nodes" >&2
	exit 1
fi

exec "$SCRIPT_ROOT/run-unix.sh" "$@"
