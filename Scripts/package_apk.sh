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
BUILD_TOOLS="${ANDROID_HOME}/build-tools/34.0.0"

if [ -f "${UBT}" ]; then
    echo "==> 1. Compiling ProjectRealWorld Native ARM64 Binaries with UBT (Shipping)..."
    "${UBT}" ProjectRealWorld Android Shipping \
        -Project="${PROJECT_ROOT}/ProjectRealWorld.uproject" \
        -NoMutex
    echo "✓ Native ARM64 compilation complete."
fi

echo "==> 2. Building and Packaging Android Release APK..."
KEYSTORE="/tmp/test_apk/release.keystore"
APK_OUT="${PROJECT_ROOT}/ProjectRealWorld.apk"

if [ -d "/tmp/light_apk/contents" ]; then
    cd /tmp/light_apk/contents
    rm -f /tmp/light_apk/unaligned.apk /tmp/light_apk/aligned.apk
    rm -rf META-INF
    zip -r -0 /tmp/light_apk/unaligned.apk .
    "${BUILD_TOOLS}/zipalign" -f -p -v 4 /tmp/light_apk/unaligned.apk /tmp/light_apk/aligned.apk > /dev/null
    "${BUILD_TOOLS}/apksigner" sign \
        --ks "${KEYSTORE}" \
        --ks-pass pass:android \
        --ks-key-alias realworld \
        --key-pass pass:android \
        --v1-signing-enabled true \
        --v2-signing-enabled true \
        --v3-signing-enabled true \
        /tmp/light_apk/aligned.apk
    "${BUILD_TOOLS}/apksigner" verify -v --min-sdk-version 21 /tmp/light_apk/aligned.apk
    cp /tmp/light_apk/aligned.apk "${APK_OUT}"
    echo "✓ APK packaged and signed successfully: ${APK_OUT}"
    ls -lh "${APK_OUT}"
else
    echo "❌ Source contents directory not found at /tmp/light_apk/contents"
    exit 1
fi

