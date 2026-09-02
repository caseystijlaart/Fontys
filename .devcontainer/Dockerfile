FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git \
    clang-tidy cppcheck \
    libgtest-dev libgmock-dev \
    python3 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --break-system-packages --no-cache-dir pytest==9.1.1 ruff==0.16.5 mypy==2.3.1

WORKDIR /workspace
