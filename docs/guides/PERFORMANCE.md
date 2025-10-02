# Руководство по производительности SpectraForge

## 🚀 Обзор

Данное руководство содержит рекомендации по оптимизации производительности при работе с SpectraForge, включая настройки рендеринга, профилирование, оптимизацию CUDA-Vulkan интеграции и использование AI-ускорения.

## 📊 Целевые показатели производительности

### Базовые метрики v1.0.0

| Разрешение | Целевой FPS | Время кадра | Использование памяти |
|------------|-------------|-------------|---------------------|
| 1080p      | 60+         | <16ms       | <200MB              |
| 1440p      | 45+         | <22ms       | <300MB              |
| 4K         | 30+         | <33ms       | <500MB              |

### Расширенные метрики

| Метрика | Целевое значение | Критическое значение |
|---------|------------------|---------------------|
| GPU Utilization | >90% | <70% |
| CPU Utilization | 50-70% | >90% |
| VRAM Usage | <80% доступной | >95% |
| Draw Calls | <1000/кадр | >2000/кадр |
| Triangle Count | <1M/кадр | >5M/кадр |

## 🔧 Настройка рендеринга

### Автоматическое определение оптимальных настроек

```cpp
#include "SpectraForge/Core/HardwareDetector.h"
#include "SpectraForge/Rendering/AdaptiveQuality.h"

void setupOptimalSettings() {
    // Определение аппаратных возможностей
    VendorType vendor = HardwareDetector::detectGPUVendor();
    bool rtSupport = HardwareDetector::supportsRayTracing();
    bool dlssSupport = HardwareDetector::supportsDLSS();
    bool fsrSupport = HardwareDetector::supportsFSR();
    
    // Настройка рендерера
    RendererConfig config;
    
    if (vendor == VendorType::NVIDIA && dlssSupport) {
        config.upscaler = UpscalerType::DLSS;
        config.renderScale = 0.67f; // Рендеринг в 67% разрешения
        Console::info("Используется DLSS для повышения производительности");
    } else if (fsrSupport) {
        config.upscaler = UpscalerType::FSR;
        config.renderScale = 0.77f; // Рендеринг в 77% разрешения
        Console::info("Используется FSR для повышения производительности");
    }
    
    if (rtSupport) {
        config.rayTracingEnabled = true;
        config.rayTracingSamples = 1; // Минимальное количество для real-time
        Console::info("Ray Tracing включен");
    }
    
    renderer->configure(config);
}
```

### Адаптивное качество

```cpp
class AdaptiveQualityManager {
private:
    float targetFPS = 60.0f;
    float currentRenderScale = 1.0f;
    bool rayTracingEnabled = true;
    int shadowQuality = 3; // 0-4 уровни качества
    
public:
    void updateQuality(float currentFPS, float frameTime) {
        PROFILE_FUNCTION();
        
        // Анализ производительности
        if (currentFPS < targetFPS * 0.9f) {
            // Производительность ниже целевой - снижаем качество
            degradeQuality();
        } else if (currentFPS > targetFPS * 1.1f && frameTime < 14.0f) {
            // Есть запас производительности - повышаем качество
            improveQuality();
        }
    }
    
private:
    void degradeQuality() {
        if (currentRenderScale > 0.5f) {
            currentRenderScale -= 0.05f;
            renderer->setRenderScale(currentRenderScale);
            Console::debug("Снижен render scale до " + SAFE_TO_STRING(currentRenderScale));
            return;
        }
        
        if (shadowQuality > 1) {
            shadowQuality--;
            renderer->setShadowQuality(shadowQuality);
            Console::debug("Снижено качество теней до " + SAFE_TO_STRING(shadowQuality));
            return;
        }
        
        if (rayTracingEnabled) {
            rayTracingEnabled = false;
            renderer->enableRayTracing(false);
            Console::debug("Ray Tracing отключен для повышения производительности");
        }
    }
    
    void improveQuality() {
        if (!rayTracingEnabled && HardwareDetector::supportsRayTracing()) {
            rayTracingEnabled = true;
            renderer->enableRayTracing(true);
            Console::debug("Ray Tracing включен");
            return;
        }
        
        if (shadowQuality < 4) {
            shadowQuality++;
            renderer->setShadowQuality(shadowQuality);
            Console::debug("Повышено качество теней до " + SAFE_TO_STRING(shadowQuality));
            return;
        }
        
        if (currentRenderScale < 1.0f) {
            currentRenderScale += 0.05f;
            currentRenderScale = std::min(currentRenderScale, 1.0f);
            renderer->setRenderScale(currentRenderScale);
            Console::debug("Повышен render scale до " + SAFE_TO_STRING(currentRenderScale));
        }
    }
};
```

