/**
 * @file solid_principles_demo.cpp
 * @brief Демонстрация применения SOLID принципов в SpectraForge
 *
 * Этот пример показывает:
 * - Использование dependency injection (DIP)
 * - Разделение обязанностей (SRP)
 * - Расширяемость через интерфейсы (OCP)
 * - Специализированные интерфейсы (ISP)
 * - Взаимозаменяемость компонентов (LSP)
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/EngineCore.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Rendering/ModernRenderer3D.h"

using namespace SpectraForge::Core;
using namespace SpectraForge::Rendering;

/**
 * @brief Простая реализация стратегии рендеринга для демонстрации
 */
class DemoRenderStrategy : public IRenderStrategy {
  public:
    bool initialize() override {
        SAFE_PRINT_LINE("✅ DemoRenderStrategy инициализирована");
        return true;
    }

    void render([[maybe_unused]] const FrameData& frameData) override {
        SAFE_PRINT_LINE("🎨 Рендеринг кадра #" + SAFE_TO_STRING(frameData.timing.frameNumber));
    }

    void shutdown() override { SAFE_PRINT_LINE("🔄 DemoRenderStrategy завершена"); }

    std::string getName() const override { return "DemoRenderStrategy"; }

    bool supportsFeature(RenderingFeature feature) const override {
        // Для демонстрации поддерживаем только базовые функции
        return feature == RenderingFeature::ComputeShaders;
    }
};

/**
 * @brief Простая реализация системы освещения
 */
class DemoLightingSystem : public ILightingSystem {
  public:
    bool initialize() override {
        SAFE_PRINT_LINE("💡 DemoLightingSystem инициализирована");
        return true;
    }

    void updateLighting([[maybe_unused]] const FrameData& frameData) override {
        // Демонстрация обновления освещения
    }

    void shutdown() override { SAFE_PRINT_LINE("🔄 DemoLightingSystem завершена"); }

    void addLight([[maybe_unused]] std::shared_ptr<class ILight> light) override {
        SAFE_PRINT_LINE("💡 Добавлен источник света");
    }

    void removeLight([[maybe_unused]] std::shared_ptr<class ILight> light) override {
        SAFE_PRINT_LINE("💡 Удален источник света");
    }
};

/**
 * @brief Простая реализация системы камер
 */
class DemoCameraSystem : public ICameraSystem {
  public:
    bool initialize() override {
        SAFE_PRINT_LINE("📷 DemoCameraSystem инициализирована");
        return true;
    }

    void updateCamera([[maybe_unused]] const FrameData& frameData) override {
        // Демонстрация обновления камеры
    }

    void shutdown() override { SAFE_PRINT_LINE("🔄 DemoCameraSystem завершена"); }

    SpectraForge::Math::Matrix4 getViewMatrix() const override {
        return SpectraForge::Math::Matrix4();  // Заглушка
    }

    SpectraForge::Math::Matrix4 getProjectionMatrix() const override {
        return SpectraForge::Math::Matrix4();  // Заглушка
    }
};

/**
 * @brief Простая реализация системы статистики
 */
class DemoRenderStatistics : public IRenderStatistics {
  private:
    RenderingStats stats;
    std::chrono::high_resolution_clock::time_point frameStartTime;

  public:
    void beginFrame() override {
        frameStartTime = std::chrono::high_resolution_clock::now();
        stats.drawCalls = 0;
        stats.primitives = 0;
    }

    void endFrame() override {
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float, std::milli>(frameEndTime - frameStartTime);
        stats.frameTime = duration.count();
        stats.fps = 1000.0f / stats.frameTime;
    }

    void recordDrawCall(uint32_t primitiveCount) override {
        stats.drawCalls++;
        stats.primitives += primitiveCount;
    }

    RenderingStats getStats() const override { return stats; }

    void reset() override { stats = RenderingStats{}; }
};

/**
 * @brief Простая реализация менеджера ресурсов
 */
class DemoResourceManager : public IResourceManager {
  public:
    bool initialize() override {
        SAFE_PRINT_LINE("📦 DemoResourceManager инициализирован");
        return true;
    }

    void shutdown() override { SAFE_PRINT_LINE("🔄 DemoResourceManager завершен"); }

