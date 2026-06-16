#\!/usr/bin/env bash
#
# T.35 Tool - Comprehensive Testing Script
# Tests injection modes with matching extraction + auto extraction
# Performs round-trip verification for valid combinations only
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
TEST_DATA_DIR="${PROJECT_ROOT}/TestData/t35_tool"

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
INPUT_VIDEO="${PROJECT_ROOT}/TestData/isobmff/01_simple.mp4"
OUTPUT_DIR="${TEST_DATA_DIR}/output_all_modes"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULTS_DIR="${OUTPUT_DIR}/${TIMESTAMP}"

# Test configuration
T35_PREFIX="B500900001:SMPTE-ST2094-50"
SOURCE_MANIFEST="${TEST_DATA_DIR}/test_manifest.json"

# Injection modes to test
INJECTION_MODES=(
    "mebx-me4c"
    "dedicated-it35"
    "sample-group"
    # Won't work with the test video 01_simple.mp4 because it's neither HEVC nor AV1
    # "bitstream"
)

# Results tracking (simple arrays instead of associative array)
RESULTS_KEYS=()
RESULTS_VALUES=()
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Helper function to store result
store_result() {
    local key="$1"
    local value="$2"
    RESULTS_KEYS+=("$key")
    RESULTS_VALUES+=("$value")
}

# Helper function to get result
get_result() {
    local key="$1"
    local i
    local len=${#RESULTS_KEYS[@]}
    for ((i=0; i<len; i++)); do
        if [ "${RESULTS_KEYS[$i]}" = "$key" ]; then
            echo "${RESULTS_VALUES[$i]}"
            return
        fi
    done
    echo "UNKNOWN"
}

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

log_warn() {
    printf "${YELLOW}[WARN]${NC} %s\n" "$1"
}

log_section() {
    printf "\n"
    printf "${BLUE}========================================${NC}\n"
    printf "${BLUE}%s${NC}\n" "$1"
    printf "${BLUE}========================================${NC}\n"
}

# Create directory structure
setup_directories() {
    log_section "Setup Test Environment"
    
    log_info "Creating output directories..."
    mkdir -p "${RESULTS_DIR}"/{injected,extracted,diffs,logs}
    
    log_info "Results directory: ${RESULTS_DIR}"
    log_info "Tool: ${TOOL}"
    log_info "Input video: ${INPUT_VIDEO}"
    log_info "Source manifest: ${SOURCE_MANIFEST}"
    printf "\n"
}

# Verify prerequisites
check_prerequisites() {
    log_section "Checking Prerequisites"
    
    if [ \! -f "${TOOL}" ]; then
        log_error "Tool not found: ${TOOL}"
        exit 1
    fi
    log_success "Tool found: ${TOOL}"
    
    if [ \! -f "${INPUT_VIDEO}" ]; then
        log_error "Input video not found: ${INPUT_VIDEO}"
        exit 1
    fi
    log_success "Input video found: ${INPUT_VIDEO}"
    
    if [ \! -f "${SOURCE_MANIFEST}" ]; then
        log_error "Source manifest not found: ${SOURCE_MANIFEST}"
        exit 1
    fi
    log_success "Source manifest found: ${SOURCE_MANIFEST}"
    
    # Check binary files referenced in manifest
    log_info "Checking test data files..."
    for bin_file in "${TEST_DATA_DIR}"/meta_*.bin; do
        if [ \! -f "${bin_file}" ]; then
            log_error "Binary file not found: ${bin_file}"
            exit 1
        fi
    done
    log_success "All test data files present"
    printf "\n"
}

# Test injection
test_injection() {
    local injection_mode=$1
    local output_file="${RESULTS_DIR}/injected/${injection_mode}.mp4"
    local log_file="${RESULTS_DIR}/logs/inject_${injection_mode}.log"
    
    log_info "Injecting with mode: ${injection_mode}"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if "${TOOL}" inject "${INPUT_VIDEO}" "${output_file}" \
        --source "generic-json:${SOURCE_MANIFEST}" \
        --method "${injection_mode}" \
        --t35-prefix "${T35_PREFIX}" \
        > "${log_file}" 2>&1; then
        
        PASSED_TESTS=$((PASSED_TESTS + 1))
        store_result "inject_${injection_mode}" "PASS"
        log_success "Injection successful: ${injection_mode}"
        
        # Get file size
        local size=$(stat -f%z "${output_file}" 2>/dev/null || stat -c%s "${output_file}" 2>/dev/null)
        log_info "Output file size: ${size} bytes"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        store_result "inject_${injection_mode}" "FAIL"
        log_error "Injection failed: ${injection_mode}"
        log_info "Check log: ${log_file}"
    fi
}

# Test extraction
test_extraction() {
    local injection_mode=$1
    local extraction_mode=$2
    local injected_file="${RESULTS_DIR}/injected/${injection_mode}.mp4"
    local extract_dir="${RESULTS_DIR}/extracted/${injection_mode}_${extraction_mode}"
    local log_file="${RESULTS_DIR}/logs/extract_${injection_mode}_${extraction_mode}.log"
    
    log_info "Extracting ${injection_mode} with ${extraction_mode}..."
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ \! -f "${injected_file}" ]; then
        log_warn "Skipping (injected file missing): ${injection_mode} -> ${extraction_mode}"
        store_result "extract_${injection_mode}_${extraction_mode}" "SKIP"
        return
    fi
    
    if "${TOOL}" extract "${injected_file}" "${extract_dir}" \
        --method "${extraction_mode}" \
        --t35-prefix "${T35_PREFIX}" \
        > "${log_file}" 2>&1; then
        
        PASSED_TESTS=$((PASSED_TESTS + 1))
        store_result "extract_${injection_mode}_${extraction_mode}" "PASS"
        log_success "Extraction successful: ${injection_mode} -> ${extraction_mode}"
        
        # Count extracted files
        local count=$(ls -1 "${extract_dir}"/metadata_*.bin 2>/dev/null | wc -l | tr -d ' ')
        log_info "Extracted ${count} metadata files"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        store_result "extract_${injection_mode}_${extraction_mode}" "FAIL"
        log_error "Extraction failed: ${injection_mode} -> ${extraction_mode}"
        log_info "Check log: ${log_file}"
    fi
}