## ⚡ CUDA-Vulkan оптимизация

### Эффективное использование FlashGS

```cpp
class OptimizedGaussianRenderer {
private:
    CudaVulkanInterop interop;
    cudaStream_t computeStream;
    cudaStream_t copyStream;
    
public:
    void renderGaussians(const std::vector<GaussianData>& gaussians, 
                        const CameraParams& camera) {
        PROFILE_SCOPE("Gaussian Rendering");
        
        // Асинхронная загрузка данных
        {
            PROFILE_SCOPE("Data Upload");
            interop.uploadGaussianDataAsync(gaussians, copyStream);
        }
        
        // Параллельная обработка на CUDA
        {
            PROFILE_SCOPE("CUDA Processing");
            
            // Сортировка по глубине (критично для качества)
            depthSortGaussians<<<blocks, threads, 0, computeStream>>>(
                gaussianData, gaussians.size(), camera.position
            );
            
            // Оптимизация тайлов для рендеринга
            optimizeTileRasterization<<<tileBlocks, tileThreads, 0, computeStream>>>(
                tileData, numTiles, outputBuffer
            );
            
            // Основная растеризация (4x ускорение по сравнению с CPU)
            rasterizeGaussians<<<rasterBlocks, rasterThreads, 0, computeStream>>>(
                gaussianData, gaussians.size(), camera, 
                outputBuffer, screenWidth, screenHeight
            );
        }
        
        // Синхронизация с Vulkan
        {
            PROFILE_SCOPE("Vulkan Sync");
            interop.signalFromCuda(computeStream);
            vulkanRenderer->waitForCuda();
        }
    }
};
```

### Оптимизация памяти

```cpp
class MemoryOptimizer {
private:
    size_t maxVRAM;
    size_t currentUsage = 0;
    std::unordered_map<std::string, size_t> resourceSizes;
    
public:
    void initialize() {
        // Определение доступной VRAM
        size_t free, total;
        cudaMemGetInfo(&free, &total);
        maxVRAM = total * 0.8f; // Используем 80% от доступной памяти
        
        Console::info("Доступно VRAM: " + SAFE_TO_STRING(total / (1024*1024)) + " MB");
        Console::info("Лимит использования: " + SAFE_TO_STRING(maxVRAM / (1024*1024)) + " MB");
    }
    
    bool canAllocate(size_t size) {
        return (currentUsage + size) <= maxVRAM;
    }
    
    void trackAllocation(const std::string& resourceId, size_t size) {
        resourceSizes[resourceId] = size;
        currentUsage += size;
        
        if (currentUsage > maxVRAM * 0.9f) {
            Console::warning("Использование VRAM превышает 90%: " + 
                           SAFE_TO_STRING(currentUsage / (1024*1024)) + " MB");
            freeUnusedResources();
        }
    }
    
private:
    void freeUnusedResources() {
        // Освобождение неиспользуемых ресурсов
        resourceManager->cleanupUnusedResources();
        
        // Принудительная сборка мусора
        cudaDeviceSynchronize();
        
        // Обновление статистики
        size_t free, total;
        cudaMemGetInfo(&free, &total);
        currentUsage = total - free;
    }
};
```

## 🎯 Профилирование и мониторинг

### Встроенная система профилирования

