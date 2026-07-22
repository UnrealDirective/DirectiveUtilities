# Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

from __future__ import annotations

import json
import re
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

from Tools.Packaging import package_fab


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class FabPackagingTest(unittest.TestCase):
    def test_repository_source_is_valid(self) -> None:
        descriptor = package_fab.validate_repository_source(REPOSITORY_ROOT)

        self.assertNotIn("EngineVersion", descriptor)
        self.assertFalse(descriptor["Installed"])

    def test_dirty_repository_is_rejected(self) -> None:
        with patch.object(package_fab.subprocess, "run") as run:
            run.return_value.returncode = 0
            run.return_value.stdout = " M Source/File.cpp\n"
            run.return_value.stderr = ""

            with self.assertRaisesRegex(
                package_fab.PackagingError,
                "working tree must be clean",
            ):
                package_fab.validate_clean_repository(REPOSITORY_ROOT)

    def test_each_supported_engine_gets_its_own_descriptor(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            output_directory = Path(temporary_directory)
            for engine_version in package_fab.SUPPORTED_ENGINE_VERSIONS:
                archive_path = package_fab.package_fab_artifact(
                    REPOSITORY_ROOT,
                    output_directory,
                    engine_version,
                )
                with zipfile.ZipFile(archive_path) as archive:
                    descriptor = json.loads(
                        archive.read(
                            "DirectiveUtilities/DirectiveUtilities.uplugin"
                        ).decode("utf-8")
                    )

                self.assertEqual(descriptor["EngineVersion"], f"{engine_version}.0")
                self.assertTrue(descriptor["Installed"])

    def test_archive_contains_one_clean_plugin_folder(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            archive_path = package_fab.package_fab_artifact(
                REPOSITORY_ROOT,
                Path(temporary_directory),
                "5.8",
            )
            with zipfile.ZipFile(archive_path) as archive:
                names = archive.namelist()

        top_level_names = {name.split("/", 1)[0] for name in names}
        path_parts = {part for name in names for part in Path(name).parts}

        self.assertEqual(top_level_names, {package_fab.PLUGIN_NAME})
        self.assertEqual(
            {
                Path(name.rstrip("/")).parts[1]
                for name in names
                if len(Path(name.rstrip("/")).parts) > 1
            },
            set(package_fab.PACKAGE_ENTRIES)
            | set(package_fab.GENERATED_PACKAGE_DIRECTORIES),
        )
        self.assertTrue(
            {
                "DirectiveUtilities/Content/",
                "DirectiveUtilities/DirectiveUtilities.uplugin",
                "DirectiveUtilities/Source/DirectiveUtilitiesRuntime/"
                "DirectiveUtilitiesRuntime.Build.cs",
                "DirectiveUtilities/README.md",
                "DirectiveUtilities/LICENSE",
            }.issubset(names)
        )
        self.assertFalse(
            any(
                name.startswith("DirectiveUtilities/Content/")
                and name != "DirectiveUtilities/Content/"
                for name in names
            )
        )
        self.assertTrue(
            package_fab.FORBIDDEN_DIRECTORY_NAMES.isdisjoint(path_parts)
        )
        self.assertLessEqual(
            max(len(name.rstrip("/")) for name in names),
            package_fab.MAX_PACKAGE_PATH_LENGTH,
        )

    def test_archives_are_reproducible(self) -> None:
        with tempfile.TemporaryDirectory() as first_directory:
            first_archive = package_fab.package_fab_artifact(
                REPOSITORY_ROOT,
                Path(first_directory),
                "5.8",
            )
            first_digest = package_fab.sha256_file(first_archive)

        with tempfile.TemporaryDirectory() as second_directory:
            second_archive = package_fab.package_fab_artifact(
                REPOSITORY_ROOT,
                Path(second_directory),
                "5.8",
            )
            second_digest = package_fab.sha256_file(second_archive)

        self.assertEqual(first_digest, second_digest)

    def test_fab_technical_information_tracks_the_source(self) -> None:
        production_module_directories = (
            REPOSITORY_ROOT / "Source" / "DirectiveUtilitiesRuntime",
            REPOSITORY_ROOT / "Source" / "DirectiveUtilitiesBlueprintNodes",
            REPOSITORY_ROOT / "Source" / "DirectiveUtilitiesEditor",
        )
        class_count = 0
        for module_directory in production_module_directories:
            for header in module_directory.rglob("*.h"):
                header_source = header.read_text(encoding="utf-8-sig")
                class_count += len(re.findall(r"^UCLASS\b", header_source, re.MULTILINE))
        technical_information = (
            REPOSITORY_ROOT / "Distribution" / "Fab" / "TechnicalInformation.md"
        ).read_text(encoding="utf-8")
        descriptor = package_fab.load_descriptor(REPOSITORY_ROOT)

        self.assertIn(
            f"Number of C++ Classes: {class_count} production UCLASS types",
            technical_information,
        )
        for field in (
            "Number of Blueprints:",
            "Network Replicated:",
            "Supported Development Platforms:",
            "Supported Target Build Platforms:",
            "Documentation Link:",
            "Example Project:",
        ):
            self.assertIn(field, technical_information)
        for module in descriptor["Modules"]:
            self.assertIn(
                f"`{module['Name']}` ({module['Type']}",
                technical_information,
            )


if __name__ == "__main__":
    unittest.main()
