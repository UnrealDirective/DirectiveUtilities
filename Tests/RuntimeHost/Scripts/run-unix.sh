#!/usr/bin/env bash

set -euo pipefail

ENGINE_ROOT="${1:?Usage: run-unix.sh <engine-root> [Development|Shipping]}"
CLIENT_CONFIGURATION="${2:-Development}"
TEST_TIMEOUT_SECONDS="${DIRECTIVE_UTILITIES_TEST_TIMEOUT_SECONDS:-1800}"
if [[ "$CLIENT_CONFIGURATION" != "Development" ]] && [[ "$CLIENT_CONFIGURATION" != "Shipping" ]]; then
	echo "Unsupported client configuration: $CLIENT_CONFIGURATION" >&2
	exit 2
fi
SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "$SCRIPT_ROOT/../../.." && pwd)"
ENGINE_VERSION="$(basename "$ENGINE_ROOT")"
WORK_ROOT="$REPOSITORY_ROOT/Build/RuntimeHost/$ENGINE_VERSION/$CLIENT_CONFIGURATION"
PROJECT_ROOT="$WORK_ROOT/Project"
PROJECT_FILE="$PROJECT_ROOT/DirectiveUtilitiesRuntimeHost.uproject"
PLUGIN_ROOT="$PROJECT_ROOT/Plugins/DirectiveUtilities"
PLUGIN_DESCRIPTOR="$PLUGIN_ROOT/DirectiveUtilities.uplugin"
RUNTIME_TEST_MODULE="$PROJECT_ROOT/Source/DirectiveUtilitiesRuntimeHostTests"
RUNTIME_TEST_SOURCE_ROOT="$REPOSITORY_ROOT/Source/DirectiveUtilitiesTests"
ARCHIVE_ROOT="$WORK_ROOT/Archive"
REPORT_ROOT="$WORK_ROOT/Reports"
PERFORMANCE_ROOT="$WORK_ROOT/Performance"
EXPECTED_EDITOR_TEST_COUNT=61
EXPECTED_PACKAGED_TEST_COUNT=42

run_with_timeout() {
	python3 - "$TEST_TIMEOUT_SECONDS" "$@" <<'PY'
import os
import signal
import subprocess
import sys

timeout_seconds = int(sys.argv[1])
command = sys.argv[2:]
process = subprocess.Popen(command, start_new_session=True)
try:
    raise SystemExit(process.wait(timeout=timeout_seconds))
except subprocess.TimeoutExpired:
    os.killpg(process.pid, signal.SIGTERM)
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        process.wait()
    print(f"Timed out after {timeout_seconds}s: {command[0]}", file=sys.stderr)
    raise SystemExit(124)
PY
}

assert_clean_report() {
	python3 - "$1" "$3" "$2" <<'PY'
import json
import sys
from collections import Counter
from pathlib import Path

path = Path(sys.argv[1])
expected = int(sys.argv[2])
label = sys.argv[3]
if not path.is_file():
    raise SystemExit(f"{label} automation report was not generated: {path}")
report = json.loads(path.read_text(encoding="utf-8-sig"))
tests = report.get("tests", [])
if any(report.get(key) != 0 for key in ("failed", "succeededWithWarnings", "notRun")):
    raise SystemExit(f"{label} automation report is not clean: {path}")
if report.get("succeeded") != expected or len(tests) != expected:
    raise SystemExit(f"{label} expected {expected} tests, found {len(tests)}: {path}")
paths = [test.get("fullTestPath") for test in tests]
if any(test.get("state") != "Success" for test in tests):
    raise SystemExit(f"{label} report contains a non-success state: {path}")
if any(count != 1 for count in Counter(paths).values()):
    raise SystemExit(f"{label} report contains duplicate paths: {path}")
PY
}

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
if [[ "$CLIENT_CONFIGURATION" == "Development" ]]; then
	mkdir -p "$REPORT_ROOT/Game"
fi

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
cp "$RUNTIME_TEST_SOURCE_ROOT/Public/Tests/DirectiveUtilTestDataTableRows.h" "$RUNTIME_TEST_MODULE/Public/Tests/DirectiveUtilTestDataTableRows.h"

"$BUILD_SCRIPT" DirectiveUtilitiesRuntimeHostEditor "$PLATFORM" Development \
	-Project="$PROJECT_FILE" \
	-WaitMutex \
	-NoHotReload \
	-ForceUnity \
	-DisableAdaptiveUnity \
	-NoUBA

