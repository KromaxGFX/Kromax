# KromaxGFX — Reproducible Linux Build Environment
#
# Usage:
#   docker build -t kromaxgfx:dev .
#   docker run --rm -v $(pwd):/src kromaxgfx:dev bash -c "cmake --preset linux-debug && cmake --build --preset linux-debug && ctest --preset linux-debug"

FROM ubuntu:24.04

# ---------------------------------------------------------------------------
# Metadata
# ---------------------------------------------------------------------------
LABEL maintainer="XGGMasterX <gianfrancoenrique95@gmail.com>"
LABEL description="KromaxGFX reproducible build environment"
LABEL version="0.1.0"

# ---------------------------------------------------------------------------
# Non-interactive apt
# ---------------------------------------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive

# ---------------------------------------------------------------------------
# Base dependencies
# ---------------------------------------------------------------------------
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    ca-certificates \
    curl \
    git \
    gnupg \
    lsb-release \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# LLVM/Clang 17 (for sanitizers, clang-tidy, clang-format)
# ---------------------------------------------------------------------------
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /usr/share/keyrings/llvm.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-17 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    clang-17 \
    clang-tools-17 \
    clang-format-17 \
    clang-tidy-17 \
    libc++-17-dev \
    libc++abi-17-dev \
    lld-17 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-17 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-17 100 \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# GCC 13 (default Ubuntu 24.04) + build tools
# ---------------------------------------------------------------------------
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    build-essential \
    gcc-13 \
    g++-13 \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Vulkan SDK (headers + loader + validation layers)
# ---------------------------------------------------------------------------
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers-dev \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# X11 / Wayland development headers (for future swapchain support)
# ---------------------------------------------------------------------------
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    libx11-dev \
    libxcb1-dev \
    libxcb-randr0-dev \
    libxcb-xfixes0-dev \
    libxcb-shm0-dev \
    libxcb-present-dev \
    libwayland-dev \
    wayland-protocols \
    libdrm-dev \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Testing / analysis tools
# ---------------------------------------------------------------------------
RUN pip3 install --no-cache-dir --break-system-packages \
    cmake-format \
    pre-commit

# ---------------------------------------------------------------------------
# Working directory
# ---------------------------------------------------------------------------
WORKDIR /src

# ---------------------------------------------------------------------------
# Default entrypoint: configure + build + test (linux-debug preset)
# ---------------------------------------------------------------------------
ENTRYPOINT ["/bin/bash", "-c"]
CMD ["cmake --preset linux-debug && cmake --build --preset linux-debug --parallel && ctest --preset linux-debug --output-on-failure"]