```cpp
class PerformanceProfiler {
private:
    struct FrameData {
        float frameTime;
        float cpuTime;
        float gpuTime;
        int drawCalls;
        int triangles;
        size_t memoryUsage;
        std::chrono::high_resolution_clock::time_point timestamp;
    };
    
    std::vector<FrameData> frameHistory;
    static constexpr size_t MAX_HISTORY = 300; // 5 секунд при 60 FPS
    
public:
    void beginFrame() {
        PROFILE_FUNCTION();
        currentFrame.timestamp = std::chrono::high_resolution_clock::now();
        
        // Сброс счетчиков кадра
        currentFrame.drawCalls = 0;
        currentFrame.triangles = 0;
    }
    
    void endFrame() {
        PROFILE_FUNCTION();
        
        auto now = std::chrono::high_resolution_clock::now();
        currentFrame.frameTime = std::chrono::duration<float, std::milli>(
            now - currentFrame.timestamp).count();
        
        // Получение GPU времени
        currentFrame.gpuTime = getGPUTime();
        currentFrame.cpuTime = currentFrame.frameTime - currentFrame.gpuTime;
        
        // Получение использования памяти
        currentFrame.memoryUsage = getCurrentMemoryUsage();
        
        // Сохранение в историю
        frameHistory.push_back(currentFrame);
        if (frameHistory.size() > MAX_HISTORY) {
            frameHistory.erase(frameHistory.begin());
        }
        
        // Анализ производительности каждые 60 кадров
        if (frameHistory.size() % 60 == 0) {
            analyzePerformance();
        }
    }
    
private:
    void analyzePerformance() {
        if (frameHistory.empty()) return;
        
        // Вычисление средних значений за последние 60 кадров
        float avgFrameTime = 0;
        float avgGPUTime = 0;
        int avgDrawCalls = 0;
        
        size_t sampleSize = std::min(frameHistory.size(), size_t(60));
        for (size_t i = frameHistory.size() - sampleSize; i < frameHistory.size(); ++i) {
            avgFrameTime += frameHistory[i].frameTime;
            avgGPUTime += frameHistory[i].gpuTime;
            avgDrawCalls += frameHistory[i].drawCalls;
        }
        
        avgFrameTime /= sampleSize;
        avgGPUTime /= sampleSize;
        avgDrawCalls /= sampleSize;
        
        float avgFPS = 1000.0f / avgFrameTime;
        
        // Вывод статистики
        Console::info("=== Статистика производительности ===");
        Console::info("FPS: " + SAFE_TO_STRING(avgFPS));
        Console::info("Frame Time: " + SAFE_TO_STRING(avgFrameTime) + "ms");
        Console::info("GPU Time: " + SAFE_TO_STRING(avgGPUTime) + "ms");
        Console::info("CPU Time: " + SAFE_TO_STRING(avgFrameTime - avgGPUTime) + "ms");
        Console::info("Draw Calls: " + SAFE_TO_STRING(avgDrawCalls));
        Console::info("Memory: " + SAFE_TO_STRING(getCurrentMemoryUsage() / (1024*1024)) + "MB");
        
        // Предупреждения о производительности
        if (avgFPS < 30.0f) {
            Console::warning("Низкий FPS! Рекомендуется снизить настройки качества");
        }
        
        if (avgGPUTime > avgFrameTime * 0.9f) {
            Console::warning("GPU является узким местом");
        } else if ((avgFrameTime - avgGPUTime) > avgFrameTime * 0.7f) {
            Console::warning("CPU является узким местом");
        }
    }
};
```

### GPU профилирование

