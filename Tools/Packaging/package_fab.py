#!/usr/bin/env python3
# Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path
from typing import Iterable, Sequence


PLUGIN_NAME = "DirectiveUtilities"
SUPPORTED_ENGINE_VERSIONS = ("5.6", "5.7", "5.8")
SUPPORTED_PLATFORMS = ("Win64", "Mac", "Linux")
REQUIRED_PLUGIN_DEPENDENCIES = ("EditorScriptingUtilities", "EnhancedInput")
PACKAGE_ENTRIES = (
    "CHANGELOG.md",
    "Config",
    "DirectiveUtilities.uplugin",
    "Documentation",
    "LICENSE",
    "README.md",
    "Resources",
    "Source",
)
GENERATED_PACKAGE_DIRECTORIES = ("Content",)
FORBIDDEN_DIRECTORY_NAMES = {
    ".git",
    "Binaries",
    "Build",
    "DerivedDataCache",
    "Intermediate",
    "Saved",
}
IGNORED_FILE_PATTERNS = (".DS_Store", "__pycache__", "*.bak", "*.pyc", "*.pyo", "*.tmp")
SOURCE_FILE_SUFFIXES = {".cpp", ".cs", ".h"}
COPYRIGHT_NOTICE = "Copyright (c) 2026 Unreal Directive. Licensed under the MIT License."
MAX_PACKAGE_PATH_LENGTH = 170
ZIP_TIMESTAMP = (2026, 1, 1, 0, 0, 0)
REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class PackagingError(RuntimeError):
    pass


