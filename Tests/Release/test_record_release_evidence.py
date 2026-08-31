from __future__ import annotations

import hashlib
import tempfile
import unittest
from pathlib import Path

from Tools.Release.aggregate_performance_baselines import BaselineError, aggregate
from Tools.Release.record_release_evidence import (
    EvidenceError,
    artifact_path,
    csv_metadata,
    linux_cross_compile_artifacts,
    resolve_fab_validation_root,
    sha256_file,
    validation_matrix,
)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ReleaseEvidenceTest(unittest.TestCase):
    def test_csv_metadata_stops_at_header(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "result.csv"
            path.write_text(
                "#configuration,Shipping\n#revision,abc123\ncheck,passed\n#ignored,value\n",
                encoding="utf-8",
            )
            self.assertEqual(
                csv_metadata(path),
                {"configuration": "Shipping", "revision": "abc123"},
            )

    def test_sha256_file_hashes_binary_content(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "artifact.bin"
            content = b"directive-utilities-release-evidence"
            path.write_bytes(content)
            self.assertEqual(sha256_file(path), hashlib.sha256(content).hexdigest())

    def test_validation_matrix_records_host_and_linux_checks(self) -> None:
        matrix = validation_matrix("Windows", True)
        self.assertEqual(len(matrix), 10)
        self.assertIn(
            {
                "engine": "5.8",
                "platform": "Win64",
                "configuration": "Shipping",
                "checks": ["editor_automation", "packaged_smoke", "packaged_benchmark"],
            },
            matrix,
        )
        self.assertEqual(
            sum(row["platform"] == "Linux" for row in matrix),
            3,
        )

    def test_artifact_path_keeps_external_paths(self) -> None:
        repository_root = Path("/repo")
        self.assertEqual(
            artifact_path(repository_root / "Build" / "report.json", repository_root),
            "Build/report.json",
        )
        self.assertEqual(
            artifact_path(Path("/artifacts/Linux/plugin.so"), repository_root),
            "/artifacts/Linux/plugin.so",
        )

    def test_fab_validation_root_supports_short_external_paths(self) -> None:
        repository_root = Path("/repo")
        self.assertEqual(
            resolve_fab_validation_root(repository_root, None),
            repository_root / "Build" / "FabValidation",
        )
        self.assertEqual(
            resolve_fab_validation_root(repository_root, Path("/tmp/DUFab")),
            Path("/tmp/DUFab"),
        )

    def test_fab_verifiers_pin_the_host_target_platform(self) -> None:
        windows_script = (
            REPOSITORY_ROOT / "Tools" / "Release" / "verify-fab-artifacts.ps1"
        ).read_text(encoding="utf-8")
        unix_script = (
            REPOSITORY_ROOT / "Tools" / "Release" / "verify-fab-artifacts.sh"
        ).read_text(encoding="utf-8")
        self.assertIn('"-TargetPlatforms=Win64"', windows_script)
        self.assertIn('-TargetPlatforms="$TARGET_PLATFORM"', unix_script)

    def test_windows_fab_verifier_caps_compile_concurrency(self) -> None:
        windows_script = (
            REPOSITORY_ROOT / "Tools" / "Release" / "verify-fab-artifacts.ps1"
        ).read_text(encoding="utf-8")
        windows_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = "$MaxParallelActions"',
            windows_script,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = "false"',
            windows_script,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = $PreviousMaxParallelActions',
            windows_script,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = $PreviousAllowUbaExecutor',
            windows_script,
        )
        self.assertIn(
            'Remove-Item Env:\\UnrealBuildTool_BuildConfiguration__MaxParallelActions',
            windows_script,
        )
        self.assertIn(
            'Remove-Item Env:\\UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor',
            windows_script,
        )
        self.assertNotIn("UBT_EXTRA_ARGS", windows_script)
        self.assertNotIn('"-ubtargs=', windows_script)
        self.assertIn(
            "-MaxParallelActions $WindowsMaxParallelActions",
            windows_gate,
        )

    def test_windows_runtime_runner_checks_generated_path_length(self) -> None:
        windows_runner = (
            REPOSITORY_ROOT / "Tests" / "RuntimeHost" / "Scripts" / "run-windows.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn("$LongestActionPath.Length -ge 260", windows_runner)

    def test_windows_runtime_runner_uses_process_exit_codes(self) -> None:
        windows_runner = (
            REPOSITORY_ROOT / "Tests" / "RuntimeHost" / "Scripts" / "run-windows.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn("[System.Diagnostics.Process]::new()", windows_runner)
        self.assertNotIn("Start-Process", windows_runner)

    def test_windows_runtime_runner_caps_compile_concurrency(self) -> None:
        windows_runner = (
            REPOSITORY_ROOT / "Tests" / "RuntimeHost" / "Scripts" / "run-windows.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn('"-MaxParallelActions=$MaxParallelActions"', windows_runner)
        self.assertIn(
            '"-ubtargs=-NoUBA -MaxParallelActions=$MaxParallelActions"',
            windows_runner,
        )

    def test_windows_release_gate_caps_linux_cross_compile_concurrency(self) -> None:
        windows_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.ps1"
        ).read_text(encoding="utf-8")
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = "$WindowsMaxParallelActions"',
            windows_gate,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = "false"',
            windows_gate,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = $PreviousMaxParallelActions',
            windows_gate,
        )
        self.assertIn(
            '$env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = $PreviousAllowUbaExecutor',
            windows_gate,
        )
        self.assertIn(
            'Remove-Item Env:\\UnrealBuildTool_BuildConfiguration__MaxParallelActions',
            windows_gate,
        )
        self.assertIn(
            'Remove-Item Env:\\UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor',
            windows_gate,
        )
        self.assertNotIn("UBT_EXTRA_ARGS", windows_gate)
        self.assertNotIn('"-ubtargs=', windows_gate)

    def test_mac_packaged_report_uses_container_path(self) -> None:
        unix_runner = (
            REPOSITORY_ROOT / "Tests" / "RuntimeHost" / "Scripts" / "run-unix.sh"
        ).read_text(encoding="utf-8")
        self.assertIn('MAC_GAME_REPORT_ROOT="$MAC_LOG_ROOT/Reports/Game"', unix_runner)
        self.assertIn('rsync -a "$MAC_GAME_REPORT_ROOT/" "$REPORT_ROOT/Game/"', unix_runner)

    def test_release_gates_require_two_clean_performance_retries(self) -> None:
        windows_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.ps1"
        ).read_text(encoding="utf-8")
        unix_gate = (
            REPOSITORY_ROOT / "Tools" / "Release" / "run-local-release-gate.sh"
        ).read_text(encoding="utf-8")
        for gate in (windows_gate, unix_gate):
            self.assertIn("candidate-attempt-2.csv", gate)
            self.assertIn("candidate-attempt-3.csv", gate)
            self.assertIn("aggregate_performance_baselines.py", gate)

    def test_performance_baseline_uses_median_across_runs(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = []
            for index, median in enumerate((4.0, 40.0, 6.0), start=1):
                path = root / f"run-{index}.csv"
                path.write_text(
                    "#engine,5.8\n"
                    "#platform,Mac\n"
                    "#revision,abc123\n"
                    "benchmark,element_count,parameter,median_ms,min_ms,max_ms,samples,baseline_median_ms,speedup,change_percent\n"
                    f"GetDistinct,1000,0,{median},{median - 1},{median + 1},7,,,\n",
                    encoding="utf-8",
                )
                inputs.append(path)
            output = root / "baseline.csv"
            aggregate(inputs, output)
            contents = output.read_text(encoding="utf-8")
            self.assertIn("#aggregate_runs,3", contents)
            self.assertIn("GetDistinct,1000,0,6.000000000,3.000000000,41.000000000,21", contents)

    def test_performance_baseline_rejects_mismatched_rows(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = []
            for index in range(3):
                path = root / f"run-{index}.csv"
                benchmark = "GetDistinct" if index < 2 else "GetMostCommon"
                path.write_text(
                    "#engine,5.8\n"
                    "benchmark,element_count,parameter,median_ms,min_ms,max_ms,samples,baseline_median_ms,speedup,change_percent\n"
                    f"{benchmark},1000,0,5,4,6,7,,,\n",
                    encoding="utf-8",
                )
                inputs.append(path)
            with self.assertRaisesRegex(BaselineError, "rows do not match"):
                aggregate(inputs, root / "baseline.csv")

    def test_performance_baseline_rejects_invalid_measurements(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = []
            for index in range(3):
                path = root / f"run-{index}.csv"
                path.write_text(
                    "#engine,5.8\n"
                    "benchmark,element_count,parameter,median_ms,min_ms,max_ms,samples,baseline_median_ms,speedup,change_percent\n"
                    "GetDistinct,1000,0,nan,4,6,7,,,\n",
                    encoding="utf-8",
                )
                inputs.append(path)
            with self.assertRaisesRegex(BaselineError, "Invalid performance value"):
                aggregate(inputs, root / "baseline.csv")

    def test_linux_evidence_requires_both_configurations(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            package_root = Path(directory)
            for configuration in ("Development", "Shipping"):
                object_path = (
                    package_root
                    / "Intermediate"
                    / "Build"
                    / "Linux"
                    / "x64"
                    / "UnrealGame"
                    / configuration
                    / "DirectiveUtilitiesRuntime"
                    / "Module.cpp.o"
                )
                object_path.parent.mkdir(parents=True, exist_ok=True)
                object_path.write_bytes(configuration.encode())
            self.assertEqual(len(linux_cross_compile_artifacts(package_root)), 2)

    def test_linux_evidence_rejects_a_missing_configuration(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(EvidenceError, "Development"):
                linux_cross_compile_artifacts(Path(directory))


if __name__ == "__main__":
    unittest.main()