```cpp
class GPUProfiler {
private:
    VkQueryPool queryPool;
    std::vector<uint64_t> timestamps;
    
public:
    void initialize(VkDevice device) {
        VkQueryPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        poolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        poolInfo.queryCount = 32; // Максимум 16 пар begin/end
        
        VK_CHECK(vkCreateQueryPool(device, &poolInfo, nullptr, &queryPool));
        timestamps.resize(32);
    }
    
    void beginGPUTimer(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                           queryPool, queryIndex * 2);
    }
    
    void endGPUTimer(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
                           queryPool, queryIndex * 2 + 1);
    }
    
    float getGPUTime(uint32_t queryIndex) {
        VK_CHECK(vkGetQueryPoolResults(device, queryPool, 
                                      queryIndex * 2, 2,
                                      sizeof(uint64_t) * 2,
                                      &timestamps[queryIndex * 2],
                                      sizeof(uint64_t),
                                      VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
        
        uint64_t startTime = timestamps[queryIndex * 2];
        uint64_t endTime = timestamps[queryIndex * 2 + 1];
        
        // Преобразование в миллисекунды
        float timestampPeriod = physicalDeviceProperties.limits.timestampPeriod;
        return (endTime - startTime) * timestampPeriod / 1000000.0f;
    }
};
```

## 🎮 Оптимизация игрового цикла

### Эффективный игровой цикл

```cpp
class OptimizedGameLoop {
private:
    static constexpr float TARGET_FPS = 60.0f;
    static constexpr float TARGET_FRAME_TIME = 1000.0f / TARGET_FPS;
    
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float deltaTime = 0.0f;
    float accumulator = 0.0f;
    
    AdaptiveQualityManager qualityManager;
    PerformanceProfiler profiler;
    
public:
    void run() {
        lastFrameTime = std::chrono::high_resolution_clock::now();
        
        while (engine->isRunning()) {
            PROFILE_SCOPE("Game Loop");
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::milli>(
                currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;
            
            // Ограничение максимального времени кадра для стабильности
            frameTime = std::min(frameTime, 50.0f); // Максимум 50ms
            
            profiler.beginFrame();
            
            // Обновление с фиксированным временным шагом
            accumulator += frameTime;
            while (accumulator >= TARGET_FRAME_TIME) {
                PROFILE_SCOPE("Fixed Update");
                engine->fixedUpdate(TARGET_FRAME_TIME / 1000.0f);
                accumulator -= TARGET_FRAME_TIME;
            }
            
            // Обновление с переменным временным шагом
            {
                PROFILE_SCOPE("Variable Update");
                engine->update(frameTime / 1000.0f);
            }
            
            // Рендеринг
            {
                PROFILE_SCOPE("Render");
                engine->render();
            }
            
            profiler.endFrame();
            
            // Адаптивное управление качеством
            float currentFPS = 1000.0f / frameTime;
            qualityManager.updateQuality(currentFPS, frameTime);
            
            // Ограничение FPS если необходимо
            limitFrameRate(frameTime);
        }
    }
    
private:
    void limitFrameRate(float frameTime) {
        if (frameTime < TARGET_FRAME_TIME) {
            float sleepTime = TARGET_FRAME_TIME - frameTime;
            std::this_thread::sleep_for(
                std::chrono::microseconds(static_cast<int>(sleepTime * 1000))
            );
        }
    }
};
```

## 📈 Оптимизация ресурсов

### Умное управление ресурсами

