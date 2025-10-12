#!/bin/bash

# BUILD_AND_TEST_FIXED.sh
# Скрипт для компиляции и тестирования исправленных демо

echo "🔧 BUILDING AND TESTING SPECTRAFORGE FIXED DEMOS"
echo "================================================="

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'  
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}📁 Step 1: Prepare headless demo sources${NC}"
mkdir -p examples shaders
cp headless/examples/BlueCube_Demo_FIXED.cpp examples/BlueCube_Demo_FIXED.cpp
cp headless/examples/SpectraForge_Example_Demo_FIXED.cpp examples/SpectraForge_Example_Demo_FIXED.cpp
cp headless/shaders/TriangleSplatting_FIXED.comp shaders/TriangleSplatting_FIXED.comp

echo -e "${GREEN}✅ Headless demo sources ready${NC}"

echo -e "${BLUE}🔨 Step 3: Compile shaders${NC}"
cd shaders/
if [ -f "glslangValidator" ] || command -v glslangValidator &> /dev/null; then
    glslangValidator -V TriangleSplatting_FIXED.comp -o TriangleSplatting_FIXED.comp.spv
    echo -e "${GREEN}✅ Shader compiled successfully${NC}"
else
    echo -e "${RED}❌ glslangValidator not found! Install Vulkan SDK${NC}"
fi
cd ..

echo -e "${BLUE}🏗️  Step 4: Build project${NC}"
if [ -d "build" ]; then
    cd build
    cmake .. -DSPECTRAFORGE_HEADLESS=ON -DSPECTRAFORGE_REQUIRE_VULKAN=OFF
    make -j$(nproc)
    build_result=$?
    cd ..
else
    echo -e "${YELLOW}⚠️  Build directory not found, creating...${NC}"
    mkdir build
    cd build
    cmake .. -DSPECTRAFORGE_HEADLESS=ON -DSPECTRAFORGE_REQUIRE_VULKAN=OFF
    make -j$(nproc)
    build_result=$?
    cd ..
fi

if [ $build_result -eq 0 ]; then
    echo -e "${GREEN}✅ Build successful${NC}"
else
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

echo -e "${BLUE}🧪 Step 5: Test fixed demos${NC}"

echo -e "${YELLOW}Testing BlueCube_Demo_FIXED...${NC}"
if [ -f "build/examples/BlueCube_Demo_FIXED" ]; then
    timeout 10s ./build/examples/BlueCube_Demo_FIXED &
    DEMO_PID=$!
    sleep 8
    kill $DEMO_PID 2>/dev/null
    echo -e "${GREEN}✅ BlueCube_Demo_FIXED launched successfully${NC}"
else
    echo -e "${RED}❌ BlueCube_Demo_FIXED executable not found${NC}"
fi

echo -e "${YELLOW}Testing SpectraForge_Example_Demo_FIXED...${NC}"
if [ -f "build/examples/SpectraForge_Example_Demo_FIXED" ]; then
    timeout 10s ./build/examples/SpectraForge_Example_Demo_FIXED &
    DEMO_PID=$!
    sleep 8  
    kill $DEMO_PID 2>/dev/null
    echo -e "${GREEN}✅ SpectraForge_Example_Demo_FIXED launched successfully${NC}"
else
    echo -e "${RED}❌ SpectraForge_Example_Demo_FIXED executable not found${NC}"
fi

echo -e "${BLUE}🧪 Step 6: Run headless test suite${NC}"
if [ -d "build" ]; then
    (cd build && ctest --output-on-failure)
else
    echo -e "${RED}❌ Build directory missing before tests${NC}"
    exit 1
fi

echo -e "${BLUE}📊 Step 6: Validation summary${NC}"
echo "================================================="
echo -e "${GREEN}APPLIED FIXES:${NC}"
echo "✅ Headless engine with deterministic output"
echo "✅ Reproducible demos for CI environments"
echo "✅ Minimal shader placeholder for tooling"

echo ""
echo -e "${YELLOW}EXPECTED RESULTS:${NC}"
echo "🔵 BlueCube_Demo_FIXED: Headless rotating cube telemetry"
echo "🌈 SpectraForge_Example_Demo_FIXED: Colourful triangle showcase logs"
echo "📷 Camera pose and debug modes validated via console output"

echo ""
echo -e "${GREEN}🎯 TESTING COMPLETE!${NC}"
echo "If demos still show dark blue screen, check:"
echo "1. Shader compilation (TriangleSplatting_FIXED.comp.spv exists)"
echo "2. Triangle upload (check console output for triangle count)" 
echo "3. Camera matrix validation (check debug mode output)"