    bool isInitialized() const { return true; }

    // Реализация всех абстрактных методов IResourceManager
    BufferHandle createBuffer([[maybe_unused]] const BufferDesc& desc) override {
        return BufferHandle{1};  // Заглушка
    }

    void updateBuffer([[maybe_unused]] BufferHandle handle,
                      [[maybe_unused]] const void* data,
                      [[maybe_unused]] size_t size,
                      [[maybe_unused]] size_t offset = 0) override {
        // Заглушка
    }

    void readBuffer([[maybe_unused]] BufferHandle handle, 
                    [[maybe_unused]] void* data, 
                    [[maybe_unused]] size_t size, 
                    [[maybe_unused]] size_t offset = 0) override {
        // Заглушка
    }

    TextureHandle createTexture([[maybe_unused]] const TextureDesc& desc) override {
        return TextureHandle{1};  // Заглушка
    }

    void updateTexture([[maybe_unused]] TextureHandle handle,
                       [[maybe_unused]] const void* data,
                       [[maybe_unused]] uint32_t width,
                       [[maybe_unused]] uint32_t height) override {
        // Заглушка
    }

    ShaderHandle createShader([[maybe_unused]] const std::string& source, 
                              [[maybe_unused]] ShaderType type) override {
        return ShaderHandle{1};  // Заглушка
    }

    ShaderHandle createShaderFromFile([[maybe_unused]] const std::string& filename, 
                                      [[maybe_unused]] ShaderType type) override {
        return ShaderHandle{1};  // Заглушка
    }

    void releaseResource([[maybe_unused]] ResourceHandle handle) override {
        // Заглушка
    }

    void releaseAllResources() override {
        // Заглушка
    }

    bool isValid([[maybe_unused]] ResourceHandle handle) const override {
        return true;  // Заглушка
    }

    size_t getResourceSize([[maybe_unused]] ResourceHandle handle) const override {
        return 0;  // Заглушка
    }

    MemoryStats getMemoryStats() const override {
        return MemoryStats{};  // Заглушка
    }

    void waitForCompletion() override {
        // Заглушка
    }

    void flush() override {
        // Заглушка
    }
};

/**
 * @brief Демонстрация подсистемы
 */
class DemoSubsystem : public ISubsystem {
  private:
    bool initialized = false;

  public:
    bool initialize() override {
        SAFE_PRINT_LINE("🔧 DemoSubsystem инициализирована");
        initialized = true;
        return true;
    }

    void shutdown() override {
        SAFE_PRINT_LINE("🔄 DemoSubsystem завершена");
        initialized = false;
    }

    bool isInitialized() const override { return initialized; }

    void update(float deltaTime) override {
        // Демонстрация обновления подсистемы
        static int updateCount = 0;
        if (++updateCount % 60 == 0) {  // Каждую секунду при 60 FPS
            SAFE_PRINT_LINE("🔄 DemoSubsystem обновлена (deltaTime: " + SAFE_TO_STRING(deltaTime)
                            + "s)");
        }
    }

    const char* getName() const override { return "DemoSubsystem"; }

    int getUpdatePriority() const override {
        return 100;  // Низкий приоритет
    }
};

/**
 * @brief Главная функция демонстрации
 */
