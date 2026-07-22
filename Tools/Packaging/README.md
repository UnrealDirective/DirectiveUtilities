# Release packaging

The repository descriptor stays compatible with every supported engine version. Distribution packages use copied descriptors with the target engine version.

## Fab packages

Run the packager from the repository root:

```sh
python3 Tools/Packaging/package_fab.py
```

On Windows, the Python launcher can run the same script:

```powershell
py -3 Tools/Packaging/package_fab.py
```

The default command writes source-only archives for Unreal Engine 5.6, 5.7, and 5.8 under `Build/Fab`. Each archive contains one `DirectiveUtilities` folder and can compile on Win64, Mac, or Linux. The packager adds the empty `Content` directory required for Fab code-plugin submissions.

Build one engine version with:

```sh
python3 Tools/Packaging/package_fab.py --engine 5.8
```

Validate the source without writing an archive:

```sh
python3 Tools/Packaging/package_fab.py --check
```

The packager rejects missing source notices, unsupported module platform lists, paths over Fab's 170-character limit, and local build directories. It sets `EngineVersion` and `Installed` only in the copied descriptor.

Upload the archive itself for each Fab engine-version entry. A repository or release page is not a project-file download link.

The completed listing fields are kept in [Fab technical information](../../Distribution/Fab/TechnicalInformation.md).

## GitHub packages

GitHub releases keep the prebuilt packages for each engine and host platform. The Fab archives do not replace those downloads.
