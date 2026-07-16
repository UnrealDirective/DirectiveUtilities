# Runtime performance tests

`Performance.DirectiveUtilities.Runtime` records median, minimum, and maximum execution times for array operations, natural sorting, medians, and string matching. The workloads vary collection size, duplicate density, sampling ratio, page size, rotation distance, input order, and candidate count. Performance measurements are informational and do not use fixed pass or fail thresholds.

The project used to run the suite must load the current source version of Directive Utilities. Build its Editor target after each implementation change before running the benchmark.

On macOS or Linux:

```sh
Tests/Performance/run-runtime-benchmarks.sh "/Users/Shared/Epic Games/UE_5.8" "/path/to/Project.uproject" "/path/to/baseline.csv"
```

On Windows:

```powershell
Tests\Performance\run-runtime-benchmarks.ps1 "C:\Program Files\Epic Games\UE_5.8" "C:\path\to\Project.uproject" "C:\path\to\baseline.csv"
```

Run the command before changing an implementation to capture the baseline. After the change, keep the baseline and choose a different output file:

```sh
Tests/Performance/run-runtime-benchmarks.sh \
  "/Users/Shared/Epic Games/UE_5.8" \
  "/path/to/Project.uproject" \
  "/path/to/optimized.csv" \
  "/path/to/baseline.csv"
```

The optimized CSV includes the baseline median, speedup ratio, and percentage change for each matching workload. Positive percentages indicate faster execution.

Compare results from the same machine, engine version, build configuration, and host project. The runners use median timing across seven samples to reduce short-lived system noise. Generated baselines belong under `Build/Performance` and are not committed.
