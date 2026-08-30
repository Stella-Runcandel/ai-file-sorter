# Testing

This page is a short map of the repository's test surfaces. It is meant to
complement, not replace, the detailed case catalog in `TESTS.md`.

## Main test layers

- **Catch2/CTest unit and integration tests**: the normal automated test suite testing endpoint domain models, URL resolution, HTTP transport, multimodal preprocessing, secret masking, and categorization workflows.
- **`TESTS.md`**: detailed case-by-case intent, setup, procedure, and expected outcomes.
- **Production self-test mode**: `--self-test` and `--self-test=whitelist` provide deterministic checks from the built app.

## Common commands

Build and run the normal test suite on Linux:

```text
cmake -S app -B build -DAI_FILE_SORTER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Build and run on Windows:

```powershell
.\app\build_windows.ps1 -Configuration Release -BuildTests -RunTests
```

Run a single Catch2 case:

```text
./build/tests/ai_file_sorter_tests "<test case name or pattern>"
```

On Windows, the direct executable lives under `.\app\build-windows\tests\Release\ai_file_sorter_tests.exe`:

```powershell
.\app\build-windows\tests\Release\ai_file_sorter_tests.exe "<test case name or pattern>"
```

## When to run what

- Endpoint provider or URL resolver changes: run `test_endpoint_url_resolver.cpp` and `test_openai_compatible_provider.cpp`.
- Vision / multimodal changes: run `test_vision_image_preprocessor.cpp`.
- Taxonomy and categorization changes: run `test_dual_path_coexistence.cpp` and `test_e2e_mixed_workload.cpp`.
- Headless or Explorer-adjacent changes: run focused `Headless*` tests and keep `TESTS.md` in sync.
- Updater/feed changes: run the updater and update-feed tests.
- Packaging/build-script changes: validate the relevant platform build path in addition to unit tests.
