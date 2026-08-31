# Runtime test host

This project runs the Directive Utilities automation suite in Unreal Editor and a packaged Development game. It is not included in packaged plugin releases. Verification is local only; the repository does not use hosted CI.

Packaged runs execute runtime function and async-task tests, then check the reflected API and module boundaries. Editor-only and uncooked modules must remain absent from the packaged game.

On macOS or Linux:

```sh
Tests/RuntimeHost/Scripts/run-unix.sh "/Users/Shared/Epic Games/UE_5.8"
```

On Windows:

```powershell
Tests\RuntimeHost\Scripts\run-windows.ps1 "C:\Program Files\Epic Games\UE_5.8"
```

The runners impose a 30-minute timeout on each Editor or packaged-game test
process and require exactly 60 successful tests in the editor host and 41
runtime-safe tests in packaged Development. A non-zero process exit, missing
report, changed test census, duplicate
test path, warning, skipped test, or missing completion marker fails the run.
Use `-TestTimeoutSeconds` on Windows or
`DIRECTIVE_UTILITIES_TEST_TIMEOUT_SECONDS` on macOS/Linux to adjust the watchdog.
Windows native, extracted Fab artifact, and Linux cross-compiled builds default
to eight parallel compile actions to avoid exhausting the system commit limit.
Use `-MaxParallelActions` to tune that bound for the host.
Windows release validation also runs the UE 5.8 Development tests with
`-StompMalloc` to expose guarded-allocation memory errors.

Run the command once per supported engine installation. Each runner creates a clean project under `Build/RuntimeHost/UE_5.x/<Configuration>`, builds the editor target, runs editor automation, packages the requested game configuration, and runs its validation. Development and Shipping evidence is retained separately.

On Windows, keep the repository path short enough for Unreal's 260-character action-path limit. The runner checks its longest generated path before starting a build and reports this requirement directly.

The local release gate runs this project for UE 5.6, 5.7, and 5.8, then runs packaged Shipping behavior checks and the append benchmark on UE 5.8. Shipping checks exercise reflected array operations, math, point generation, hex queries, easing, and strings without loading the test module.

The release gate rejects warnings, failures, skipped tests, and dirty working trees. Set `DIRECTIVE_UTILITIES_ALLOW_DIRTY_RELEASE=1` on macOS or Linux, or pass `-AllowDirty` on Windows, only while developing the gate. A publishing run must use a clean tree. The final manifest under `Build/ReleaseEvidence` records the commit, validation matrix, automation counts, and SHA-256 hashes for the reports, performance results, compiled Fab packages, and Fab archives. Windows runs with `-IncludeLinux` also record the Linux Development and Shipping cross-compiled object files. Unreal's monolithic Linux game target keeps these under the packaged plugin's `Intermediate` directory instead of emitting plugin shared libraries.

On macOS, the UE 5.8 packaged test process starts with `-LLM`. The installed 5.8 runtime can crash while disabling its low-level memory tracker before log initialization. Keeping the tracker enabled avoids that engine startup fault. This changes only the test command; it does not change the packaged plugin or game.

The runner copies runtime-compatible test sources into a game-only host module. The plugin descriptor and released packages remain unchanged.