def load_descriptor(plugin_root: Path) -> dict:
    descriptor_path = plugin_root / "DirectiveUtilities.uplugin"
    try:
        return json.loads(descriptor_path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise PackagingError(f"Could not read {descriptor_path}: {error}") from error


def validate_common_descriptor(descriptor: dict) -> list[str]:
    errors: list[str] = []
    modules = descriptor.get("Modules")
    if not isinstance(modules, list) or not modules:
        return ["The plugin descriptor does not define any modules."]

    expected_platforms = set(SUPPORTED_PLATFORMS)
    for module in modules:
        module_name = module.get("Name", "<unnamed>")
        actual_platforms = set(module.get("PlatformAllowList", []))
        if actual_platforms != expected_platforms:
            errors.append(
                f"Module {module_name} must allow exactly: {', '.join(SUPPORTED_PLATFORMS)}."
            )

    dependency_names = {
        dependency.get("Name")
        for dependency in descriptor.get("Plugins", [])
        if isinstance(dependency, dict)
    }
    for dependency_name in REQUIRED_PLUGIN_DEPENDENCIES:
        if dependency_name not in dependency_names:
            errors.append(f"The plugin descriptor is missing dependency {dependency_name}.")

    return errors


def validate_source_notices(source_root: Path) -> list[str]:
    errors: list[str] = []
    for source_file in sorted(path for path in source_root.rglob("*") if path.is_file()):
        if source_file.suffix.lower() not in SOURCE_FILE_SUFFIXES:
            continue
        try:
            first_lines = source_file.read_text(encoding="utf-8-sig").splitlines()[:4]
        except UnicodeDecodeError:
            errors.append(f"Source file is not valid UTF-8: {source_file}")
            continue
        if not any(COPYRIGHT_NOTICE in line for line in first_lines):
            errors.append(f"Source file is missing the copyright notice: {source_file}")
    return errors


def validate_package_paths(plugin_root: Path) -> list[str]:
    errors: list[str] = []
    for path in sorted(plugin_root.rglob("*")):
        relative_path = path.relative_to(plugin_root).as_posix()
        package_path = f"{PLUGIN_NAME}/{relative_path}"
        if len(package_path) > MAX_PACKAGE_PATH_LENGTH:
            errors.append(
                f"Package path is {len(package_path)} characters and exceeds "
                f"the {MAX_PACKAGE_PATH_LENGTH}-character limit: {package_path}"
            )
    return errors


def validate_forbidden_directories(plugin_root: Path) -> list[str]:
    errors: list[str] = []
    for directory in sorted(path for path in plugin_root.rglob("*") if path.is_dir()):
        if directory.name in FORBIDDEN_DIRECTORY_NAMES:
            errors.append(f"Package contains a forbidden directory: {directory}")
    return errors


def raise_for_errors(errors: Iterable[str]) -> None:
    error_list = list(errors)
    if error_list:
        raise PackagingError("\n".join(error_list))


def validate_repository_source(repository_root: Path) -> dict:
    missing_entries = [
        entry for entry in PACKAGE_ENTRIES if not (repository_root / entry).exists()
    ]
    errors = [f"Required package entry is missing: {entry}" for entry in missing_entries]

    descriptor = load_descriptor(repository_root)
    errors.extend(validate_common_descriptor(descriptor))
    if "EngineVersion" in descriptor:
        errors.append("The repository descriptor must remain engine-neutral.")
    if descriptor.get("Installed") is not False:
        errors.append("The repository descriptor must set Installed to false.")
    errors.extend(validate_source_notices(repository_root / "Source"))

    raise_for_errors(errors)
    return descriptor


def validate_clean_repository(repository_root: Path) -> None:
    result = subprocess.run(
        ["git", "status", "--porcelain=v1", "--untracked-files=all"],
        cwd=repository_root,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise PackagingError(result.stderr.strip() or "Could not inspect the Git working tree.")
    if result.stdout.strip():
        raise PackagingError("The Git working tree must be clean before packaging.")


def copy_package_source(repository_root: Path, plugin_root: Path) -> None:
    plugin_root.mkdir(parents=True)
    for entry in PACKAGE_ENTRIES:
        source = repository_root / entry
        destination = plugin_root / entry
        if source.is_dir():
            shutil.copytree(
                source,
                destination,
                ignore=shutil.ignore_patterns(*IGNORED_FILE_PATTERNS),
            )
        else:
            shutil.copy2(source, destination)

    for directory in GENERATED_PACKAGE_DIRECTORIES:
        (plugin_root / directory).mkdir()


def write_fab_descriptor(plugin_root: Path, descriptor: dict, engine_version: str) -> None:
    fab_descriptor: dict = {}
    for key, value in descriptor.items():
        if key == "CanContainContent":
            fab_descriptor["EngineVersion"] = f"{engine_version}.0"
        fab_descriptor[key] = True if key == "Installed" else value

    if "EngineVersion" not in fab_descriptor:
        fab_descriptor["EngineVersion"] = f"{engine_version}.0"
    fab_descriptor["Installed"] = True

    descriptor_path = plugin_root / "DirectiveUtilities.uplugin"
    descriptor_path.write_text(
        json.dumps(fab_descriptor, indent="\t", ensure_ascii=False) + "\n",
        encoding="utf-8",
    )


def validate_fab_artifact(plugin_root: Path, engine_version: str) -> None:
    errors: list[str] = []
    if plugin_root.name != PLUGIN_NAME:
        errors.append(f"The plugin folder must be named {PLUGIN_NAME}.")

    actual_entries = {path.name for path in plugin_root.iterdir()}
    expected_entries = set(PACKAGE_ENTRIES) | set(GENERATED_PACKAGE_DIRECTORIES)
    unexpected_entries = sorted(actual_entries - expected_entries)
    missing_entries = sorted(expected_entries - actual_entries)
    errors.extend(f"Unexpected package entry: {entry}" for entry in unexpected_entries)
    errors.extend(f"Required package entry is missing: {entry}" for entry in missing_entries)

    descriptor = load_descriptor(plugin_root)
    errors.extend(validate_common_descriptor(descriptor))
    if descriptor.get("EngineVersion") != f"{engine_version}.0":
        errors.append(f"EngineVersion must be {engine_version}.0.")
    if descriptor.get("Installed") is not True:
        errors.append("Fab packages must set Installed to true.")

    errors.extend(validate_source_notices(plugin_root / "Source"))
    errors.extend(validate_package_paths(plugin_root))
    errors.extend(validate_forbidden_directories(plugin_root))
    raise_for_errors(errors)


def write_deterministic_archive(plugin_root: Path, archive_path: Path) -> None:
    temporary_archive = archive_path.with_suffix(".zip.tmp")
    try:
        with zipfile.ZipFile(temporary_archive, "w") as archive:
            paths = [plugin_root, *sorted(plugin_root.rglob("*"))]
            for path in paths:
                archive_name = path.relative_to(plugin_root.parent).as_posix()
                if path.is_dir():
                    archive_name += "/"

                info = zipfile.ZipInfo(archive_name, ZIP_TIMESTAMP)
                info.create_system = 3
                if path.is_dir():
                    info.external_attr = 0o40755 << 16
                    archive.writestr(info, b"")
                else:
                    info.external_attr = 0o100644 << 16
                    archive.writestr(
                        info,
                        path.read_bytes(),
                        compress_type=zipfile.ZIP_DEFLATED,
                        compresslevel=9,
                    )

        temporary_archive.replace(archive_path)
    finally:
        temporary_archive.unlink(missing_ok=True)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def package_fab_artifact(
    repository_root: Path,
    output_directory: Path,
    engine_version: str,
) -> Path:
    if engine_version not in SUPPORTED_ENGINE_VERSIONS:
        raise PackagingError(f"Unsupported engine version: {engine_version}")

    descriptor = validate_repository_source(repository_root)
    plugin_version = descriptor.get("VersionName")
    if not isinstance(plugin_version, str) or not plugin_version:
        raise PackagingError("The plugin descriptor must define VersionName.")

    output_directory.mkdir(parents=True, exist_ok=True)
    archive_path = output_directory / (
        f"{PLUGIN_NAME}-{plugin_version}-UE{engine_version}-Fab.zip"
    )

    with tempfile.TemporaryDirectory(prefix="directive-utilities-fab-") as staging_directory:
        plugin_root = Path(staging_directory) / PLUGIN_NAME
        copy_package_source(repository_root, plugin_root)
        write_fab_descriptor(plugin_root, descriptor, engine_version)
        validate_fab_artifact(plugin_root, engine_version)
        write_deterministic_archive(plugin_root, archive_path)

    return archive_path


def parse_arguments(arguments: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build source-only Directive Utilities packages for Fab."
    )
    parser.add_argument(
        "--engine",
        action="append",
        choices=SUPPORTED_ENGINE_VERSIONS,
        dest="engine_versions",
        help="Engine version to package. Repeat for more than one version.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=REPOSITORY_ROOT / "Build" / "Fab",
        help="Directory for generated archives.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate the source without creating archives.",
    )
    parser.add_argument(
        "--allow-dirty",
        action="store_true",
        help="Allow packaging from a modified working tree.",
    )
    return parser.parse_args(arguments)


def main(arguments: Sequence[str] | None = None) -> int:
    options = parse_arguments(arguments)
    try:
        validate_repository_source(REPOSITORY_ROOT)
        if options.check:
            print("Fab package source is valid.")
            return 0

        if not options.allow_dirty:
            validate_clean_repository(REPOSITORY_ROOT)

        engine_versions = options.engine_versions or SUPPORTED_ENGINE_VERSIONS
        for engine_version in engine_versions:
            archive_path = package_fab_artifact(
                REPOSITORY_ROOT,
                options.output_dir.resolve(),
                engine_version,
            )
            print(f"{sha256_file(archive_path)}  {archive_path}")
    except (OSError, PackagingError) as error:
        print(error, file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