run_with_timeout "$EDITOR_COMMAND" "$PROJECT_FILE" \
	-ExecCmds="Automation RunTests DirectiveUtilities; Quit" \
	-ReportExportPath="$REPORT_ROOT/Editor" \
	-TestExit="Automation Test Queue Empty" \
	-abslog="$WORK_ROOT/EditorTests.log" \
	-unattended \
	-nop4 \
	-nosplash \
	-nosound \
	-NullRHI \
	-NoEngineAnalytics \
	-NoEpicPortal \
	'-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False'

"$RUN_UAT" BuildCookRun \
	-project="$PROJECT_FILE" \
	-noP4 \
	-platform="$PLATFORM" \
	-clientconfig="$CLIENT_CONFIGURATION" \
	-build \
	-cook \
	-stage \
	-pak \
	-skipzenstore \
	-package \
	-archive \
	-archivedirectory="$ARCHIVE_ROOT" \
	-ubtargs=-NoUBA \
	-unattended \
	-utf8output

GAME_LOG="$WORK_ROOT/GameTests.log"
APPEND_OUTPUT_NAME="shipping-append-comparison.csv"
SMOKE_OUTPUT_NAME="shipping-smoke.csv"
REVISION="$(git -C "$REPOSITORY_ROOT" rev-parse HEAD 2>/dev/null || true)"
if [[ -n "$REVISION" ]] && [[ -n "$(git -C "$REPOSITORY_ROOT" status --porcelain 2>/dev/null)" ]]; then
	REVISION="${REVISION}-dirty"
fi
if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
	mkdir -p "$PERFORMANCE_ROOT"
fi

if [[ "$PLATFORM" == "Mac" ]]; then
	GAME_APP="$(find "$ARCHIVE_ROOT" -type d -name 'DirectiveUtilitiesRuntimeHost*.app' -print -quit)"
	if [[ -z "$GAME_APP" ]]; then
		echo "Packaged game app not found under $ARCHIVE_ROOT" >&2
		exit 1
	fi
	GAME_EXECUTABLE="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$GAME_APP/Contents/Info.plist" 2>/dev/null || true)"
	GAME_COMMAND="$GAME_APP/Contents/MacOS/$GAME_EXECUTABLE"
	if [[ -z "$GAME_EXECUTABLE" ]] || [[ ! -f "$GAME_COMMAND" ]] || [[ ! -x "$GAME_COMMAND" ]]; then
		echo "Packaged game executable not found in $GAME_APP" >&2
		exit 1
	fi

	MAC_LOG_ROOT="$HOME/Library/Containers/com.unrealdirective.directiveutilitiesruntimehosttests/Data/Library/Logs/DirectiveUtilitiesRuntimeHostTests"
	MAC_GAME_LOG="$MAC_LOG_ROOT/GameTests.log"
	mkdir -p "$MAC_LOG_ROOT"
	rm -f "$MAC_GAME_LOG"
	if [[ "$CLIENT_CONFIGURATION" == "Development" ]]; then
		MAC_GAME_REPORT_ROOT="$MAC_LOG_ROOT/Reports/Game"
		rm -rf "$MAC_GAME_REPORT_ROOT"
		mkdir -p "$MAC_GAME_REPORT_ROOT"
	fi

	GAME_ARGUMENTS=(
		"-abslog=$MAC_GAME_LOG"
		-unattended
		-nop4
		-nosplash
		-nosound
		-NullRHI
		-NoEngineAnalytics
		-NoEpicPortal
	)
	if [[ "$ENGINE_VERSION" == "UE_5.8" ]]; then
		GAME_ARGUMENTS+=(
			-LLM
		)
	fi
	if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
		MAC_APPEND_OUTPUT="$MAC_LOG_ROOT/$APPEND_OUTPUT_NAME"
		MAC_SMOKE_OUTPUT="$MAC_LOG_ROOT/$SMOKE_OUTPUT_NAME"
		rm -f "$MAC_APPEND_OUTPUT"
		rm -f "$MAC_SMOKE_OUTPUT"
		GAME_ARGUMENTS+=(
			"-DirectiveUtilitiesAppendShippingBenchmarkOutput=$MAC_APPEND_OUTPUT"
			"-DirectiveUtilitiesShippingSmokeOutput=$MAC_SMOKE_OUTPUT"
			"-DirectiveUtilitiesPerfRevision=$REVISION"
		)
	else
		GAME_ARGUMENTS+=(
			"-ExecCmds=Automation RunTests DirectiveUtilities; Quit"
			"-TestExit=Automation Test Queue Empty"
			"-ReportExportPath=$MAC_GAME_REPORT_ROOT"
		)
	fi

	set +e
	run_with_timeout "$GAME_COMMAND" "${GAME_ARGUMENTS[@]}" >/dev/null 2>&1
	GAME_EXIT_CODE=$?
	set -e
	if [[ -f "$MAC_GAME_LOG" ]]; then
		cp "$MAC_GAME_LOG" "$GAME_LOG"
	fi
	if [[ "$CLIENT_CONFIGURATION" == "Development" ]] && [[ -d "$MAC_GAME_REPORT_ROOT" ]]; then
		rsync -a "$MAC_GAME_REPORT_ROOT/" "$REPORT_ROOT/Game/"
	fi
	if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
		cp "$MAC_APPEND_OUTPUT" "$PERFORMANCE_ROOT/$APPEND_OUTPUT_NAME"
		cp "$MAC_SMOKE_OUTPUT" "$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME"
	fi