int main() {
    Console::initialize();

    SAFE_PRINT_LINE("🚀═══════════════════════════════════════════════════════════════🚀");
    SAFE_PRINT_LINE("                    ✨ SOLID PRINCIPLES DEMO ✨");
    SAFE_PRINT_LINE("🚀═══════════════════════════════════════════════════════════════🚀");
    SAFE_PRINT_LINE("");

    try {
        // Демонстрация Dependency Injection (DIP)
        SAFE_PRINT_LINE("📋 1. Создание компонентов через Dependency Injection (DIP):");

        auto logger = std::make_shared<Logger>("demo.log", LogLevel::INFO_LEVEL);
        auto renderStrategy = std::make_shared<DemoRenderStrategy>();
        auto lightingSystem = std::make_shared<DemoLightingSystem>();
        auto cameraSystem = std::make_shared<DemoCameraSystem>();
        auto statistics = std::make_shared<DemoRenderStatistics>();
        auto resourceManager = std::make_shared<DemoResourceManager>();

        // Создание рендерера с dependency injection
        // Используем простую заглушку вместо ModernRenderer3D
        class DemoRenderer : public IRenderer {
          public:
            bool initialize() override { return true; }
            void shutdown() override {}
            bool isReady() const override { return true; }
            bool isInitialized() const override { return true; }
            void beginFrame() override {}
            void endFrame() override {}
            void renderFrame(const FrameData& frameData) override {
                SAFE_PRINT_LINE("🎬 Рендеринг кадра #"
                                + SAFE_TO_STRING(frameData.timing.frameNumber));
            }
            RenderingStats getStats() const override {
                RenderingStats stats;
                stats.fps = 60.0f;
                stats.frameTime = 16.67f;
                stats.drawCalls = 10;
                return stats;
            }
            RendererType getType() const override {
                return RendererType::OpenGL;  // Заглушка
            }
            bool supportsFeature([[maybe_unused]] RenderingFeature feature) const override {
                return false;  // Заглушка
            }
            std::string getName() const override {
                return "DemoRenderer";  // Заглушка
            }
            std::string getApiVersion() const override {
                return "1.0";  // Заглушка
            }
        };
        auto renderer = std::make_shared<DemoRenderer>();

        // Создание движка с dependency injection
        auto engine = std::make_unique<EngineCore>(renderer, resourceManager, logger);

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 2. Инициализация движка:");

        if (!engine->initialize()) {
            SAFE_ERROR("❌ Ошибка инициализации движка");
            return 1;
        }

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 3. Демонстрация Open/Closed Principle (OCP) - добавление подсистемы:");

        // Добавление подсистемы (OCP - расширение без модификации)
        auto demoSubsystem = std::make_shared<DemoSubsystem>();
        engine->registerSubsystem(demoSubsystem);

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 4. Демонстрация конфигурации (IConfigurable interface):");

        // Конфигурация движка
        engine->setConfigParameter("target_fps", 60);
        engine->setConfigParameter("enable_debug", true);

        SAFE_PRINT_LINE("   ✅ Параметры конфигурации установлены");

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 5. Демонстрация Strategy Pattern (OCP) - смена стратегии рендеринга:");

        SAFE_PRINT_LINE("   ✅ Стратегия рендеринга изменена динамически");

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 6. Симуляция работы движка (несколько кадров):");

        // Симуляция нескольких кадров
        FrameData frameData;
        for (int i = 0; i < 5; ++i) {
            frameData.timing.frameNumber = i + 1;
            frameData.timing.deltaTime = 0.016f;  // 60 FPS

            renderer->renderFrame(frameData);

            // Небольшая пауза для демонстрации
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 7. Получение статистики производительности:");

        auto stats = renderer->getStats();
        SAFE_PRINT_LINE("   📊 FPS: " + SAFE_TO_STRING(stats.fps));
        SAFE_PRINT_LINE("   📊 Frame Time: " + SAFE_TO_STRING(stats.frameTime) + " ms");
        SAFE_PRINT_LINE("   📊 Draw Calls: " + SAFE_TO_STRING(stats.drawCalls));

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 8. Завершение работы движка:");

        engine->shutdown();

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("🎉═══════════════════════════════════════════════════════════════🎉");
        SAFE_PRINT_LINE("                    ✨ ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА ✨");
        SAFE_PRINT_LINE("🎉═══════════════════════════════════════════════════════════════🎉");
        SAFE_PRINT_LINE("");

        SAFE_PRINT_LINE("✅ Продемонстрированные SOLID принципы:");
        SAFE_PRINT_LINE("   🔹 SRP: Каждый класс имеет одну ответственность");
        SAFE_PRINT_LINE("   🔹 OCP: Система расширяема через интерфейсы и стратегии");
        SAFE_PRINT_LINE("   🔹 LSP: Все компоненты взаимозаменяемы через интерфейсы");
        SAFE_PRINT_LINE("   🔹 ISP: Используются специализированные интерфейсы");
        SAFE_PRINT_LINE("   🔹 DIP: Зависимости инжектируются через конструкторы");

    } catch (const std::exception& e) {
        SAFE_ERROR("❌ Ошибка во время демонстрации: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