# Verify round-trip
verify_roundtrip() {
    local injection_mode=$1
    local extraction_mode=$2
    local extract_dir="${RESULTS_DIR}/extracted/${injection_mode}_${extraction_mode}"
    local diff_file="${RESULTS_DIR}/diffs/${injection_mode}_${extraction_mode}.diff"
    
    log_info "Verifying round-trip: ${injection_mode} -> ${extraction_mode}"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ \! -d "${extract_dir}" ]; then
        log_warn "Skipping verification (extraction failed)"
        store_result "verify_${injection_mode}_${extraction_mode}" "SKIP"
        return
    fi
    
    # Compare each extracted file with original
    local all_match=true
    for i in 1 2 3; do
        local original="${TEST_DATA_DIR}/meta_00${i}.bin"
        local extracted="${extract_dir}/metadata_${i}.bin"
        
        if [ \! -f "${extracted}" ]; then
            log_error "Missing extracted file: metadata_${i}.bin"
            all_match=false
            echo "Missing: metadata_${i}.bin" >> "${diff_file}"
            continue
        fi
        
        if \! diff -q "${original}" "${extracted}" > /dev/null 2>&1; then
            log_error "Mismatch: metadata_${i}.bin"
            all_match=false
            diff "${original}" "${extracted}" >> "${diff_file}" 2>&1 || true
        fi
    done
    
    if [ "$all_match" = true ]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        store_result "verify_${injection_mode}_${extraction_mode}" "PASS"
        log_success "Round-trip verified: ${injection_mode} -> ${extraction_mode}"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        store_result "verify_${injection_mode}_${extraction_mode}" "FAIL"
        log_error "Round-trip verification failed: ${injection_mode} -> ${extraction_mode}"
        log_info "Check diff: ${diff_file}"
    fi
}

