<!-- markdownlint-disable MD046 -->
# AI File Sorter

[![Code Version](https://img.shields.io/badge/Code-1.9.2-blue)](#)
[![Release Version](https://img.shields.io/github/v/release/hyperfield/ai-file-sorter?label=Release)](#)
![filesorter.app Downloads](https://filesorter.app/download-stats/badge.svg)
[![SourceForge Downloads](https://img.shields.io/sourceforge/dt/ai-file-sorter.svg?label=SourceForge%20downloads)](https://sourceforge.net/projects/ai-file-sorter/files/latest/download)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2c646c836a9844be964fbf681649c3cd)](https://app.codacy.com/gh/hyperfield/ai-file-sorter/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Donate](https://img.shields.io/badge/Support%20AI%20File%20Sorter-orange)](https://filesorter.app/donate/)

<p align="center">
  <img src="app/resources/images/icon_256x256.png" alt="AI File Sorter logo" width="128" height="128">
</p>

<p align="center">
  <img src="images/platform-logos/logo-windows.png" alt="Windows" width="160">
  <img src="images/platform-logos/logo-macos.png" alt="macOS" width="160">
  <img src="images/platform-logos/logo-linux.png" alt="Linux" width="160">
</p>

AI File Sorter is a cross-platform desktop application that uses AI to organize files and suggest cleaner, more consistent names for images, documents, and supported audio/video files. It is designed to reduce clutter, improve consistency, and make files easier to find later, whether for review, archiving, or long-term storage.

<p align="center">
  <img src="images/screenshots/aifs-main.png" alt="AI File Sorter main window on Windows" width="900">
</p>

The app analyzes files using standard OpenAI-compatible endpoints, Google Gemini, or self-hosted local AI servers (such as Ollama, LM Studio, or vLLM). It suggests meaningful, human-readable names: for example, a generic file like IMG_2048.jpg can be renamed to something descriptive such as `clouds_over_lake.jpg`. It can also analyze supported document files and propose clearer names based on their text content. AI File Sorter can also clean up messy audio and video filenames by using the metadata already stored inside supported media files. If tags such as year, artist, album, or title are available, the app can turn them into a clear suggestion like `2024_artist_album_title.mp3`, which you can review, edit, or ignore before any change is applied.

AI File Sorter helps tidy up cluttered folders such as Downloads, external drives, or NAS storage by grouping files based on their names, file types, folder context, and past sorting results.

Instead of relying only on fixed rules, the app combines AI suggestions with optional whitelists, recent similar results, and your approved review decisions. This helps keep sorting more consistent over time while still letting you review and adjust everything before anything is changed.

Categories (and optional subcategories) are suggested for each file, and for supported file types, rename suggestions are provided as well. Once you confirm, the required folders are created automatically and files are sorted accordingly.

Privacy & flexibility by design:
AI File Sorter uses a clean endpoint architecture. You can connect it to cloud APIs (OpenAI, Gemini) or to private local servers (Ollama, LM Studio, LocalAI) where your files stay entirely on your local machine/network. No telemetry is sent.

---

#### How It Works

1. Point the app at a folder or drive  
2. Files (and image content, when applicable) are analyzed using your configured AI endpoint or media tag extractors
3. Category and rename suggestions are generated  
4. You review and adjust if needed before anything is changed  

---

## Safe First Run

If you are trying AI File Sorter for the first time, start with a small test folder instead of a full archive or drive. Copy 20-50 files from `Downloads`, screenshots, photos, or documents into a temporary folder, run the analysis, and inspect the review table before applying anything.

This keeps the first run low risk: the AI only suggests categories and filenames, and no move or rename happens until you approve it. If you do apply changes and then want to reverse them, use **Edit -> Undo last run**.

---

[![Download ai-file-sorter](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/ai-file-sorter/files/latest/download)

[![Get it from Microsoft](https://get.microsoft.com/images/en-us%20dark.svg)](https://apps.microsoft.com/detail/9npk4dzd6r6s)

<p align="center">
  <img src="images/screenshots/aifs-analyzing.png" alt="AI File Sorter analysis progress dialog on Windows" width="49%" align="top" style="vertical-align: top;">
  <img src="images/screenshots/aifs-done-linux.png" alt="AI File Sorter review and confirm results on Linux" width="49%" align="top" style="vertical-align: top;">
</p>

<p align="center">
  <img src="images/screenshots/aifs-benchmark-dialog-macos.png" alt="AI File Sorter compatibility benchmark on macOS" width="49%" align="top" style="vertical-align: top;">
  <img src="images/screenshots/aifs-done-2.png" alt="AI File Sorter review and confirm results with mixed file types on Windows" width="49%" align="top" style="vertical-align: top;">
</p>

---

- [AI File Sorter](#ai-file-sorter)
  - [Safe First Run](#safe-first-run)
  - [Technical reference](#technical-reference)
  - [Changelog](#changelog)
  - [Features](#features)
  - [Categorization](#categorization)
    - [Categorization modes](#categorization-modes)
    - [Category language selection](#category-language-selection)
    - [Category whitelists](#category-whitelists)
  - [Image analysis (Multimodal Vision)](#image-analysis-multimodal-vision)
    - [Main window options](#main-window-options)
  - [Document analysis (Text LLM)](#document-analysis-text-llm)
    - [Supported document formats](#supported-document-formats)
    - [Main window options (documents)](#main-window-options-documents)
  - [Audio/video metadata filename suggestions](#audiovideo-metadata-filename-suggestions)
    - [Supported audio/video formats](#supported-audiovideo-formats)
  - [Requirements](#requirements)
  - [Installation](#installation)
    - [Linux](#linux)
    - [macOS](#macos)
    - [Windows](#windows)
  - [Testing](#testing)
  - [Environment Variables](#environment-variables)
  - [Categorization cache and learned behavior](#categorization-cache-and-learned-behavior)
  - [Uninstallation](#uninstallation)
  - [Using your OpenAI API key](#using-your-openai-api-key)
  - [Using your Gemini API key](#using-your-gemini-api-key)
  - [Using a custom OpenAI-compatible API](#using-a-custom-openai-compatible-api)
  - [Diagnostics](#diagnostics)
  - [Help and onboarding](#help-and-onboarding)
  - [How to Use](#how-to-use)
  - [Sorting a Remote Directory (e.g., NAS)](#sorting-a-remote-directory-eg-nas)
  - [Contributing](#contributing)
  - [Credits](#credits)
  - [License](#license)
  - [Donation](#donation)

---

## Technical reference

The main README stays focused on installation, features, and normal everyday
use. For contributor-facing and integration-facing details that are too deep for
the main entry page, use these technical references:

- [Architecture](docs/architecture.md)
- [Headless runtime contract](docs/headless-runtime-contract.md)
- [Configuration and environment](docs/configuration-and-environment.md)
- [Categorization behavior](docs/categorization-behavior.md)
- [Testing](docs/testing.md)
- [Updater contract](docs/updater-contract.md)

---

## Changelog

## [1.9.2] - 2026-08-14

- Fixed a Windows startup crash in Qt GUI theme refresh handling.

## [1.9.1] - 2026-08-06

- Fixed bundled Windows local-LLM runtime builds so the packaged GGML libraries run on generic SSE4.2-capable x64 CPUs instead of requiring AVX2.
- Fixed embedded CA bundle staging for packaged and Microsoft Store builds by writing the certificate bundle under writable app data instead of the read-only install directory.

## [1.9.0] - 2026-07-03

- Added smart branching whitelists so categories can have their own allowed subcategories, with an improved whitelist editor that keeps global and category-specific subcategory modes mutually exclusive.
- Added structured project-folder protection for recursive scans, covering Unity, Unreal, Godot, Blender, Git repositories, and common source-code project layouts.
- Added file preview support in the Categorization Review dialog and improved accessibility labels/progress announcements for screen readers.
- Added custom visual model support, configurable local model storage, and improved reuse of already-downloaded Gemma 3 4B model files.
- Improved category and filename consistency by localizing suggested filenames, preserving UTF-8 metadata, stripping inline subcategory artifacts, and keeping date suffixes out of canonical cache labels.
- Improved remote LLM handling with rate-limit/backoff parsing and optional request pacing.
- Improved local runtime and release packaging reliability across Windows, Linux, and macOS, including safer backend probing, CUDA/Vulkan fallback handling, RPM packaging, and verified macOS release helpers.

## [1.8.0] - 2026-05-10

- Added backend status indicator to the status bar.
- The app now runs as a single instance - opening it again brings the existing window to the front instead of starting a second copy.
- Restored the app launcher for the non-Microsoft Store versions of the app and improved GPU selection, now preferring CUDA over Vulkan when both are available.
- Improved local GPU startup and local visual model handling for better reliability and compatibility.
- Added Gemma 3 4B IT and set it as the default visual model.
- Added Gemma 3 4B IT and Gemma 1.1 7B as built-in local categorization model choices, replacing LLaMa 3B.
- Improved image categorization quality and consistency by preserving image descriptions, using richer prompt context, adding special handling for screenshots and UI captures, and reducing drift between category labels.
- Improved image analysis stability, fallback behavior, and model-download validation.
- Added options to clear categorization and app caches, including a deeper reset of stored categorization state.
- Added local learning from your review decisions to improve future suggestions.
- Added localized Quick Start help, an FAQ link, and additional interface languages including Hindi, Swedish, Icelandic, Norwegian, Finnish, Danish, and Simplified Chinese.

See [CHANGELOG.md](CHANGELOG.md) for the full history.

---

## Features

- **Endpoint AI-powered categorization**: Sort files using cloud AI providers (OpenAI, Google Gemini) or private self-hosted OpenAI-compatible servers (Ollama, LM Studio, vLLM, LocalAI).
- **Offline & Private**: Connect to local self-hosted servers (e.g. Ollama on `localhost:11434`) so files and metadata never leave your machine or local network — zero API keys required.
- **Robust categorization**: Built-in rules and category matching help keep results consistent across runs.
- **Configurable categorization controls**: Use whitelists, taxonomy normalization, consistency modes, and review-time edits to steer categories and subcategories.
- **Two categorization modes**: Pick **More Refined** for specific labels with less pressure to stay in broad default categories, or **More Consistent** for steadier top-level categories across similar files.
- **Category whitelists**: Define named whitelists of allowed categories/subcategories, including smart branching lists where each main category has its own allowed subcategories. Manage them under **Settings → Manage category whitelists…**, then toggle/select them in the main window when you want to constrain model output for a session.
- **Category and rename languages**: Categories are chosen in English behind the scenes and then shown in your selected category language. Suggested filenames for images, documents, and supported audio/video files are localized the same way.
- **Custom OpenAI-compatible endpoints**: Add and manage custom endpoints (Ollama, LM Studio, LocalAI, vLLM, or corporate gateways) with base URL, model name, and optional API keys directly from the **Select LLM** dialog.
- **Image content analysis (Multimodal Vision)**: Analyze supported picture files using vision-capable models (such as GPT-4o-mini, Gemini, or Ollama LLaVA/vision models). Images are preprocessed and downscaled client-side (max 2048x2048) in-memory before sending.
- **Image date-to-category suffix (optional)**: Append image creation date metadata to image category names when available.
- **Document content analysis (Text LLM)**: Analyze supported document files to summarize content and suggest filenames using the configured AI endpoint.
- **Audio/video metadata filename suggestions**: Turn embedded media tags into clean, library-style filenames for supported audio and video files, with full review before anything is renamed.
- **Sortable review**: Sort the Categorization Review table by file name, category, or subcategory to triage faster.
- **Qt6 Interface**: Lightweight and responsive UI with refreshed menus and native dark theme support.
- **Interface languages**: English, Danish, Dutch, Finnish, French, German, Hindi, Icelandic, Italian, Korean, Norwegian, Simplified Chinese, Spanish, Swedish, and Turkish.
- **Cross-Platform Compatibility**: Native support on Windows, macOS, and Linux.
- **Local Database Caching**: Speeds up repeated categorization, preserves approved labels and rename suggestions, and provides recent-category hints for consistency.
- **Local learning from approved reviews**: Approved category decisions can be stored locally and reused as hints for future runs without modifying the underlying model.
- **Cache maintenance tools**: Use **Settings → Clear cache…** to inspect and clear categorization cache, image location cache, and logs, or **Settings → Reset learned behavior…** to remove the separate learned-review database.
- **Sorting Preview**: See how files will be organized before confirming changes.
- **Dry run** / preview-only mode to inspect planned moves without touching files.
- **Persistent Undo** ("Undo last run") even after closing the sort dialog.
- **Project-folder protection**: Recursive scans skip recognized structured project roots such as Unity, Unreal, Godot, Blender project folders, Git repositories, and common source-code projects so files that depend on project-relative paths are not moved independently.
- **Bring your own remote credentials**: Store your OpenAI key, Gemini key, or custom OpenAI-compatible endpoint details locally for reuse in later runs.
- **Update Notifications**: Get notified about updates - with optional or required update flows.
- **Storage plugin support**: Install provider-specific compatibility modes from the **Plugins** menu when the app detects supported cloud-backed folders.
- **In-app help**: Open the localized **Help → Quick Start Guide** for a guided walkthrough or **Help → FAQ** for troubleshooting and common questions.

---

## Categorization

### Categorization modes

- **More refined**: The flexible, detail-oriented mode. It is less tied to the default category structure, so it can choose a more specific top-level category when that fits better. For example: `Security / PCI DSS`, `Manuals / Camera Guides`, or `Wildlife / Lions`. This works well for mixed, specialized, or long-tail folders.
- **More consistent**: The uniform mode. It uses recent similar results to favor stable broad folder categories, so related files are more likely to land under the same top-level folders. For example: `Documents / PCI DSS`, `Documents / Camera Guides`, or `Images / Lions`. This works well when you want a cleaner, more uniform folder layout.
- Switch between the two via the **Categorization type** radio buttons on the main window; your choice is saved for the next run.

Example without a whitelist:

```text
More refined
- pci_dss_quick_reference.pdf -> Security / PCI DSS
- camera_setup_manual.pdf -> Manuals / Camera Guides
- lion_photo.jpg -> Wildlife / Lions

More consistent
- pci_dss_quick_reference.pdf -> Documents / PCI DSS
- camera_setup_manual.pdf -> Documents / Camera Guides
- lion_photo.jpg -> Images / Lions
```

### Category language selection

- Category labels are chosen in English first behind the scenes, then shown in the category language you selected under **Settings → Category language**.
- Suggested filenames for image, document, and supported audio/video rename flows are also localized into the selected **Settings → Category language** target before review/apply.
- The menu is grouped into alphabetical submenus to keep it usable on smaller screens.

### Category whitelists

- Enable **Use a whitelist** to apply the selected category list during categorization; disable it to let the model choose more freely.
- Manage lists (add, edit, remove) under **Settings → Manage category whitelists…**. Built-in `Default` and `Documents` lists are auto-created only when no lists exist, and multiple named lists can be kept for different projects.
- The whitelist editor has three sections. **Main categories / top-level folders** defines the destination category folders. **Global subcategories** defines subcategories that may be used under any main category. **Category-specific subcategories** defines smart branching rows where each main category has its own allowed subcategories.
- **Global subcategories** and **Category-specific subcategories** are alternatives. If you enter values in one section, the other section is disabled/ignored for that whitelist so the app does not mix two incompatible constraint styles.
- Smart branching is useful when the same broad top-level folders should contain different allowed subfolders. For example, `Documents -> Invoices, Receipts, Taxes` and `Images -> Screenshots, Photos` tells the app that `Screenshots` is valid under `Images`, but not under `Documents`.
- The built-in `Documents` whitelist uses smart branching: the only top-level category is `Documents`, while topics such as invoices, receipts, taxes, contracts, reports, and notes are stored as `Documents` subcategories.
- Large and smart branching whitelists are dynamically filtered to relevant candidates plus valid category/subcategory pair guidance in the prompt.
- Whitelists apply in either categorization mode; pair them with **More consistent** when you want the strongest adherence to a constrained vocabulary.

---

## Image analysis (Multimodal Vision)

Image analysis sends preprocessed image data to a vision-capable AI endpoint (such as OpenAI `gpt-4o-mini`, Gemini, or a local Ollama vision model) to understand what a picture shows and suggest a better category or filename.

The client decodes images in-memory, downscales them to a maximum of 2048x2048 pixels, encodes them to JPEG/PNG format, and transmits them as standard Base64 Data URIs in the multimodal chat payload.

### Main window options

Image analysis adds related checkboxes to the main window:

- **Analyze picture files by content**: Runs multimodal vision analysis on supported picture files and reports progress in the analysis dialog.
- **Process picture files only (ignore any other files)**: Restricts the run to supported picture files and disables the categorization controls while active.
- **Add image creation date (if available) to category name**: Appends `YYYY-MM-DD` from image metadata to the category label when available. Disabled when rename-only is enabled.
- **Add photo date and place to filename (if available)**: Adds metadata-based date/place prefixes to suggested image filenames when available.
- **Offer to rename picture files**: Shows a **Suggested filename** column in the Review dialog with the AI proposal. You can edit it before confirming.
- **Do not categorize picture files (only rename)**: Skips text categorization for images and keeps them in place while applying (optional) renames.

The separate top-level checkbox **Add audio/video metadata to file name (if available)** controls metadata-based rename suggestions for supported audio/video files. See [Audio/video metadata filename suggestions](#audiovideo-metadata-filename-suggestions).

---

## Document analysis (Text LLM)

Document analysis uses the configured AI endpoint to extract text from supported document files, summarize content, and optionally suggest a better filename.

### Supported document formats

- Plain text: `.txt`, `.md`, `.rtf`, `.csv`, `.tsv`, `.json`, `.xml`, `.yml`/`.yaml`, `.ini`/`.cfg`/`.conf`, `.log`, `.html`/`.htm`, `.tex`, `.rst`
- PDF: `.pdf` (embedded PDFium by default; CLI fallback via `pdftotext` is available only if you explicitly configure `-DAI_FILE_SORTER_REQUIRE_EMBEDDED_PDF_BACKEND=OFF`)
- Office/OpenOffice: `.docx`, `.xlsx`, `.pptx`, `.odt`, `.ods`, `.odp` (embedded libzip+pugixml)
- Legacy binary formats like `.doc`, `.xls`, `.ppt` are not currently supported.

### Main window options (documents)

- **Analyze document files by content**: Extracts document text and feeds it into the LLM for summary + rename suggestion.
- **Process document files only (ignore any other files)**: Restricts the run to supported document files and disables the categorization controls while active.
- **Offer to rename document files**: Shows a **Suggested filename** column in the Review dialog with the LLM proposal. You can edit it before confirming.
- **Do not categorize document files (only rename)**: Skips text categorization for documents and keeps them in place while applying (optional) renames.
- **Add document creation date (if available) to category name**: Appends `YYYY-MM` from metadata when available. Disabled when rename-only is enabled.

---

## Audio/video metadata filename suggestions

Let AI File Sorter turn embedded media tags into clean, consistent filenames for your music and video library. When enabled, the app reads supported metadata fields and builds a polished suggested name in the format `year_artist_album_title.ext`. As with all rename suggestions, nothing is changed until you review and confirm it.

### Supported audio/video formats

- Audio extensions: `.aac`, `.aif`, `.aiff`, `.alac`, `.ape`, `.flac`, `.m4a`, `.mp3`, `.ogg`, `.oga`, `.opus`, `.wav`, `.wma`
- Video extensions: `.3gp`, `.avi`, `.flv`, `.m4v`, `.mkv`, `.mov`, `.mp4`, `.mpeg`, `.mpg`, `.mts`, `.m2ts`, `.ts`, `.webm`, `.wmv`
- Built-in tag readers currently cover MP3 (`ID3v1`/`ID3v2`), FLAC (Vorbis comments), OGG/OGA/Opus (Vorbis comments), and MP4-family containers such as `.m4a`, `.mp4`, `.m4v`, `.mov`, and `.3gp` (MP4/MOV metadata atoms).
- When compiled with package-managed `MediaInfoLib`, the same rename flow can also use metadata exposed by MediaInfo for additional supported containers when available.

---

## Requirements

- **Operating System**: Linux, macOS, or Windows.
- **Compiler**: A C++20-capable compiler (`g++` or `clang++` on Linux/macOS, MSVC 2022/2026 on Windows).
- **Qt 6**: Core, Gui, Widgets, Network, and Sql modules (`qt6-base-dev` / `qt6-tools` on Linux, `brew install qt` on macOS, or `qtbase` via vcpkg on Windows).
- **Libraries**: `curl`, `sqlite3`, `openssl`, `fmt`, `spdlog`, `libmediainfo`, `libwebp`, and `zlib`. On Windows, dependencies are restored automatically in manifest mode via project-local `.tools\vcpkg`.
- **MediaInfo policy**: MediaInfo must be installed through a package manager (`apt`/`dnf`/`pacman`/`brew`/`vcpkg`). The build rejects vendored MediaInfo submodules and checked-in binaries.
- **Document analysis libraries**: PDFium (vendored) and libzip.
- **AI Runtime**: No embedded GGUF models, llama.cpp, CUDA, Vulkan, or Metal runtimes required. All AI interactions go through HTTP/HTTPS endpoints.

---

## Installation

File categorization using local servers (like Ollama or LM Studio) is completely free of charge. If you use cloud providers (OpenAI or Google Gemini), you supply your own API key (see [Using your OpenAI API key](#using-your-openai-api-key), [Using your Gemini API key](#using-your-gemini-api-key), or [Using a custom OpenAI-compatible API](#using-a-custom-openai-compatible-api)).

### Linux

#### Prebuilt Debian/Ubuntu package

1. **Install runtime prerequisites** (Qt6, networking, database, math libraries):
   - Ubuntu 24.04 / Debian 12:
     ```bash
     sudo apt update && sudo apt install -y \
       libqt6widgets6 libcurl4 libjsoncpp25 libfmt9 libopenblas0-pthread \
       libvulkan1 mesa-vulkan-drivers patchelf
     ```
   - Debian 13 (trixie):
     ```bash
     sudo apt update && sudo apt install -y \
       libqt6widgets6 libcurl4t64 libjsoncpp26 libfmt10 libopenblas0-pthread \
       libvulkan1 mesa-vulkan-drivers patchelf
     ```
   If you build the Vulkan backend from source, install `glslc` (Debian/Ubuntu package: `glslc`; on some distros: `shaderc` or `shaderc-tools`).
   On Debian 13, use `libjsoncpp26`, `libfmt10`, and `libcurl4t64` (APT may auto-select `libcurl4t64` if `libcurl4` is not available).
   Ensure that the Qt platform plugins are installed (on Ubuntu 22.04 this is provided by `qt6-wayland`).
   GPU acceleration additionally requires either a working Vulkan 1.2+ stack (Mesa, AMD/Intel/NVIDIA drivers) or, for NVIDIA users, the matching CUDA runtime (`nvidia-cuda-toolkit` or vendor packages). The launcher automatically prefers CUDA when both are present and falls back to CPU if neither is available.
2. **Install the package**
   ```bash
   sudo apt install ./aifilesorter_*.deb
   ```
   Using `apt install` (rather than `dpkg -i`) ensures any missing dependencies listed above are installed automatically.

#### Build from source

1. **Install dependencies**
   - Debian / Ubuntu:
    ```bash
    sudo apt update && sudo apt install -y \
      build-essential cmake git qt6-base-dev qt6-base-dev-tools qt6-l10n-tools qt6-tools-dev-tools \
      libcurl4-openssl-dev libjsoncpp-dev libsqlite3-dev libssl-dev libfmt-dev libspdlog-dev libmediainfo-dev \
      zlib1g-dev patchelf
    ### Linux

#### Prerequisites & Installation

1. **Install system dependencies**:
   - **Ubuntu 24.04 / Debian 12**:
     ```bash
     sudo apt update && sudo apt install -y \
       build-essential cmake ninja-build pkg-config git \
       qt6-base-dev qt6-tools-dev libcurl4-openssl-dev \
       libjsoncpp-dev libsqlite3-dev libssl-dev libfmt-dev \
       libspdlog-dev libmediainfo-dev libwebp-dev zlib1g-dev
     ```
   - **Fedora 40+ / RHEL 9+**:
     ```bash
     sudo dnf install -y \
       gcc-c++ cmake ninja-build git pkgconf-pkg-config \
       qt6-qtbase-devel qt6-qttools-devel libcurl-devel \
       jsoncpp-devel sqlite-devel openssl-devel fmt-devel \
       spdlog-devel libmediainfo-devel libwebp-devel zlib-devel
     ```
   - **Arch / Manjaro**:
     ```bash
     sudo pacman -S --needed \
       base-devel git cmake ninja pkgconf qt6-base qt6-tools \
       curl jsoncpp sqlite openssl fmt spdlog mediainfo libwebp zlib
     ```

2. **Clone the repository and submodules**:
   ```bash
   git clone https://github.com/hyperfield/ai-file-sorter.git
   cd ai-file-sorter
   git submodule update --init --recursive
   ```

3. **Configure and build with CMake**:
   ```bash
   cmake -S app -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ```

4. **Run the application**:
   ```bash
   ./build/aifilesorter
   ```

5. **Run the unit tests**:
   ```bash
   ctest --test-dir build --output-on-failure
   ```

*(Alternatively, use VS Code with the provided `.devcontainer` configuration for a 1-click isolated build environment).*

---

### macOS

Apple Silicon Macs running macOS 14 or later are supported for macOS builds.

1. **Install Xcode Command Line Tools**: `xcode-select --install`
2. **Install dependencies via Homebrew**:
   ```bash
   brew install qt cmake ninja pkg-config curl jsoncpp sqlite openssl fmt spdlog mediainfo webp zlib
   ```
3. **Configure and build**:
   ```bash
   export PATH="$(brew --prefix)/opt/qt/bin:$PATH"
   export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig:$PKG_CONFIG_PATH"
   cmake -S app -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ./build/aifilesorter
   ```

---

### Windows

Native Windows compilation uses MSVC and project-local vcpkg dependencies in manifest mode.

#### Prerequisites

- **Visual Studio 2022 or 2026** (Community, Professional, or Enterprise) with the **Desktop development with C++** workload (including MSVC compiler, CMake, and Windows 10/11 SDK).
- **PowerShell 7** (`pwsh`) or Windows PowerShell.
- **No global installations or PATH changes are needed.** The build script manages all third-party libraries locally under `.tools\vcpkg`.

#### Build Steps

1. **Clone repository and submodules**:
   ```powershell
   git clone https://github.com/hyperfield/ai-file-sorter.git
   cd ai-file-sorter
   git submodule update --init --recursive
   ```

2. **Build using the Windows build script**:
   Open a PowerShell terminal and run:
   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   .\app\build_windows.ps1 -Configuration Release -BuildTests
   ```

   - Dependencies in `app/vcpkg.json` are restored automatically to `app\build-windows-vcpkg_installed`.
   - The primary development executable is created at `app\build-windows\Release\aifilesorter.exe`.
   - Required Qt plugins and runtime DLLs are deployed beside the executable automatically via `windeployqt`.

3. **Run the application**:
   ```powershell
   .\app\build-windows\Release\aifilesorter.exe
   ```

4. **Run the test suite**:
   ```powershell
   .\app\build-windows\tests\Release\ai_file_sorter_tests.exe
   ```

---

## Testing

Catch2-based unit tests verify the endpoint domain models, URL resolution, multimodal payload formatting, secret masking, and categorization pipeline.

### Running unit tests on Linux/macOS

```bash
cmake -S app -B build -DAI_FILE_SORTER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Running unit tests on Windows

```powershell
.\app\build_windows.ps1 -Configuration Release -BuildTests -RunTests
```

Or execute specific test cases directly:
```powershell
.\app\build-windows\tests\Release\ai_file_sorter_tests.exe "<test-pattern>"
```

---

## Environment Variables

### Endpoint Timeouts & Pacing

- `AI_FILE_SORTER_REMOTE_LLM_TIMEOUT` - Timeout in seconds for OpenAI/Gemini API calls (default 30).
- `AI_FILE_SORTER_CUSTOM_LLM_TIMEOUT` - Timeout in seconds for custom OpenAI-compatible endpoints (default 60).
- `AI_FILE_SORTER_REMOTE_REQUESTS_PER_MINUTE` - Optional client-side request pacing limit (e.g. `20` limits calls to 20/min). Set to `0` or unset to disable pacing.

### Storage & Cache

- `AI_FILE_SORTER_CONFIG_DIR` - Override base configuration directory where `config.ini` and databases reside.
- `CATEGORIZATION_CACHE_FILE` - Override the SQLite categorization database filename.

### Updater Configuration

- `UPDATE_SPEC_FILE_URL` - Primary update feed URL.
- `UPDATE_SPEC_FILE_URL_DEVELOPMENT` - Development update feed URL.

Example update feed:

```json
{
  "update": {
    "current_version": "1.7.1",
    "min_version": "1.6.0",
    "download_url": "https://filesorter.app/download",
    "changelog": [
      "General compatibility fixes for older clients"
    ],
    "windows": {
      "current_version": "1.7.1",
      "min_version": "1.6.0",
      "download_url": "https://filesorter.app/download",
      "changelog": [
        "Improved installer handoff on Windows",
        "Added more update details in the dialog"
      ],
      "installer_url": "https://filesorter.app/downloads/AIFileSorterSetup-1.7.1.exe",
      "installer_sha256": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    },
    "macos": {
      "current_version": "1.7.1",
      "min_version": "1.6.0",
      "download_url": "https://filesorter.app/download",
      "changelog": [
        "Updated notarized package metadata"
      ]
    },
    "linux": {
      "current_version": "1.7.1",
      "min_version": "1.6.0",
      "download_url": "https://filesorter.app/download",
      "changelog": [
        "Improved Linux wrapper backend selection"
      ]
    }
  }
}
```

Compatibility note:

- Older app versions only read the flat top-level fields under `update`, so keep `current_version`, `min_version`, and `download_url` there as a legacy compatibility stream if you still need to support them.
- Newer app versions prefer the platform-specific streams and will use `update.windows`, `update.macos`, or `update.linux` when present.
- The legacy compatibility stream can only represent one generic stream, not separate per-platform versions or installers.
- `changelog` is evaluated per stream. Use a JSON array of strings for new feeds; each entry is shown as a bullet item in the update dialog for that stream.

Windows-only direct installer updates:

- `installer_url` - direct URL to the Windows installer package.
- `installer_sha256` - SHA-256 checksum used to verify the downloaded installer before launch.
- `installer_url` can now also point to a ZIP archive, as long as the archive contains exactly one installer payload (`.exe` or `.msi`).
- When both fields are present on Windows, the app can download the installer, verify it, and then prompt: `Quit the app and launch the installer to update`.

Development feed selection:

- When the app starts with `--development`, the updater prefers `UPDATE_SPEC_FILE_URL_DEVELOPMENT`.
- If `UPDATE_SPEC_FILE_URL_DEVELOPMENT` is unset, development mode falls back to `UPDATE_SPEC_FILE_URL`.

GUI test mode:

- `--test` launches the normal app window, implies development mode, and adds a **Tests** menu.
- The first test preset is **Run large whitelist LLM test…**. It creates a sample folder, configures a large transient category whitelist, and starts the normal analysis flow with the currently selected real LLM.
- The large-whitelist preset is meant for manual/runtime validation: inspect the Review dialog to see whether the real LLM selected the expected broad categories from the compact whitelist candidates.
- Test mode uses the user's selected LLM configuration but stores test-mode whitelists, categorization cache, learned behavior, undo data, and sample files under an isolated `test_mode_profile` directory inside the normal config directory.
- Test mode does not save normal app settings on shutdown, so the preset folder/whitelist selection should not replace the user's ordinary configuration.

Headless self-test mode:

- `--self-test` runs deterministic self-tests from the production executable and exits with a pass/fail status instead of opening the main window.
- `--self-test=whitelist` runs the deterministic large-whitelist suite explicitly. `--self-test=whitelists` is accepted as an alias.
- The headless whitelist suite uses temporary app data, a large synthetic category list, learned-behavior fixtures, and a deterministic LLM stub. It verifies that large whitelists are reduced to relevant candidates, learned categories can outrank generic model output, and Unicode labels such as emoji survive the flow.
- The full Catch2 unit suite extends this coverage with smart branching whitelist persistence, prompt construction, and valid category/subcategory pair enforcement.
- On Windows GUI builds, add `--console-log` if you want to see the self-test output in the launching console.

Windows updater live-test mode:

- `aifilesorter.exe` accepts the following flags directly on Windows:
  `--updater-live-test`
  `--updater-live-test-url=<https://.../AIFileSorterSetup.zip>`
  `--updater-live-test-sha256=<sha256-of-the-downloaded-package>`
  `--updater-live-test-version=<optional-version>`
  `--updater-live-test-min-version=<optional-min-version>`
- Live-test mode is Windows-only and intentionally bypasses the normal update JSON feed.
- If the ZIP contains more than one `.exe` or `.msi`, the updater stops instead of guessing which installer to launch.
- If `--updater-live-test` is present and the URL / SHA flags are omitted, `aifilesorter.exe` also looks for a `live-test.ini` file next to the executable and fills in the missing values from there.
- Command-line flags still win over `live-test.ini`, so you can keep a default file and override just one field when needed.

Example `live-test.ini`:

```ini
[LiveTest]
download_url = https://files.example.com/AIFileSorterSetup-1.7.3.zip
sha256 = 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
current_version = 1.7.3
min_version = 0.0.0
```

Example PowerShell launch:

```powershell
.\aifilesorter.exe `
  --development `
  --updater-live-test
```

---

## Categorization cache and learned behavior

AI File Sorter keeps two separate kinds of local memory under the app config directory (the base directory can be overridden via `AI_FILE_SORTER_CONFIG_DIR`):

- A **categorization cache** for faster reruns and consistency hints.
- A separate **learned-behavior database** for category decisions you explicitly approve in the Review dialog.

### Categorization cache

AI File Sorter stores categorization results in a local SQLite database next to `config.ini`. This cache allows the app to skip already-processed files, preserve rename suggestions between runs, and reuse recent category/subcategory assignments as consistency hints.

What is stored:

- Directory path, file name, and file type (used as a unique key).
- Category/subcategory, taxonomy id, categorization style, and timestamp.
- Suggested filename (for picture and document rename suggestions).
- Rename-only flag (used when picture/document rename-only modes are enabled).
- Rename-applied flag (marks when a rename was executed so it is not offered again).

This cache is used as lightweight memory for consistency, not as model training. In **More consistent** mode, the app can feed recent assignments for similar file types back into the prompt so labels trend toward the same taxonomy over time.

If you rename or move a file from the Review dialog, the cache entry is updated to the new name. Already-renamed picture files are skipped for visual analysis and rename suggestions on later runs. In the Review dialog, those already-renamed rows are hidden when rename-only is enabled, but they stay visible when categorization is enabled so you can still move them into category folders. To reset a folder's cache, accept the recategorization prompt. You can also delete the cache file directly (or point `CATEGORIZATION_CACHE_FILE` to a new filename).

### Local learning from approved reviews

When you approve categories in the Review dialog, the app can remember those local decisions and reuse them as hints for future runs. This helps stabilize similar folders over time, but it does **not** train or modify the underlying AI model.

These learned examples are stored in a separate local database from the normal categorization cache. Clearing the categorization cache does **not** remove learned behavior.

To remove learned review data, use **Settings → Reset learned behavior…**.

### Cache maintenance tooling

Use **Settings → Clear cache…** to inspect and clear the disposable maintenance data the app manages:

- **Categorization cache**: Past file and folder categorization results.
- **Image location cache**: Reverse-geocoded place names for photo GPS lookups.
- **Logs**: Application log files used for troubleshooting and diagnostics.

Downloaded models are managed separately in **Settings → Select LLM…** and are not removed by the cache cleanup dialog.

---

## Uninstallation

- **Debian/Ubuntu package installs**: `sudo apt remove aifilesorter`
- **Linux source installs**: `cd app && sudo make uninstall`
- **macOS source installs**: `cd app && sudo make uninstall`

For source installs, `make uninstall` removes the executable and the staged precompiled libraries. You can also delete downloaded local LLM models in `~/.local/share/aifilesorter/llms` (Linux) or `~/Library/Application Support/aifilesorter/llms` (macOS) if you no longer need them.

---

## Using your OpenAI API key

Want to use ChatGPT instead of the bundled local models? Bring your own OpenAI API key:

1. Open **Settings -> Select LLM** in the app.
2. Choose **ChatGPT (OpenAI API key)**, paste your key, and enter the ChatGPT model you want to use (for example `gpt-4o-mini`, `gpt-4.1`, or `o3-mini`).
3. Click **OK**. The key is stored locally in your AI File Sorter config (`config.ini` in the app data folder) and reused for future runs. Clear the field to remove it.
4. An internet connection is only required while this option is selected.

> The app no longer embeds a bundled key; you always provide your own OpenAI key.

---

## Using your Gemini API key

Prefer Google's models? Use your own Gemini API key:

1. Visit **https://aistudio.google.com** and sign in with your Google account.
2. In the left navigation, open **API keys** (or **Get API key**) and click **Create API key**. Choose *Create API key in new project* (or select an existing project) and copy the generated key.
3. In the app, open **Settings -> Select LLM**, choose **Gemini (Google AI Studio API key)**, paste your key, and enter the Gemini model you want (for example `gemini-2.5-flash-lite`, `gemini-2.5-flash`, or `gemini-2.5-pro`).
4. Click **OK**. The key is stored locally in your AI File Sorter config and reused for future runs. Clear the field to remove it.

> AI Studio keys can be used on the free tier until you hit Google’s limits; higher quotas or enterprise use require billing via Google Cloud.
> The app calls the Gemini `v1` `generateContent` endpoint; use model IDs from `https://generativelanguage.googleapis.com/v1/models?key=YOUR_KEY`. You can enter them with or without the leading `models/` prefix.

---

## Using a custom OpenAI-compatible API

Prefer an OpenAI-compatible endpoint such as **LM Studio**, **Ollama**, or your own hosted gateway? AI File Sorter can use that too:

1. Open **Settings -> Select LLM** in the app.
2. Choose **Custom OpenAI-compatible API (advanced)**.
3. Click **Add…**, then enter a friendly name, the endpoint base URL, the model name to use, and an API key if your endpoint requires one.
4. Save the endpoint, select it from the list, and click **OK**.
5. The endpoint configuration is stored locally in your AI File Sorter config and can be edited or removed later from the same dialog.

Use this option for local servers or remote providers that follow the OpenAI-style API shape. Response time can be tuned with `AI_FILE_SORTER_CUSTOM_LLM_TIMEOUT`; rate-limited providers can be paced with `AI_FILE_SORTER_REMOTE_REQUESTS_PER_MINUTE` or `[Settings] RemoteRequestsPerMinute` (see [Environment variables](#environment-variables)).

---

## Testing

- From the repo root, clean any old cache and run the CTest wrapper:
  
  ```bash
  cd app
  rm -rf ../build-tests      # clear a cache from another checkout
  ./scripts/rebuild_and_test.sh
  ```

- The script configures to `../build-tests`, builds, then runs `ctest`.
- If you have multiple copies of the repo (e.g., `ai-file-sorter` and `ai-file-sorter-mac-dist`), each needs its own `build-tests` folder; reusing one from a different path will make CMake complain about mismatched source/build directories.

---

## Diagnostics

If you need to report a bug or collect troubleshooting data, use the bundled diagnostics scripts:

- **macOS:** `./app/scripts/collect_macos_diagnostics.sh`
- **Linux:** `./app/scripts/collect_linux_diagnostics.sh`
- **Windows (PowerShell):** `.\app\scripts\collect_windows_diagnostics.ps1`

Each script collects relevant logs, redacts common sensitive paths, and packages the result into a zip archive for sharing. See [app/scripts/README.md](app/scripts/README.md) for options such as time filtering and opening the output folder automatically.

For log locations, rotation details, and common troubleshooting notes, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

---

## Help and onboarding

If you want an in-app walkthrough before your first run, open **Help → Quick Start Guide**. The Quick Start guide is localized and covers a safe small-folder trial, the review flow, undo, local learning, and the most common settings you may want to change.

If something looks wrong or you want troubleshooting tips, open **Help → FAQ**.

For log locations, rotation details, and other troubleshooting notes outside the app, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

---

## How to Use

1. Launch the application (see the last step in [Installation](#installation) according your OS).
2. Select a directory to analyze.

If you want a guided walkthrough first, open **Help → Quick Start Guide**. For troubleshooting during setup or after a run, open **Help → FAQ**.

### Using dry run and undo

- In the results dialog, you can enable **"Dry run (preview only, do not move files)"** to preview planned moves. A preview dialog shows From/To without moving any files.
- After a real sort, the app saves a persistent undo plan. You can revert later via **Edit → "Undo last run"** (best-effort; skips conflicts/changes).

3. Tick off the checkboxes on the main window according to your preferences.
4. Click the **"Analyze"** button. The app will scan each file and/or directory based on your selected options.

Recursive scans intentionally skip recognized structured project roots, including Unity, Unreal, Godot, conservative Blender project folders, Git repositories, and common source-code project layouts. This protects folders where moving individual files can break project-relative links, imports, metadata, or build files.

5. A review dialog will appear. Verify the assigned categories (and subcategories, if enabled in step 3).
6. Click **"Confirm & Sort!"** to move the files, or **"Continue Later"** to postpone. You can always resume where you left off since categorization results are saved.

---

## Sorting a Remote Directory (e.g., NAS)

Follow the steps in [How to Use](#how-to-use), but modify **step 2** as follows:  

- **Windows:** Assign a drive letter (e.g., `Z:` or `X:`) to your network share ([instructions here](https://support.microsoft.com/en-us/windows/map-a-network-drive-in-windows-29ce55d1-34e3-a7e2-4801-131475f9557d)).  
- **Linux & macOS:** Mount the network share to a local folder using a command like:  

  ```sh
  sudo mount -t cifs //192.168.1.100/shared_folder /mnt/nas -o username=myuser,password=mypass,uid=$(id -u),gid=$(id -g)
  ```

(Replace 192.168.1.100/shared_folder with your actual network location path and adjust options as needed.)

---

## Contributing

- Fork the repository and submit pull requests.
- Report issues or suggest features on the GitHub issue tracker.
- Follow the existing code style and documentation format.

---

## Credits

- Curl: <https://github.com/curl/curl>
- Dotenv: <https://github.com/motdotla/dotenv>
- git-scm: <https://git-scm.com>
- Hugging Face: <https://huggingface.co>
- JSONCPP: <https://github.com/open-source-parsers/jsoncpp>
- Llama: <https://www.llama.com>
- libzip: <https://libzip.org>
- Local File Organizer <https://github.com/QiuYannnn/Local-File-Organizer>
- llama.cpp <https://github.com/ggml-org/llama.cpp>
- MediaInfoLib: <https://mediaarea.net/en/MediaInfo>
- Mistral AI: <https://mistral.ai>
- OpenAI: <https://platform.openai.com/docs/overview>
- OpenSSL: <https://github.com/openssl/openssl>
- PDFium: <https://pdfium.googlesource.com/pdfium/>
- Poppler (pdftotext): <https://poppler.freedesktop.org/>
- pugixml: <https://pugixml.org>
- Qt: <https://www.qt.io/>
- spdlog: <https://github.com/gabime/spdlog>
- unzip (Info-ZIP): <https://infozip.sourceforge.net/>

## License

This project is licensed under the GNU AFFERO GENERAL PUBLIC LICENSE (GNU AGPL). See the [LICENSE](LICENSE) file for details, or https://www.gnu.org/licenses/agpl-3.0.html.

---

## Donation

Support the development of **AI File Sorter** and its future features. Every contribution counts, and the app remains usable without removing privacy, preview, undo, or local-processing features.

Suggested support levels:

- **$5**: Say thanks and help keep the project going.
- **$15**: Support ongoing fixes, packaging work, and compatibility testing.
- **$30**: Help fund release maintenance, signing/listing costs, and user support.
- **$100+**: Sponsor heavier development work or commercial/organizational use.

You can still choose any amount from **$1 and up**. Donation codes are supporter markers that hide the periodic support reminder; they are not used to take away the free local core.

- **[Donate](https://filesorter.app/donate/)**

---
