# Runtime test host

This project runs the Directive Utilities automation suite in Unreal Editor and a packaged Development game. It is not included in packaged plugin releases and does not run through GitHub Actions.

Packaged runs execute runtime function and async-task tests, then check the reflected API and module boundaries. Editor-only and uncooked modules must remain absent from the packaged game.

On macOS or Linux:

```sh
Tests/RuntimeHost/Scripts/run-unix.sh "/Users/Shared/Epic Games/UE_5.8"
```

On Windows:

```powershell
Tests\RuntimeHost\Scripts\run-windows.ps1 "C:\Program Files\Epic Games\UE_5.8"
```

Run the command once per supported engine installation. Each runner creates a clean project under `Build/RuntimeHost`, builds the editor target, runs editor automation, packages a Development game, and runs automation from that package.

The runner copies runtime-compatible test sources into a game-only host module. The plugin descriptor and released packages remain unchanged.
