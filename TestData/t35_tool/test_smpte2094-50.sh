#!/usr/bin/env bash
#
# T.35 Tool - SMPTE ST 2094-50 Testing Script
# Tests injection and extraction with real SMPTE ST 2094-50 metadata
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/../.."
TEST_DATA_DIR="${SCRIPT_DIR}"

# Find t35_tool executable (priority order: bin/, mybuild/, build/, find)
find_tool() {
    local tool_path=""
    local source=""

    # 1. Check bin/ directory (SET_CUSTOM_OUTPUT_DIRS=ON)
    if [ -f "${PROJECT_ROOT}/bin/t35_tool" ]; then
        tool_path="${PROJECT_ROOT}/bin/t35_tool"
        source="bin/ (custom output dirs)"
    # 2. Check mybuild/ directory
    elif [ -f "${PROJECT_ROOT}/mybuild/IsoLib/t35_tool/t35_tool" ]; then
        tool_path="${PROJECT_ROOT}/mybuild/IsoLib/t35_tool/t35_tool"
        source="mybuild/ (build directory)"
    # 3. Check build/ directory
    elif [ -f "${PROJECT_ROOT}/build/IsoLib/t35_tool/t35_tool" ]; then
        tool_path="${PROJECT_ROOT}/build/IsoLib/t35_tool/t35_tool"
        source="build/ (build directory)"
    # 4. Search for tool
    else
        tool_path=$(find "${PROJECT_ROOT}" -name "t35_tool" -type f -executable 2>/dev/null | grep -v legacy | head -1)
        if [ -n "$tool_path" ]; then
            source="found at $(dirname "$tool_path")"
        fi
    fi

    if [ -z "$tool_path" ] || [ ! -f "$tool_path" ]; then
        printf "${RED}[ERROR]${NC} t35_tool not found. Please build the project first.\n" >&2
        printf "  Searched locations:\n" >&2
        printf "    - ${PROJECT_ROOT}/bin/t35_tool\n" >&2
        printf "    - ${PROJECT_ROOT}/mybuild/IsoLib/t35_tool/t35_tool\n" >&2
        printf "    - ${PROJECT_ROOT}/build/IsoLib/t35_tool/t35_tool\n" >&2
        return 1
    fi

    printf "${GREEN}[TOOL]${NC} Using t35_tool from: ${BLUE}%s${NC}\n" "$source" >&2
    printf "       Path: %s\n\n" "$tool_path" >&2

    echo "$tool_path"
}

TOOL=$(find_tool) || exit 1

# Test configuration
INPUT_VIDEO="${TEST_DATA_DIR}/ST2094-50_LightDetector.mov"
OUTPUT_DIR="${TEST_DATA_DIR}/output_smpte"
T35_PREFIX="B500900001:SMPTE-ST2094-50"

# Injection methods to test
METHODS=(
    "mebx-me4c"
    "dedicated-it35"
)

# SMPTE test folders
SMPTE_FOLDERS=(
    "NoAdaptiveToneMap"
    "DefaultToneMapRWTMO"
    "CustomTMO"
)

# Helper functions
log_info() {
    printf "${BLUE}[INFO]${NC} %s\n" "$1"
}

log_success() {
    printf "${GREEN}[PASS]${NC} %s\n" "$1"
}

log_error() {
    printf "${RED}[FAIL]${NC} %s\n" "$1"
}

log_section() {
    printf "\n"
    printf "${BLUE}========================================${NC}\n"
    printf "${BLUE}%s${NC}\n" "$1"
    printf "${BLUE}========================================${NC}\n"
}

# Check prerequisites
check_prerequisites() {
    log_section "Checking Prerequisites"

    if [ ! -f "${INPUT_VIDEO}" ]; then
        log_error "Input video not found: ${INPUT_VIDEO}"
        exit 1
    fi
    log_success "Input video found: ${INPUT_VIDEO}"

    for folder in "${SMPTE_FOLDERS[@]}"; do
        if [ ! -d "${TEST_DATA_DIR}/${folder}" ]; then
            log_error "SMPTE folder not found: ${TEST_DATA_DIR}/${folder}"
            exit 1
        fi
    done
    log_success "All SMPTE folders present"

    printf "\n"
}

# Test a single SMPTE folder with a specific method
test_smpte_folder() {
    local folder=$1
    local method=$2
    local injected_file="${OUTPUT_DIR}/${folder}_${method}.mov"
    local extract_dir="${OUTPUT_DIR}/${folder}_${method}_extracted"
    local inject_log="${OUTPUT_DIR}/${folder}_${method}_inject.log"
    local extract_log="${OUTPUT_DIR}/${folder}_${method}_extract.log"

    log_section "Testing: ${folder} with ${method}"

    # Inject
    log_info "Injecting SMPTE ST 2094-50 metadata from ${folder} using ${method}..."
    if "${TOOL}" inject "${INPUT_VIDEO}" "${injected_file}" \
        --source "smpte-folder:${TEST_DATA_DIR}/${folder}" \
        --method "${method}" \
        --t35-prefix "${T35_PREFIX}" \
        > "${inject_log}" 2>&1; then
        log_success "Injection successful"
    else
        log_error "Injection failed - check ${inject_log}"
        return 1
    fi

    # Extract
    log_info "Extracting metadata with auto-detection..."
    if "${TOOL}" extract "${injected_file}" "${extract_dir}" \
        --method "auto" \
        --t35-prefix "${T35_PREFIX}" \
        > "${extract_log}" 2>&1; then
        log_success "Extraction successful"

        # Count extracted files
        local count=$(ls -1 "${extract_dir}"/metadata_*.bin 2>/dev/null | wc -l | tr -d ' ')
        log_info "Extracted ${count} metadata files"
    else
        log_error "Extraction failed - check ${extract_log}"
        return 1
    fi

    printf "\n"
}

# Main execution
main() {
    log_section "T.35 Tool - SMPTE ST 2094-50 Tests"

    log_info "Test configuration:"
    log_info "  Input video: ${INPUT_VIDEO}"
    log_info "  Output directory: ${OUTPUT_DIR}"
    log_info "  Injection methods: ${METHODS[*]}"
    log_info "  T.35 prefix: ${T35_PREFIX}"
    printf "\n"

    # Setup
    mkdir -p "${OUTPUT_DIR}"
    check_prerequisites

    # Test all SMPTE folders with all methods
    local failed=0
    local total=0
    for method in "${METHODS[@]}"; do
        log_section "Testing with method: ${method}"
        for folder in "${SMPTE_FOLDERS[@]}"; do
            total=$((total + 1))
            if ! test_smpte_folder "${folder}" "${method}"; then
                failed=$((failed + 1))
            fi
        done
    done

    # Summary
    log_section "Test Summary"
    log_info "Total tests: ${total} (${#METHODS[@]} methods × ${#SMPTE_FOLDERS[@]} folders)"
    log_info "Passed: $((total - failed))"
    log_info "Failed: ${failed}"
    printf "\n"

    if [ ${failed} -eq 0 ]; then
        log_success "All SMPTE tests passed!"
        exit 0
    else
        log_error "${failed} test(s) failed"
        log_info "Check logs in: ${OUTPUT_DIR}"
        exit 1
    fi
}

# Run main
main
