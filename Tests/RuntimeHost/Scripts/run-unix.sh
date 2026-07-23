#!/usr/bin/env bash

set -euo pipefail

ENGINE_ROOT="${1:?Usage: run-unix.sh <engine-root>}"
SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_ROOT/../../.." && pwd)"
ENGINE_VERSION="$(basename "$ENGINE_ROOT")"
WORK_ROOT="$REPOSITORY_ROOT/Build/RuntimeHost/$ENGINE_VERSION"
PROJECT_ROOT="$WORK_ROOT/Project"
PROJECT_FILE="$PROJECT_ROOT/DirectiveUtilitiesRuntimeHost.uproject"
PLUGIN_ROOT="$PROJECT_ROOT/Plugins/DirectiveUtilities"
PLUGIN_DESCRIPTOR="$PLUGIN_ROOT/DirectiveUtilities.uplugin"
RUNTIME_TEST_MODULE="$PROJECT_ROOT/Source/DirectiveUtilitiesRuntimeHostTests"
RUNTIME_TEST_SOURCE_ROOT="$REPOSITORY_ROOT/Source/DirectiveUtilitiesTests"
ARCHIVE_ROOT="$WORK_ROOT/Archive"
REPORT_ROOT="$WORK_ROOT/Reports"

case "$(uname -s)" in
	Darwin)
		PLATFORM="Mac"
		BUILD_SCRIPT="$ENGINE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh"
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd"
		;;
	Linux)
		PLATFORM="Linux"
		BUILD_SCRIPT="$ENGINE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh"
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
		;;
	*)
		echo "Unsupported host platform: $(uname -s)" >&2
		exit 2
		;;
esac

RUN_UAT="$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"

require_file() {
	if [[ ! -f "$1" ]]; then
		echo "Required file not found: $1" >&2
		exit 2
	fi
}

require_file "$BUILD_SCRIPT"
require_file "$EDITOR_COMMAND"
require_file "$RUN_UAT"

rm -rf "$WORK_ROOT"
mkdir -p "$PLUGIN_ROOT" "$REPORT_ROOT/Editor"

rsync -a \
	--exclude Binaries \
	--exclude Intermediate \
	--exclude Saved \
	--exclude Scripts \
	--exclude Plugins \
	"$REPOSITORY_ROOT/Tests/RuntimeHost/" \
	"$PROJECT_ROOT/"

cp "$REPOSITORY_ROOT/DirectiveUtilities.uplugin" "$PLUGIN_DESCRIPTOR"
for DIRECTORY in Source Config Resources; do
	if [[ -d "$REPOSITORY_ROOT/$DIRECTORY" ]]; then
		rsync -a "$REPOSITORY_ROOT/$DIRECTORY/" "$PLUGIN_ROOT/$DIRECTORY/"
	fi
done

mkdir -p "$RUNTIME_TEST_MODULE/Private/Tests" "$RUNTIME_TEST_MODULE/Public/Tests"
while IFS= read -r TEST_SOURCE; do
	if [[ -n "$TEST_SOURCE" ]]; then
		cp "$RUNTIME_TEST_SOURCE_ROOT/Private/Tests/$TEST_SOURCE" "$RUNTIME_TEST_MODULE/Private/Tests/$TEST_SOURCE"
	fi
done < "$REPOSITORY_ROOT/Tests/RuntimeHost/RuntimeTestSources.txt"
cp "$RUNTIME_TEST_SOURCE_ROOT/Public/Tests/DirectiveUtilTestObject.h" "$RUNTIME_TEST_MODULE/Public/Tests/DirectiveUtilTestObject.h"

"$BUILD_SCRIPT" DirectiveUtilitiesRuntimeHostEditor "$PLATFORM" Development \
	-Project="$PROJECT_FILE" \
	-WaitMutex \
	-NoHotReload \
	-ForceUnity \
	-DisableAdaptiveUnity