# Generate summary report
generate_report() {
    local report_file="${RESULTS_DIR}/TEST_REPORT.txt"
    
    log_section "Generating Test Report"
    
    {
        echo "========================================="
        echo "T.35 Tool - Test Report"
        echo "========================================="
        echo "Date: $(date)"
        echo "Results Directory: ${RESULTS_DIR}"
        echo ""
        echo "Configuration:"
        echo "  Tool: ${TOOL}"
        echo "  Input Video: ${INPUT_VIDEO}"
        echo "  T.35 Prefix: ${T35_PREFIX}"
        echo ""
        echo "Test Summary:"
        echo "  Total Tests: ${TOTAL_TESTS}"
        echo "  Passed: ${PASSED_TESTS}"
        echo "  Failed: ${FAILED_TESTS}"
        echo ""
        echo "========================================="
        echo "Injection Tests"
        echo "========================================="
        for mode in "${INJECTION_MODES[@]}"; do
            local result=$(get_result "inject_${mode}")
            printf "  %-20s %s\n" "${mode}:" "${result}"
        done
        echo ""
        echo "========================================="
        echo "Extraction Tests"
        echo "========================================="
        for inj_mode in "${INJECTION_MODES[@]}"; do
            echo ""
            echo "  ${inj_mode}:"
            # Show matching extraction mode
            local result=$(get_result "extract_${inj_mode}_${inj_mode}")
            printf "    %-20s %s\n" "${inj_mode}:" "${result}"
            # Show auto extraction
            local result_auto=$(get_result "extract_${inj_mode}_auto")
            printf "    %-20s %s\n" "auto:" "${result_auto}"
        done
        echo ""
        echo "========================================="
        echo "Round-Trip Verification"
        echo "========================================="
        for inj_mode in "${INJECTION_MODES[@]}"; do
            echo ""
            echo "  ${inj_mode}:"
            # Show matching extraction mode
            local result=$(get_result "verify_${inj_mode}_${inj_mode}")
            printf "    %-20s %s\n" "${inj_mode}:" "${result}"
            # Show auto extraction
            local result_auto=$(get_result "verify_${inj_mode}_auto")
            printf "    %-20s %s\n" "auto:" "${result_auto}"
        done
        echo ""
        echo "========================================="
        echo "Files Generated"
        echo "========================================="
        echo "  Injected MP4 files: ${RESULTS_DIR}/injected/"
        echo "  Extracted metadata: ${RESULTS_DIR}/extracted/"
        echo "  Diff files: ${RESULTS_DIR}/diffs/"
        echo "  Log files: ${RESULTS_DIR}/logs/"
        echo ""
    } | tee "${report_file}"
    
    log_success "Report saved to: ${report_file}"
}

# Main test execution
main() {
    log_section "T.35 Tool - Comprehensive Test Suite"
    
    setup_directories
    check_prerequisites
    
    # Test all injection modes
    log_section "Testing Injection Modes"
    for mode in "${INJECTION_MODES[@]}"; do
        test_injection "${mode}"
    done
    printf "\n"
    
    # Test extraction: each injection mode with matching extractor + auto
    log_section "Testing Extraction Modes"
    for inj_mode in "${INJECTION_MODES[@]}"; do
        log_info "Testing extractions from: ${inj_mode}"
        # Test with matching extraction mode
        test_extraction "${inj_mode}" "${inj_mode}"
        # Test with auto extraction
        test_extraction "${inj_mode}" "auto"
        printf "\n"
    done
    
    # Verify round-trips: each injection mode with matching extractor + auto
    log_section "Verifying Round-Trip Integrity"
    for inj_mode in "${INJECTION_MODES[@]}"; do
        log_info "Verifying round-trips for: ${inj_mode}"
        # Verify with matching extraction mode
        verify_roundtrip "${inj_mode}" "${inj_mode}"
        # Verify with auto extraction
        verify_roundtrip "${inj_mode}" "auto"
        printf "\n"
    done
    
    # Generate report
    generate_report
    
    # Final summary
    log_section "Test Execution Complete"
    if [ ${FAILED_TESTS} -eq 0 ]; then
        log_success "All tests passed\! (${PASSED_TESTS}/${TOTAL_TESTS})"
        exit 0
    else
        log_error "Some tests failed: ${FAILED_TESTS}/${TOTAL_TESTS}"
        log_info "Check results in: ${RESULTS_DIR}"
        exit 1
    fi
}

# Run main
main
