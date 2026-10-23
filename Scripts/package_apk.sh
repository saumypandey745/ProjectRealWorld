#!/usr/bin/env bash
# ==============================================================================
# Project RealWorld — Android Mobile Packaging Pipeline
# Builds and packages 100% Android-exclusive game APK
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
echo "Project RealWorld — Android Mobile Packaging Pipeline"
echo "============================================================"
echo "Project Root: ${PROJECT_ROOT}"
echo "Android SDK:  ${ANDROID_HOME}"

BUILD_TOOLS="${ANDROID_HOME}/build-tools/34.0.0"
ANDROID_JAR="${ANDROID_HOME}/platforms/android-34/android.jar"
APP_DIR="${PROJECT_ROOT}/Platforms/Android/App"
KEYSTORE="${PROJECT_ROOT}/Platforms/Android/release.keystore"
APK_OUT="${PROJECT_ROOT}/ProjectRealWorld.apk"

# Ensure Keystore exists
if [ ! -f "${KEYSTORE}" ]; then
    echo "Creating new release keystore..."
    keytool -genkey -v -keystore "${KEYSTORE}" \
        -alias realworld -keyalg RSA -keysize 2048 -validity 10000 \
        -storepass android -keypass android \
        -dname "CN=RealWorld, OU=Mobile, O=GameStudio, L=Mumbai, ST=Maharashtra, C=IN"
fi

BUILD_DIR="/tmp/rw_apk_build"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}/compiled_res" "${BUILD_DIR}/gen" "${BUILD_DIR}/bin" "${BUILD_DIR}/apk" "${BUILD_DIR}/dex"

echo "==> 1. Compiling Android Resources with AAPT2..."
"${BUILD_TOOLS}/aapt2" compile --dir "${APP_DIR}/res" -o "${BUILD_DIR}/compiled_res/res.zip"

echo "==> 2. Linking Android Resources & Generating R.java..."
"${BUILD_TOOLS}/aapt2" link \
    -I "${ANDROID_JAR}" \
    "${BUILD_DIR}/compiled_res/res.zip" \
    --manifest "${APP_DIR}/AndroidManifest.xml" \
    --java "${BUILD_DIR}/gen" \
    -o "${BUILD_DIR}/apk/base.apk"

echo "==> 3. Compiling Java Source Files..."
find "${APP_DIR}/src" "${BUILD_DIR}/gen" -name "*.java" > "${BUILD_DIR}/sources.txt"
javac -encoding UTF-8 \
    -cp "${ANDROID_JAR}" \
    -d "${BUILD_DIR}/bin" \
    @"${BUILD_DIR}/sources.txt"

echo "==> 4. Converting Java Bytecode to Dalvik/ART Dex (D8)..."
CLASS_FILES=$(find "${BUILD_DIR}/bin" -name "*.class")
"${BUILD_TOOLS}/d8" \
    --min-api 26 \
    --output "${BUILD_DIR}/dex" \
    --lib "${ANDROID_JAR}" \
    ${CLASS_FILES}

echo "==> 5. Assembling Android APK with Assets..."
cd "${BUILD_DIR}/apk"
unzip -q base.apk -d unzipped/
cp "${BUILD_DIR}/dex/classes.dex" unzipped/
mkdir -p unzipped/assets
cp -r "${APP_DIR}/assets/"* unzipped/assets/

cd unzipped
zip -r -0 "${BUILD_DIR}/unaligned.apk" . > /dev/null

echo "==> 6. Aligning APK with zipalign..."
"${BUILD_TOOLS}/zipalign" -f -p -v 4 "${BUILD_DIR}/unaligned.apk" "${BUILD_DIR}/aligned.apk" > /dev/null

echo "==> 7. Signing APK with apksigner (v1, v2, v3 schemes)..."
"${BUILD_TOOLS}/apksigner" sign \
    --ks "${KEYSTORE}" \
    --ks-pass pass:android \
    --ks-key-alias realworld \
    --key-pass pass:android \
    --v1-signing-enabled true \
    --v2-signing-enabled true \
    --v3-signing-enabled true \
    "${BUILD_DIR}/aligned.apk"

"${BUILD_TOOLS}/apksigner" verify -v --min-sdk-version 21 "${BUILD_DIR}/aligned.apk"

cp "${BUILD_DIR}/aligned.apk" "${APK_OUT}"
echo "============================================================"
echo "✓ Android APK packaged and signed successfully!"
echo "Location: ${APK_OUT}"
ls -lh "${APK_OUT}"
echo "============================================================"
