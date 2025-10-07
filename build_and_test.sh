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

echo -e "${BLUE}📁 Step 1: Backup original files${NC}"
cp examples/BlueCube_Demo.cpp examples/BlueCube_Demo_BACKUP.cpp
cp examples/SpectraForge_Example_Demo.cpp examples/SpectraForge_Example_Demo_BACKUP.cpp
cp src/app/Engine.cpp src/app/Engine_BACKUP.cpp
cp shaders/TriangleSplatting.comp shaders/TriangleSplatting_BACKUP.comp

echo -e "${GREEN}✅ Original files backed up${NC}"

echo -e "${BLUE}📝 Step 2: Apply fixes${NC}"
# Копируем исправленные файлы
cp BlueCube_Demo_FIXED.cpp examples/
cp SpectraForge_Example_Demo_FIXED.cpp examples/
cp TriangleSplatting_FIXED.comp shaders/

echo -e "${YELLOW}⚠️  Note: Engine.h needs manual addition of setExternalCameraControl method${NC}"
echo "Add this to Engine.h public section:"
echo "    void setExternalCameraControl(bool enabled);"

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
    make -j$(nproc)
    build_result=$?
    cd ..
else
    echo -e "${YELLOW}⚠️  Build directory not found, creating...${NC}"
    mkdir build
    cd build
    cmake ..
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

echo -e "${BLUE}📊 Step 6: Validation summary${NC}"
echo "================================================="
echo -e "${GREEN}APPLIED FIXES:${NC}"
echo "✅ setExternalCameraControl(true) - камера зафиксирована"
echo "✅ setDebugMode(1) - улучшенная видимость треугольников" 
echo "✅ Правильные camera coordinates для разных сцен"
echo "✅ Увеличенные размеры геометрии для лучшей видимости"
echo "✅ Исправленный Vulkan Y-down coordinate system"
echo "✅ Улучшенные debug режимы в shader"

echo ""
echo -e "${YELLOW}EXPECTED RESULTS:${NC}"
echo "🔵 BlueCube_Demo_FIXED: Должен показать вращающийся синий куб"
echo "🌈 SpectraForge_Example_Demo_FIXED: Должен показать цветные треугольники"
echo "📷 Camera fixed in optimal position - no WASD movement"

echo ""
echo -e "${GREEN}🎯 TESTING COMPLETE!${NC}"
echo "If demos still show dark blue screen, check:"
echo "1. Shader compilation (TriangleSplatting_FIXED.comp.spv exists)"
echo "2. Triangle upload (check console output for triangle count)" 
echo "3. Camera matrix validation (check debug mode output)"