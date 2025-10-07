#!/bin/bash

# ============================================================================
# SpectraForge Test Runner with Code Coverage
# Цель: >98% покрытия кода
# ============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}SpectraForge Test Suite with Coverage${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Configuration
BUILD_DIR="build"
COVERAGE_TARGET=98.0
COVERAGE_REPORT_DIR="$BUILD_DIR/coverage_report"

# Step 1: Clean previous coverage data
echo -e "${YELLOW}[1/6] Cleaning previous coverage data...${NC}"
if [ -d "$BUILD_DIR" ]; then
    cd "$BUILD_DIR"
    make coverage_clean 2>/dev/null || true
    cd ..
fi

# Step 2: Configure with coverage enabled
echo -e "${YELLOW}[2/6] Configuring CMake with coverage enabled...${NC}"
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TESTS=ON \
    -DENABLE_COVERAGE=ON \
    -DBUILD_EXAMPLES=OFF

# Step 3: Build tests
echo -e "${YELLOW}[3/6] Building tests...${NC}"
cmake --build "$BUILD_DIR" --target run_all_tests -j$(nproc)

# Step 4: Run all tests
echo -e "${YELLOW}[4/6] Running all tests...${NC}"
cd "$BUILD_DIR"
ctest --output-on-failure --verbose

# Step 5: Generate coverage report
echo -e "${YELLOW}[5/6] Generating coverage report...${NC}"
make coverage

# Step 6: Analyze coverage
echo -e "${YELLOW}[6/6] Analyzing coverage results...${NC}"
echo ""

if [ -f "coverage_filtered.info" ]; then
    # Extract coverage percentage
    COVERAGE_PERCENT=$(lcov --summary coverage_filtered.info 2>&1 | grep "lines" | awk '{print $2}' | sed 's/%//')
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}Coverage Report Summary${NC}"
    echo -e "${BLUE}========================================${NC}"
    lcov --summary coverage_filtered.info
    echo ""
    
    # Check if target coverage is met
    if (( $(echo "$COVERAGE_PERCENT >= $COVERAGE_TARGET" | bc -l) )); then
        echo -e "${GREEN}✓ SUCCESS: Coverage target met!${NC}"
        echo -e "${GREEN}  Current: ${COVERAGE_PERCENT}%${NC}"
        echo -e "${GREEN}  Target:  ${COVERAGE_TARGET}%${NC}"
    else
        echo -e "${RED}✗ WARNING: Coverage target not met${NC}"
        echo -e "${RED}  Current: ${COVERAGE_PERCENT}%${NC}"
        echo -e "${RED}  Target:  ${COVERAGE_TARGET}%${NC}"
        echo ""
        echo -e "${YELLOW}Run 'make coverage' to see detailed report${NC}"
    fi
    
    echo ""
    echo -e "${BLUE}HTML Report: ${COVERAGE_REPORT_DIR}/index.html${NC}"
    echo -e "${BLUE}View with: firefox ${COVERAGE_REPORT_DIR}/index.html${NC}"
    
else
    echo -e "${RED}Error: Coverage report not generated${NC}"
    exit 1
fi

cd ..

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Test execution completed!${NC}"
echo -e "${BLUE}========================================${NC}"

# Open coverage report in browser (optional)
if command -v xdg-open &> /dev/null; then
    read -p "Open coverage report in browser? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        xdg-open "$COVERAGE_REPORT_DIR/index.html"
    fi
fi

exit 0
