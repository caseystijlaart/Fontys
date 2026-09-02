# Embedded Sensing System — Source

## Structure

- `firmware/` — C++ firmware and real-time processing pipeline (CMake, GoogleTest, clang-tidy, cppcheck)
- `pipeline/` — Python prototyping / ML pipeline (pytest, ruff, mypy)

Both directories currently hold a placeholder module (`add`) only to prove the
build/test/lint toolchain end to end; replace it with the real sensor-fusion
code as it's implemented.

## Dev environment

Open the repo in VS Code with the Dev Containers extension and "Reopen in
Container" — `.devcontainer/` provides cmake, g++, clang-tidy, cppcheck,
python3, pytest, ruff, and mypy, matching what CI runs.

## Build & test (firmware)

    cmake -S firmware -B firmware/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build firmware/build
    ctest --test-dir firmware/build --output-on-failure

## Lint (firmware)

    clang-tidy -p firmware/build firmware/src/*.cpp
    cppcheck --enable=warning,style,performance,portability -I firmware/include firmware/src

## Test & lint (pipeline)

    cd pipeline
    pytest
    ruff check .
    mypy src

CI runs all of the above on every push/PR — see `.github/workflows/ci.yml`.
