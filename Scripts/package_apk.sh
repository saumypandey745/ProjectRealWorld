#!/usr/bin/env bash
# ==============================================================================
# Project RealWorld — Native Unreal Engine Android Packaging Pipeline
# ==============================================================================
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE_ROOT="${ENGINE_ROOT:-/home/saumy/UnrealEngine}"
ANDROID_HOME="${ANDROID_HOME:-/home/saumy/Android/sdk}"
NDKROOT="${NDKROOT:-${ANDROID_HOME}/ndk/25.1.8937393}"

export DOTNET_ROLL_FORWARD=Major
export ANDROID_HOME
export NDKROOT
export NDK_ROOT="${NDKROOT}"

echo "============================================================"
echo "Project RealWorld — Native Unreal Engine Android Build Pipeline"
echo "============================================================"
echo "Project Root: ${PROJECT_ROOT}"
echo "Engine Root:  ${ENGINE_ROOT}"
echo "Android SDK:  ${ANDROID_HOME}"
echo "Android NDK:  ${NDKROOT}"

UBT="${ENGINE_ROOT}/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool"

if [ ! -f "${UBT}" ]; then
    echo "❌ UnrealBuildTool not found at: ${UBT}"
    exit 1
fi

echo "==> 1. Compiling ProjectRealWorld Native ARM64 Binaries with UBT..."
"${UBT}" ProjectRealWorld Android Development \
    -Project="${PROJECT_ROOT}/ProjectRealWorld.uproject" \
    -NoMutex

echo "✓ Native ARM64 compilation complete."
