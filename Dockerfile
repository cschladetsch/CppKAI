# syntax=docker/dockerfile:1
# ---------------------------------------------------------------------------
# CppKAI build container
#
# This targets a WINDOWS container, not Linux. Everything observed in this
# session's build logs points at a clang++ front-end deliberately targeting
# the MSVC ABI (-D_DLL -D_MT -Xclang --dependent-lib=msvcrt), linking against
# msvcrt/UCRT, and depending on a Windows Boost install path
# (C:/local/boost_1_91_0). None of that exists on Linux, so a Linux
# container would need a from-scratch MinGW/libstdc++ port -- a much bigger
# job than containerizing the build as-is. This Dockerfile reproduces the
# toolchain actually used tonight.
#
# IMPORTANT CAVEAT: building/running this image requires Docker in
# "Windows containers" mode (Docker Desktop: right-click tray icon ->
# "Switch to Windows containers..."), on a Windows host. It will NOT build
# under Linux containers / Docker Desktop's default Linux mode.
#
# Build:
#   docker build -t cppkai-build .
#
# Run (build the repo mounted from the host):
#   docker run --rm -v C:\Users\chris\local\repos\CppKAI:C:\src cppkai-build
#
# The base image below is large (Visual Studio Build Tools + Windows SDK),
# so the first build will take a while and produce a multi-GB image. That's
# expected for an MSVC-ABI C++ toolchain image.
# ---------------------------------------------------------------------------

FROM mcr.microsoft.com/windows/servercore:ltsc2022 AS base
SHELL ["powershell", "-NoLogo", "-NoProfile", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'SilentlyContinue';"]

# --- Chocolatey, for CMake/Ninja/LLVM ---------------------------------------
RUN Set-ExecutionPolicy Bypass -Scope Process -Force; \
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# --- Visual Studio 2022 Build Tools: MSVC ABI headers/libs (UCRT, Windows SDK) ---
# clang++ is used as the *compiler*, but it needs the MSVC-flavoured CRT
# headers/import libs (--dependent-lib=msvcrt) which only ship with the
# VC++ workload / Windows SDK, not with LLVM itself.
RUN choco install -y visualstudio2022buildtools `
      --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --includeRecommended --passive --wait"

# --- LLVM/clang, CMake, Ninja -------------------------------------------------
# Pin versions here once you know the exact ones on your dev machine
# (this session observed clang++ at "C:\Program Files\LLVM\bin\clang++.EXE"
# but never captured `clang++ --version`; adjust the pins below to match).
RUN choco install -y llvm --version=18.1.8 ; \
    choco install -y cmake --installargs 'ADD_CMAKE_TO_PATH=System' ; \
    choco install -y ninja

# --- Boost 1.91.0, installed at the exact path baked into the build (C:/local/boost_1_91_0) ---
# NOTE: 1.91.0 has no prebuilt Windows/clang binary package at time of
# writing, so this builds it from source with the same clang++ that will
# build CppKAI. This step alone can take 30-60+ minutes; swap in a prebuilt
# archive here if you have one already staged internally.
RUN New-Item -ItemType Directory -Force -Path C:\local | Out-Null; \
    Invoke-WebRequest -Uri "https://archives.boost.io/release/1.91.0/source/boost_1_91_0.zip" -OutFile C:\local\boost_1_91_0.zip; \
    Expand-Archive -Path C:\local\boost_1_91_0.zip -DestinationPath C:\local; \
    Remove-Item C:\local\boost_1_91_0.zip

WORKDIR C:\local\boost_1_91_0
RUN $env:PATH = "C:\Program Files\LLVM\bin;" + $env:PATH; \
    .\bootstrap.bat clang-win; \
    .\b2.exe toolset=clang-win address-model=64 --with-system --with-filesystem --with-thread --with-date_time --with-regex -j4 stage

# --- ENet / rang are consumed as submodules under Ext/, so no separate ---
# --- install step is needed for them -- they build from source with the ---
# --- rest of the project (see Ext/ENet, Ext/rang in the repo tree).      ---

# Make the toolchain discoverable for the build step below.
ENV PATH="C:\\Program Files\\LLVM\\bin;C:\\ProgramData\\chocolatey\\bin;${PATH}"
ENV BOOST_ROOT="C:\\local\\boost_1_91_0"

WORKDIR C:\src

# Source is expected to be bind-mounted at build/run time (see the `docker
# run -v ...:C:\src` example above) so the container always builds whatever
# is currently on disk, submodules included, without needing a COPY + git
# checkout step baked into the image. If you'd rather bake the source in,
# replace the two lines below with:
#   COPY . C:\src
# (COPY does not currently pull submodules automatically, so if you go
# this route make sure the checkout on the host already has
# Ext\CppKaiCore and Ext\CppKaiLanguage populated before `docker build`.)

# --- Build entrypoint --------------------------------------------------------
# Mirrors the exact command line used throughout this session:
#   cmake --build "<repo>\build" --config Debug
# The CMake *configure* step is assumed to already produce a Ninja project
# using clang++ with the MSVC-ABI flags (-D_DLL -D_MT
# -Xclang --dependent-lib=msvcrt, -std=c++23) via the repo's own
# CMakeLists.txt / toolchain file -- this Dockerfile does not re-derive
# those flags, it just makes sure clang++, cmake, ninja and Boost are present
# at the paths the existing build already expects.
COPY docker-entrypoint.ps1 C:\docker-entrypoint.ps1
ENTRYPOINT ["powershell", "-NoLogo", "-NoProfile", "-File", "C:\\docker-entrypoint.ps1"]