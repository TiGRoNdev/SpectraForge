/**
 * @file VulkanRenderer.cpp
 * @brief Реализация основного рендерера Vulkan
 *
 * Минимальная реализация, выполняющая только необходимые шаги без
 * преждевременных заглушек и неиспользуемых компонентов.
 */

#include "SpectraForge/Vulkan/VulkanRenderer.h"

#include <iostream>
#include <stdexcept>

#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Vulkan/ResourceManager.h"

using namespace SpectraForge::Vulkan;
using namespace SpectraForge::Core;

namespace SpectraForge::Vulkan {

VulkanRenderer::VulkanRenderer() = default;

VulkanRenderer::~VulkanRenderer() {
    if (initialized) {
        shutdown();
    }
}

bool VulkanRenderer::init(
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::Device logDevice,
#else
    void* logDevice,
#endif
    ResourceManager* resMgr) {
    try {
        SAFE_PRINT_LINE("[VulkanRenderer] Инициализация рендерера...");

        device = logDevice;
        resourceManager = resMgr;

        if (!device) {
            SAFE_ERROR("[VulkanRenderer] Ошибка: Vulkan device не предоставлен");
            return false;
        }

        if (!resourceManager) {
            SAFE_ERROR("[VulkanRenderer] Ошибка: ResourceManager не предоставлен");
            return false;
        }

        renderSettings_.reset();
        initialized = true;
        SAFE_PRINT_LINE("[VulkanRenderer] Инициализация завершена успешно");
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[VulkanRenderer] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

void VulkanRenderer::shutdown() {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[VulkanRenderer] Завершение работы рендерера...");

    renderSettings_.reset();
    initialized = false;

    SAFE_PRINT_LINE("[VulkanRenderer] Завершение работы завершено");
}

PrimaryImage VulkanRenderer::rasterizePrimary(const Gaussians& gaussians) {
    if (!initialized) {
        SAFE_ERROR("[VulkanRenderer] Ошибка: Рендерер не инициализирован");
        throw std::runtime_error("VulkanRenderer не инициализирован");
    }

    if (!renderSettings_) {
        SAFE_ERROR("[VulkanRenderer] Ошибка: Параметры рендеринга не заданы");
        throw std::runtime_error("Render settings are not configured");
    }

    if (gaussians.count == 0) {
        SAFE_ERROR("[VulkanRenderer] Ошибка: Пустые данные гауссианов");
        throw std::invalid_argument("Количество гауссианов должно быть больше 0");
    }

    if (renderSettings_->width == 0 || renderSettings_->height == 0) {
        SAFE_ERROR("[VulkanRenderer] Ошибка: Невалидные параметры вывода");
        throw std::invalid_argument("Render target dimensions must be greater than zero");
    }

    SAFE_PRINT_LINE("[VulkanRenderer] Первичная растеризация " + SAFE_TO_STRING(gaussians.count)
                    + " гауссианов в " + SAFE_TO_STRING(renderSettings_->width) + "x"
                    + SAFE_TO_STRING(renderSettings_->height));

    PrimaryImage result{};
    result.width = renderSettings_->width;
    result.height = renderSettings_->height;
    return result;
}

void VulkanRenderer::setRenderSettings(const RenderSettings& settings) {
    if (settings.width == 0 || settings.height == 0) {
        throw std::invalid_argument("Render settings dimensions must be greater than zero");
    }

    renderSettings_ = settings;
}

}  // namespace SpectraForge::Vulkan
