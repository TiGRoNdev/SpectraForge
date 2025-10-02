#!/bin/bash
# FreqVox Sponza Demo - Vulkan Presentation Version
# Запуск полноценного демо с Vulkan swapchain презентацией

cd "$(dirname "$0")"

echo "🚀 FreqVox Sponza Demo - Vulkan Presentation"
echo "============================================="
echo ""
echo "✅ Реализовано:"
echo "  - Полный Vulkan presentation layer (VkSurfaceKHR + VkSwapchainKHR)"
echo "  - Hardware-aware backend selection (cuFFT для NVIDIA, VkFFT для остальных)"
echo "  - Загрузка и вокселизация сцены Sponza"
echo "  - FreqVox pipeline (Foveation, Frequency Shading, Temporal, Upscaling)"
echo ""
echo "🎮 Управление:"
echo "  WASD       - движение камеры"
echo "  Мышь       - поворот камеры"  
echo "  Scroll     - изменение скорости"
echo "  F          - переключение фовеации"
echo "  T          - переключение temporal reprojection"
echo "  U          - переключение upscaling"
echo "  ESC        - выход"
echo ""
echo "Запуск..."
echo ""

./build/FreqVox_Sponza_Demo

echo ""
echo "Демо завершено."