"$EDITOR_COMMAND" "$PROJECT_FILE" \
	-ExecCmds="Automation RunTests DirectiveUtilities; Quit" \
	-ReportExportPath="$REPORT_ROOT/Editor" \
	-TestExit="Automation Test Queue Empty" \
	-abslog="$WORK_ROOT/EditorTests.log" \
	-unattended \
	-nop4 \
	-nosplash \
	-NullRHI

"$RUN_UAT" BuildCookRun \
	-project="$PROJECT_FILE" \
	-noP4 \
	-platform="$PLATFORM" \
	-clientconfig=Development \
	-build \
	-cook \
	-stage \
	-pak \
	-skipzenstore \
	-package \
	-archive \
	-archivedirectory="$ARCHIVE_ROOT" \
	-unattended \
	-utf8output

GAME_LOG="$WORK_ROOT/GameTests.log"
if [[ "$PLATFORM" == "Mac" ]]; then
	GAME_APP="$(find "$ARCHIVE_ROOT" -type d -name DirectiveUtilitiesRuntimeHost.app -print -quit)"
	if [[ -z "$GAME_APP" ]]; then
		echo "Packaged game app not found under $ARCHIVE_ROOT" >&2
		exit 1
	fi

	MAC_LOG_ROOT="$HOME/Library/Containers/com.unrealdirective.directiveutilitiesruntimehosttests/Data/Library/Logs/DirectiveUtilitiesRuntimeHostTests"
	MAC_GAME_LOG="$MAC_LOG_ROOT/GameTests.log"
	mkdir -p "$MAC_LOG_ROOT"
	rm -f "$MAC_GAME_LOG"

	open -n -W "$GAME_APP" --args \
		-ExecCmds="Automation RunTests DirectiveUtilities; Quit" \
		-TestExit="Automation Test Queue Empty" \
		-abslog="$MAC_GAME_LOG" \
		-unattended \
		-nop4 \
		-nosplash \
		-nosound \
		-NullRHI
	if [[ -f "$MAC_GAME_LOG" ]]; then
		cp "$MAC_GAME_LOG" "$GAME_LOG"
	fi
	GAME_EXIT_CODE=0
else
	GAME_COMMAND="$(find "$ARCHIVE_ROOT" -type f -name DirectiveUtilitiesRuntimeHost -perm -111 -print -quit)"
	if [[ -z "$GAME_COMMAND" ]]; then
		echo "Packaged game executable not found under $ARCHIVE_ROOT" >&2
		exit 1
	fi

	set +e
	"$GAME_COMMAND" \
		-ExecCmds="Automation RunTests DirectiveUtilities; Quit" \
		-TestExit="Automation Test Queue Empty" \
		-abslog="$GAME_LOG" \
		-unattended \
		-nop4 \
		-nosplash \
		-nosound \
		-NullRHI \
		>/dev/null 2>&1
	GAME_EXIT_CODE=$?
	set -e
fi

if [[ "$GAME_EXIT_CODE" -ne 0 ]] || ! grep -q 'TEST COMPLETE. EXIT CODE: 0' "$GAME_LOG"; then
	if [[ -f "$GAME_LOG" ]]; then
		tail -n 100 "$GAME_LOG" >&2
	fi
	echo "Packaged game automation failed. Log: $GAME_LOG" >&2
	exit 1
fi

if [[ ! -f "$REPORT_ROOT/Editor/index.json" ]] || \
	! grep -q '"failed": 0' "$REPORT_ROOT/Editor/index.json" || \
	! grep -q '"notRun": 0' "$REPORT_ROOT/Editor/index.json"; then
	echo "Editor automation report is missing or incomplete: $REPORT_ROOT/Editor" >&2
	exit 1
fi

echo "Editor and packaged game tests passed for $ENGINE_VERSION."
echo "Reports: $REPORT_ROOT"
echo "Packaged game log: $GAME_LOG"