```cpp
class SmartResourceManager {
private:
    struct ResourceInfo {
        std::shared_ptr<Resource> resource;
        std::chrono::steady_clock::time_point lastAccessed;
        size_t memorySize;
        int accessCount;
        bool isEssential; // Критичные ресурсы не выгружаются
    };
    
    std::unordered_map<std::string, ResourceInfo> resources;
    size_t totalMemoryUsage = 0;
    size_t memoryLimit;
    
public:
    void initialize() {
        // Установка лимита памяти (80% от доступной VRAM)
        size_t free, total;
        cudaMemGetInfo(&free, &total);
        memoryLimit = total * 0.8f;
        
        Console::info("Лимит памяти для ресурсов: " + 
                     SAFE_TO_STRING(memoryLimit / (1024*1024)) + " MB");
    }
    
    template<typename T>
    std::shared_ptr<T> loadResource(const std::string& path, bool essential = false) {
        PROFILE_FUNCTION();
        
        // Проверка кэша
        auto it = resources.find(path);
        if (it != resources.end()) {
            it->second.lastAccessed = std::chrono::steady_clock::now();
            it->second.accessCount++;
            return std::static_pointer_cast<T>(it->second.resource);
        }
        
        // Загрузка нового ресурса
        auto resource = std::make_shared<T>();
        if (!resource->loadFromFile(path)) {
            Console::error("Не удалось загрузить ресурс: " + path);
            return nullptr;
        }
        
        size_t resourceSize = resource->getMemorySize();
        
        // Проверка лимита памяти
        if (totalMemoryUsage + resourceSize > memoryLimit) {
            freeUnusedResources(resourceSize);
        }
        
        // Добавление в кэш
        ResourceInfo info;
        info.resource = resource;
        info.lastAccessed = std::chrono::steady_clock::now();
        info.memorySize = resourceSize;
        info.accessCount = 1;
        info.isEssential = essential;
        
        resources[path] = info;
        totalMemoryUsage += resourceSize;
        
        Console::debug("Загружен ресурс: " + path + 
                      " (" + SAFE_TO_STRING(resourceSize / 1024) + " KB)");
        
        return std::static_pointer_cast<T>(resource);
    }
    
private:
    void freeUnusedResources(size_t requiredSpace) {
        PROFILE_FUNCTION();
        
        // Сортировка ресурсов по приоритету выгрузки
        std::vector<std::pair<std::string, float>> candidates;
        auto now = std::chrono::steady_clock::now();
        
        for (const auto& [path, info] : resources) {
            if (info.isEssential) continue;
            
            // Вычисление приоритета (меньше = выгружается первым)
            auto timeSinceAccess = std::chrono::duration_cast<std::chrono::seconds>(
                now - info.lastAccessed).count();
            
            float priority = static_cast<float>(info.accessCount) / 
                           (timeSinceAccess + 1.0f);
            
            candidates.emplace_back(path, priority);
        }
        
        // Сортировка по приоритету
        std::sort(candidates.begin(), candidates.end(),
                 [](const auto& a, const auto& b) {
                     return a.second < b.second;
                 });
        
        // Выгрузка ресурсов
        size_t freedSpace = 0;
        for (const auto& [path, priority] : candidates) {
            auto it = resources.find(path);
            if (it != resources.end()) {
                freedSpace += it->second.memorySize;
                totalMemoryUsage -= it->second.memorySize;
                
                Console::debug("Выгружен ресурс: " + path);
                resources.erase(it);
                
                if (freedSpace >= requiredSpace) break;
            }
        }
        
        Console::info("Освобождено памяти: " + SAFE_TO_STRING(freedSpace / (1024*1024)) + " MB");
    }
};
```

## 🔍 Диагностика производительности

### Автоматическая диагностика