else
	GAME_COMMAND="$(find "$ARCHIVE_ROOT" -type f -name DirectiveUtilitiesRuntimeHost -perm -111 -print -quit)"
	if [[ -z "$GAME_COMMAND" ]]; then
		echo "Packaged game executable not found under $ARCHIVE_ROOT" >&2
		exit 1
	fi

	GAME_ARGUMENTS=(
		"-abslog=$GAME_LOG"
		-unattended
		-nop4
		-nosplash
		-nosound
		-NullRHI
		-NoEngineAnalytics
		-NoEpicPortal
	)
	if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
		GAME_ARGUMENTS+=(
			"-DirectiveUtilitiesAppendShippingBenchmarkOutput=$PERFORMANCE_ROOT/$APPEND_OUTPUT_NAME"
			"-DirectiveUtilitiesShippingSmokeOutput=$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME"
			"-DirectiveUtilitiesPerfRevision=$REVISION"
		)
	else
		GAME_ARGUMENTS+=(
			"-ExecCmds=Automation RunTests DirectiveUtilities; Quit"
			"-TestExit=Automation Test Queue Empty"
			"-ReportExportPath=$REPORT_ROOT/Game"
		)
	fi

	set +e
	run_with_timeout "$GAME_COMMAND" "${GAME_ARGUMENTS[@]}" >/dev/null 2>&1
	GAME_EXIT_CODE=$?
	set -e
fi

if [[ "$GAME_EXIT_CODE" -ne 0 ]]; then
	if [[ -f "$GAME_LOG" ]]; then
		tail -n 100 "$GAME_LOG" >&2
	fi
	echo "Packaged game failed. Log: $GAME_LOG" >&2
	exit 1
fi

if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
	if [[ ! -f "$PERFORMANCE_ROOT/$APPEND_OUTPUT_NAME" ]] || \
		! grep -q '#configuration,Shipping' "$PERFORMANCE_ROOT/$APPEND_OUTPUT_NAME"; then
		if [[ -f "$GAME_LOG" ]]; then
			tail -n 100 "$GAME_LOG" >&2
		fi
		echo "Packaged Shipping append benchmark failed. Log: $GAME_LOG" >&2
		exit 1
	fi
	for ELEMENT_TYPE in bool int32 float FVector FString UObject; do
		if [[ "$(grep -c "^${ELEMENT_TYPE}," "$PERFORMANCE_ROOT/$APPEND_OUTPUT_NAME")" -ne 10 ]]; then
			echo "Packaged Shipping append benchmark is missing $ELEMENT_TYPE scenarios." >&2
			exit 1
		fi
	done
	if [[ ! -f "$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME" ]] || \
		! grep -q '#configuration,Shipping' "$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME" || \
		grep -q ',false$' "$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME" || \
		[[ "$(grep -c ',true$' "$PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME")" -ne 15 ]]; then
		echo "Packaged Shipping smoke test failed: $PERFORMANCE_ROOT/$SMOKE_OUTPUT_NAME" >&2
		exit 1
	fi
elif ! grep -q 'TEST COMPLETE. EXIT CODE: 0' "$GAME_LOG"; then
	tail -n 100 "$GAME_LOG" >&2
	echo "Packaged game automation failed. Log: $GAME_LOG" >&2
	exit 1
fi

if [[ "$CLIENT_CONFIGURATION" == "Development" ]]; then
	assert_clean_report "$REPORT_ROOT/Game/index.json" "Packaged game" "$EXPECTED_PACKAGED_TEST_COUNT"
fi

assert_clean_report "$REPORT_ROOT/Editor/index.json" "Editor" "$EXPECTED_EDITOR_TEST_COUNT"

echo "Reports: $REPORT_ROOT"
if [[ "$CLIENT_CONFIGURATION" == "Shipping" ]]; then
	echo "Editor tests and packaged Shipping smoke and benchmark passed for $ENGINE_VERSION."
	echo "Shipping performance results: $PERFORMANCE_ROOT"
else
	echo "Editor and packaged game tests passed for $ENGINE_VERSION."
	echo "Packaged game log: $GAME_LOG"
fi
