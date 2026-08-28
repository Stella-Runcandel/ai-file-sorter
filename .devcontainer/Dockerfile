FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install core build toolchain and project dependencies (strictly endpoint/application dependencies)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    gcc \
    gdb \
    cmake \
    ninja-build \
    pkg-config \
    git \
    ca-certificates \
    curl \
    sudo \
    # Qt6 Core, Gui, Widgets, Network, and tools
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-l10n-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libxkbcommon-dev \
    libgl-dev \
    # Application libraries
    libcurl4-gnutls-dev \
    libjsoncpp-dev \
    libsqlite3-dev \
    libssl-dev \
    libfmt-dev \
    libspdlog-dev \
    libmediainfo-dev \
    libglib2.0-dev \
    libwebp-dev \
    zlib1g-dev \
    # Diagnostics / utilities
    locales \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Setup developer user
RUN if id -u ubuntu >/dev/null 2>&1; then \
        usermod -l vscode ubuntu && groupmod -n vscode ubuntu && usermod -d /home/vscode -m vscode ; \
    else \
        useradd -m -s /bin/bash vscode ; \
    fi \
    && echo "vscode ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

USER vscode
WORKDIR /workspaces/ai-file-sorter
