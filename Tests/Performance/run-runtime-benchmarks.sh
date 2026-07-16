#!/usr/bin/env bash

set -euo pipefail

ENGINE_ROOT="${1:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
PROJECT_FILE="${2:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
OUTPUT_FILE="${3:?Usage: run-runtime-benchmarks.sh <engine-root> <project-file> <output-file> [baseline-file]}"
BASELINE_FILE="${4:-}"

case "$(uname -s)" in
	Darwin)
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd"
		;;
	Linux)
		EDITOR_COMMAND="$ENGINE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
		;;
	*)
		echo "Unsupported host platform: $(uname -s)" >&2
		exit 2
		;;
esac

if [[ ! -x "$EDITOR_COMMAND" ]]; then
	echo "Unreal Editor command not found: $EDITOR_COMMAND" >&2
	exit 2
fi

if [[ ! -f "$PROJECT_FILE" ]]; then
	echo "Project file not found: $PROJECT_FILE" >&2
	exit 2
fi

mkdir -p "$(dirname "$OUTPUT_FILE")"
LOG_FILE="${OUTPUT_FILE%.*}.log"
ARGUMENTS=(
	"$PROJECT_FILE"
	'-ExecCmds=Automation RunTests Performance.DirectiveUtilities.Runtime; Quit'
	'-TestExit=Automation Test Queue Empty'
	"-DirectiveUtilitiesPerfOutput=$OUTPUT_FILE"
	"-abslog=$LOG_FILE"
	-unattended
	-nop4
	-nosplash
	-nosound
	-NullRHI
)

if [[ -n "$BASELINE_FILE" ]]; then
	if [[ ! -f "$BASELINE_FILE" ]]; then
		echo "Baseline file not found: $BASELINE_FILE" >&2
		exit 2
	fi
	ARGUMENTS+=("-DirectiveUtilitiesPerfBaseline=$BASELINE_FILE")
fi

set +e
"$EDITOR_COMMAND" "${ARGUMENTS[@]}"
EDITOR_EXIT_CODE=$?
set -e

if [[ "$EDITOR_EXIT_CODE" -ne 0 ]] || [[ ! -f "$OUTPUT_FILE" ]] || \
	! grep -q 'Test Completed. Result={Success} Name={Runtime} Path={Performance.DirectiveUtilities.Runtime}' "$LOG_FILE"; then
	tail -n 100 "$LOG_FILE" >&2
	echo "Runtime performance suite failed. Log: $LOG_FILE" >&2
	exit 1
fi

echo "Runtime performance results: $OUTPUT_FILE"
echo "Automation log: $LOG_FILE"
