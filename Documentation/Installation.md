# Installation

Directive Utilities is a code plugin. It compiles three modules: `DirectiveUtilitiesRuntime` (Runtime), `DirectiveUtilitiesEditor` (Editor), and `DirectiveUtilitiesTests` (Editor-only automation tests). See [Compatibility](Compatibility.md) for supported engine versions and platforms.

## Before You Start

- Use Unreal Engine 5.6, 5.7, or 5.8.
- Close the editor before copying or replacing the plugin folder.
- If your project already has an older `UDCore` plugin folder, remove it before installing `DirectiveUtilities`.
- C++ projects compile the plugin with the project. Blueprint-only projects may need to let the editor compile the plugin on first launch.

## Option 1: GitHub release

1. Close the Unreal Editor.
2. Open [GitHub Releases](https://github.com/UnrealDirective/DirectiveUtilities/releases), choose the plugin version you need, and download its archive for your Unreal Engine version.
3. Extract the archive to `YourProject/Plugins/DirectiveUtilities`. Confirm that `DirectiveUtilities.uplugin` is directly inside that folder.
4. Open the project, enable `Directive Utilities`, and restart the editor.

Release packages are already compiled for the engine version named in the archive. If no package matches your engine version, install the source with Git.

## Option 2: Git Clone

```sh
git clone https://github.com/UnrealDirective/DirectiveUtilities.git DirectiveUtilities
```

1. Copy the `DirectiveUtilities` folder into your project's `Plugins/` directory.
2. Open the project, go to `Edit` → `Plugins`, search for `Directive Utilities`, and enable it.
3. Restart the editor. For a C++ project the modules build automatically; for a Blueprint-only project, allow the editor to compile the plugin when prompted.

## Verify The Install

Once enabled, the Directive Utilities Blueprint nodes appear under the **`Directive Utilities|...`** categories in any Blueprint graph. See the [Node Reference](README.md#node-reference) for the full list.

For C++ projects, also confirm your `.Build.cs` file references the module you need:

```csharp
PublicDependencyModuleNames.AddRange(
    new string[]
    {
        "DirectiveUtilitiesRuntime",
    });
```

Only add `DirectiveUtilitiesEditor` from editor modules or editor-only targets.

## Updating From UDCore

1. Remove the old `UDCore` plugin folder from `Plugins/`.
2. Install the current plugin folder as `DirectiveUtilities`.
3. Open the project and allow Blueprint assets to load. Bundled Core Redirects migrate saved references from the 1.x names.
4. For C++ projects, update module names, include paths, class names, and API macros to the Directive Utilities names.

## Troubleshooting

- **The plugin does not appear in the Plugins window:** confirm the folder is `YourProject/Plugins/DirectiveUtilities/DirectiveUtilities.uplugin`.
- **A C++ project fails to compile:** check that the project targets a supported engine version and that module names use `DirectiveUtilitiesRuntime` or `DirectiveUtilitiesEditor`.
- **Blueprint nodes are missing:** restart the editor after enabling the plugin, then search the Blueprint palette under `Directive Utilities|`.
- **Packaged game build fails after adding editor helpers:** move any `DirectiveUtilitiesEditor` dependency into an editor module or editor-only target.
