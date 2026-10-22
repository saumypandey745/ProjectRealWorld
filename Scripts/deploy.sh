#!/usr/bin/env bash
# ==============================================================================
# Project RealWorld - Local Deployment & CI/CD Automation Tool
# ==============================================================================
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UPROJECT_FILE="${PROJECT_ROOT}/ProjectRealWorld.uproject"
UE_ROOT="${UE_ROOT:-/home/saumy/UnrealEngine}"

# ANSI Colors
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_header() {
    echo -e "${CYAN}======================================================${NC}"
    echo -e "${CYAN}  🌆 Project RealWorld - Build & Deploy Pipeline     ${NC}"
    echo -e "${CYAN}======================================================${NC}"
}

usage() {
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  check                Validate Android project files, C++ contracts, and configurations"
    echo "  package [Config]     Package Android ARM64 APK via RunUAT (Development/Shipping)"
    echo "  release <version>    Create a git tag and push to trigger automated GitHub Release"
    echo "  ci                   Run full local CI pipeline checks before pushing"
    echo "  help                 Display this help message"
    echo ""
    echo "Examples:"
    echo "  $0 check"
    echo "  $0 package Shipping"
    echo "  $0 release v1.0.0"
    exit 1
}

cmd_check() {
    echo -e "${YELLOW}==> Running CI validation checks...${NC}"
    cd "${PROJECT_ROOT}"

    # 1. Validate .uproject JSON
    echo "1. Checking ProjectRealWorld.uproject..."
    python3 -c "
import json
with open('ProjectRealWorld.uproject') as f:
    data = json.load(f)
assert 'FileVersion' in data
assert 'Modules' in data and len(data['Modules']) > 0
print(f'   ✓ UProject valid (Engine: {data[\"EngineAssociation\"]}, Plugins: {len(data[\"Plugins\"])})')
"

    # 2. Check configs
    echo "2. Checking configuration files..."
    CONFIGS=(
        "Config/DefaultEngine.ini"
        "Config/DefaultGame.ini"
        "Config/DefaultInput.ini"
        "Config/DefaultScalability.ini"
        "Config/Android/AndroidEngine.ini"
        "Config/Android/AndroidScalability.ini"
    )
    for cfg in "${CONFIGS[@]}"; do
        if [ ! -f "$cfg" ]; then
            echo -e "${RED}   ❌ Missing: $cfg${NC}"
            exit 1
        fi
    done
    echo "   ✓ All essential Android platform configs present."

    # 3. Check C++ headers & GENERATED_BODY()
    echo "3. Checking C++ source contracts..."
    python3 -c "
import os, glob
headers = glob.glob('Source/ProjectRealWorld/**/*.h', recursive=True)
err = 0
for h in headers:
    with open(h, 'r', encoding='utf-8', errors='ignore') as f:
        c = f.read()
        if 'UCLASS(' in c or 'USTRUCT(' in c:
            bn = os.path.basename(h).replace('.h', '')
            if f'{bn}.generated.h' not in c:
                print(f'   ❌ Missing {bn}.generated.h in {h}')
                err += 1
if err == 0:
    print(f'   ✓ All {len(headers)} C++ headers adhere to UE conventions.')
else:
    exit(1)
"
    echo -e "${GREEN}✓ All integrity checks passed successfully!${NC}"
}

cmd_package() {
    PLATFORM="Android"
    CONFIG="${1:-Shipping}"
    OUTPUT_DIR="${PROJECT_ROOT}/PackagedBuilds/Android"

    echo -e "${YELLOW}==> Packaging Android Mobile Game (${CONFIG} - ARM64)...${NC}"

    if [ ! -f "${UE_ROOT}/Engine/Build/BatchFiles/RunUAT.sh" ]; then
        echo -e "${RED}❌ RunUAT.sh not found at ${UE_ROOT}/Engine/Build/BatchFiles/RunUAT.sh${NC}"
        echo "Please set UE_ROOT environment variable to your Unreal Engine directory."
        exit 1
    fi

    mkdir -p "${OUTPUT_DIR}"

    EXTRA_FLAGS="-cookflavor=ETC2 -targetplatform=Android_ARM64"

    echo "Executing RunUAT for Android..."
    "${UE_ROOT}/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
        -project="${UPROJECT_FILE}" \
        -platform="Android" \
        -configuration="${CONFIG}" \
        -archivedirectory="${OUTPUT_DIR}" \
        -build -cook -package -stage -archive \
        ${EXTRA_FLAGS}

    echo -e "${GREEN}✓ Android Packaging complete! Output stored in: ${OUTPUT_DIR}${NC}"
}

cmd_release() {
    VERSION="$1"
    if [ -z "${VERSION}" ]; then
        echo -e "${RED}❌ Version required. E.g.: $0 release v1.0.0${NC}"
        exit 1
    fi

    # Ensure format begins with 'v'
    if [[ ! "${VERSION}" =~ ^v ]]; then
        VERSION="v${VERSION}"
    fi

    echo -e "${YELLOW}==> Initiating release for version: ${VERSION}${NC}"
    cmd_check

    # Commit any uncommitted changes if desired, or verify working tree is clean
    if [ -n "$(git status --porcelain)" ]; then
        echo -e "${YELLOW}==> Staging and committing pending files...${NC}"
        git add -A
        git commit -m "chore(release): prepare for release ${VERSION}"
    fi

    # Create annotated tag
    echo "==> Creating git tag ${VERSION}..."
    git tag -a "${VERSION}" -m "Release ${VERSION} - Project RealWorld"

    echo "==> Pushing commits and tag to origin/main..."
    git push origin main
    git push origin "${VERSION}"

    echo -e "${GREEN}✓ Tag ${VERSION} pushed! GitHub Actions will now automatically build and publish the release.${NC}"
    echo "Check workflow status at: https://github.com/saumypandey745/ProjectRealWorld/actions"
}

# Main routing
print_header
case "$1" in
    check|ci)
        cmd_check
        ;;
    package)
        shift
        cmd_package "$@"
        ;;
    release)
        shift
        cmd_release "$@"
        ;;
    help|--help|-h|"")
        usage
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        usage
        ;;
esac
