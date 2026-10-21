#!/usr/bin/env bash
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_SRC="${PROJECT_ROOT}/Platforms/Android/App"
BUILD_DIR="/tmp/apk_build_workspace"
OUTPUT_APK="${PROJECT_ROOT}/PackagedBuilds/Android/ProjectRealWorld.apk"

# Resolve Android SDK paths
ANDROID_HOME="${ANDROID_HOME:-/home/saumy/Android/sdk}"
if [ ! -d "${ANDROID_HOME}" ]; then
    echo "❌ ANDROID_HOME not found: ${ANDROID_HOME}"
    exit 1
fi

BUILD_TOOLS_VERSION=$(ls "${ANDROID_HOME}/build-tools" 2>/dev/null | sort -V | tail -n 1)
PLATFORM_VERSION=$(ls "${ANDROID_HOME}/platforms" 2>/dev/null | sort -V | tail -n 1)
SDK_TOOLS="${ANDROID_HOME}/build-tools/${BUILD_TOOLS_VERSION}"
ANDROID_JAR="${ANDROID_HOME}/platforms/${PLATFORM_VERSION}/android.jar"

echo "Using Android Build Tools: ${SDK_TOOLS}"
echo "Using Android Platform: ${ANDROID_JAR}"

# Prepare workspace
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cp -r "${APP_SRC}/"* "${BUILD_DIR}/"
mkdir -p "$(dirname "${OUTPUT_APK}")"

echo "==> 1. Compiling Android Resources with AAPT2..."
"${SDK_TOOLS}/aapt2" compile --dir "${BUILD_DIR}/res" -o "${BUILD_DIR}/resources.zip"

echo "==> 2. Linking APK with AAPT2..."
"${SDK_TOOLS}/aapt2" link \
    -I "${ANDROID_JAR}" \
    "${BUILD_DIR}/resources.zip" \
    --manifest "${BUILD_DIR}/AndroidManifest.xml" \
    --java "${BUILD_DIR}/src" \
    -o "${BUILD_DIR}/unaligned.apk"

echo "==> 3. Compiling Java Source Files..."
mkdir -p "${BUILD_DIR}/bin"
javac -cp "${ANDROID_JAR}:${BUILD_DIR}/src" \
    -source 1.8 -target 1.8 \
    $(find "${BUILD_DIR}/src" -name "*.java") \
    -d "${BUILD_DIR}/bin"

echo "==> 4. Converting Java Bytecode to DEX with D8..."
"${SDK_TOOLS}/d8" $(find "${BUILD_DIR}/bin" -name "*.class") \
    --lib "${ANDROID_JAR}" \
    --output "${BUILD_DIR}"

echo "==> 5. Adding DEX and Assets to APK..."
cd "${BUILD_DIR}"
zip -u unaligned.apk classes.dex
cd "${BUILD_DIR}"
zip -r -u unaligned.apk assets/

echo "==> 6. Aligning APK with ZipAlign..."
"${SDK_TOOLS}/zipalign" -f -v -p 4 "${BUILD_DIR}/unaligned.apk" "${BUILD_DIR}/aligned.apk"

echo "==> 7. Generating Keystore & Signing APK..."
KEYSTORE="${BUILD_DIR}/debug.keystore"
keytool -genkey -v -keystore "${KEYSTORE}" -alias androiddebugkey \
    -keyalg RSA -keysize 2048 -validity 10000 \
    -storepass android -keypass android \
    -dname "CN=Android Debug,O=Android,C=US"

"${SDK_TOOLS}/apksigner" sign \
    --ks "${KEYSTORE}" \
    --ks-key-alias androiddebugkey \
    --ks-pass pass:android \
    --key-pass pass:android \
    --out "${OUTPUT_APK}" \
    "${BUILD_DIR}/aligned.apk"

echo "==> 8. Verifying Signature..."
"${SDK_TOOLS}/apksigner" verify "${OUTPUT_APK}"

echo "✓ Successfully generated signed APK: ${OUTPUT_APK}"
ls -lh "${OUTPUT_APK}"