```cpp
class PerformanceDiagnostics {
public:
    struct DiagnosticReport {
        std::string issue;
        std::string recommendation;
        float severity; // 0.0 - 1.0
    };
    
    std::vector<DiagnosticReport> analyzePerformance(const PerformanceData& data) {
        std::vector<DiagnosticReport> reports;
        
        // Анализ FPS
        if (data.averageFPS < 30.0f) {
            reports.push_back({
                "Критически низкий FPS: " + SAFE_TO_STRING(data.averageFPS),
                "Снизьте разрешение рендеринга или отключите ray tracing",
                0.9f
            });
        } else if (data.averageFPS < 45.0f) {
            reports.push_back({
                "Низкий FPS: " + SAFE_TO_STRING(data.averageFPS),
                "Рассмотрите снижение качества теней или эффектов",
                0.6f
            });
        }
        
        // Анализ времени GPU
        float gpuPercent = data.averageGPUTime / data.averageFrameTime;
        if (gpuPercent > 0.9f) {
            reports.push_back({
                "GPU является узким местом (" + SAFE_TO_STRING(gpuPercent * 100) + "%)",
                "Снизьте разрешение рендеринга или используйте DLSS/FSR",
                0.7f
            });
        }
        
        // Анализ времени CPU
        float cpuPercent = (data.averageFrameTime - data.averageGPUTime) / data.averageFrameTime;
        if (cpuPercent > 0.7f) {
            reports.push_back({
                "CPU является узким местом (" + SAFE_TO_STRING(cpuPercent * 100) + "%)",
                "Оптимизируйте логику игры или используйте многопоточность",
                0.6f
            });
        }
        
        // Анализ использования памяти
        float memoryPercent = static_cast<float>(data.memoryUsage) / data.totalMemory;
        if (memoryPercent > 0.9f) {
            reports.push_back({
                "Критическое использование памяти: " + SAFE_TO_STRING(memoryPercent * 100) + "%",
                "Снизьте качество текстур или включите сжатие",
                0.8f
            });
        }
        
        // Анализ draw calls
        if (data.averageDrawCalls > 1500) {
            reports.push_back({
                "Слишком много draw calls: " + SAFE_TO_STRING(data.averageDrawCalls),
                "Используйте instancing или объединение мешей",
                0.5f
            });
        }
        
        return reports;
    }
    
    void printDiagnosticReport(const std::vector<DiagnosticReport>& reports) {
        if (reports.empty()) {
            Console::success("Производительность в норме!");
            return;
        }
        
        Console::warning("=== Диагностика производительности ===");
        
        // Сортировка по серьезности
        auto sortedReports = reports;
        std::sort(sortedReports.begin(), sortedReports.end(),
                 [](const auto& a, const auto& b) {
                     return a.severity > b.severity;
                 });
        
        for (const auto& report : sortedReports) {
            if (report.severity >= 0.8f) {
                Console::error("🔴 " + report.issue);
                Console::error("   💡 " + report.recommendation);
            } else if (report.severity >= 0.6f) {
                Console::warning("🟡 " + report.issue);
                Console::warning("   💡 " + report.recommendation);
            } else {
                Console::info("🟢 " + report.issue);
                Console::info("   💡 " + report.recommendation);
            }
        }
    }
};
```

## 📋 Чек-лист оптимизации

### Перед релизом

- [ ] **Профилирование**: Проведено полное профилирование на целевом оборудовании
- [ ] **Адаптивное качество**: Настроена система автоматической адаптации качества
- [ ] **Управление памятью**: Реализована система умного управления ресурсами
- [ ] **GPU оптимизация**: Настроена CUDA-Vulkan интеграция для максимальной производительности
- [ ] **AI ускорение**: Включены DLSS/FSR для поддерживаемых GPU
- [ ] **Диагностика**: Добавлена автоматическая диагностика производительности
- [ ] **Тестирование**: Протестировано на различных конфигурациях оборудования

### Во время разработки

- [ ] **Профилирование кода**: Используйте `PROFILE_SCOPE()` для критичных участков
- [ ] **Безопасный вывод**: Всегда используйте `SAFE_TO_STRING()` для отладочного вывода
- [ ] **Управление ресурсами**: Помечайте критичные ресурсы как `essential`
- [ ] **Мониторинг памяти**: Регулярно проверяйте использование VRAM
- [ ] **Оптимизация шейдеров**: Минимизируйте сложность фрагментных шейдеров
- [ ] **Batch rendering**: Группируйте объекты для уменьшения draw calls

## 🎯 Заключение

Следование данным рекомендациям поможет достичь оптимальной производительности SpectraForge:

1. **Используйте встроенные инструменты профилирования** для выявления узких мест
2. **Настройте адаптивное качество** для автоматической оптимизации
3. **Максимально используйте GPU ускорение** через CUDA и AI технологии
4. **Мониторьте использование ресурсов** в реальном времени
5. **Тестируйте на различном оборудовании** для обеспечения совместимости

Помните: производительность - это баланс между качеством изображения и скоростью рендеринга. SpectraForge предоставляет все необходимые инструменты для достижения этого баланса.
