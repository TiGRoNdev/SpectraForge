#include <benchmark/benchmark.h>
#include <memory>
#include <vector>
#include "Engine3D/Rendering/RendererAdapter.h"
#include "TestFramework.h"
#include "mocks/MockRenderer.h"
#include "mocks/MockVulkanRenderer.h"

using namespace HyperEngine::Testing;
using namespace HyperEngine::Testing::Mocks;
using namespace Engine3D::Rendering;

// Глобальные переменные для бенчмарков
class RenderingBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp(const ::benchmark::State& state) override {
        // Создание mock рендерера для бенчмарков
        mockRenderer = MockFactory::createBasicRenderer();

        // Создание тестовых данных
        testMesh = std::make_shared<Mesh3D>();
        testShader = std::make_shared<Shader3D>();
        testTransform = Engine3D::Math::Matrix4::identity();

        // Создание камеры
        testCamera = std::make_shared<Camera3D>();
        testCamera->setPosition(Engine3D::Math::Vector3(0, 0, 5));
        testCamera->lookAt(Engine3D::Math::Vector3(0, 0, 5),
                           Engine3D::Math::Vector3(0, 0, 0),
                           Engine3D::Math::Vector3(0, 1, 0));
    }

    void TearDown(const ::benchmark::State& state) override {
        mockRenderer.reset();
        testMesh.reset();
        testShader.reset();
        testCamera.reset();
    }

  protected:
    std::unique_ptr<MockRendererAdapter> mockRenderer;
    std::shared_ptr<Mesh3D> testMesh;
    std::shared_ptr<Shader3D> testShader;
    std::shared_ptr<Camera3D> testCamera;
    Engine3D::Math::Matrix4 testTransform;
};

// Бенчмарк инициализации рендерера
BENCHMARK_F(RenderingBenchmarkFixture, BM_RendererInitialization)(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto renderer = std::make_unique<RendererAdapter>();
        state.ResumeTiming();

        bool result = renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
        benchmark::DoNotOptimize(result);

        state.PauseTiming();
        renderer->cleanup();
        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations());
}

// Бенчмарк базового рендеринга кадра
BENCHMARK_F(RenderingBenchmarkFixture, BM_BasicFrameRendering)(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    for (auto _ : state) {
        renderer->beginFrame();
        renderer->clear();
        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations());
    state.SetLabel("1920x1080 frames");
}

// Бенчмарк рендеринга одного объекта
BENCHMARK_F(RenderingBenchmarkFixture, BM_SingleObjectRendering)(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    for (auto _ : state) {
        renderer->beginFrame();
        renderer->renderMesh(testMesh, testTransform, testShader);
        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations());
}

// Бенчмарк рендеринга множественных объектов
BENCHMARK_F(RenderingBenchmarkFixture, BM_MultipleObjectRendering)(benchmark::State& state) {
    const int objectCount = static_cast<int>(state.range(0));

    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    // Подготовка трансформаций для объектов
    std::vector<Engine3D::Math::Matrix4> transforms(objectCount);
    for (int i = 0; i < objectCount; ++i) {
        transforms[i] = Engine3D::Math::Matrix4::translation(
            static_cast<float>(i % 10) - 5.0f, static_cast<float>(i / 10) - 5.0f, 0.0f);
    }

    for (auto _ : state) {
        renderer->beginFrame();

        for (int i = 0; i < objectCount; ++i) {
            renderer->renderMesh(testMesh, transforms[i], testShader);
        }

        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations() * objectCount);
    state.SetLabel(std::to_string(objectCount) + " objects/frame");
}
BENCHMARK_F(RenderingBenchmarkFixture, BM_MultipleObjectRendering)->Range(1, 1000);

// Бенчмарк wireframe рендеринга
BENCHMARK_F(RenderingBenchmarkFixture, BM_WireframeRendering)(benchmark::State& state) {
    const int objectCount = static_cast<int>(state.range(0));

    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    for (auto _ : state) {
        renderer->beginFrame();
        renderer->enableWireframe(true);

        for (int i = 0; i < objectCount; ++i) {
            Engine3D::Math::Matrix4 transform = Engine3D::Math::Matrix4::translation(
                static_cast<float>(i % 10), static_cast<float>(i / 10), 0.0f);
            renderer->renderWireframe(*testMesh, transform, *testShader);
        }

        renderer->enableWireframe(false);
        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations() * objectCount);
}
BENCHMARK_F(RenderingBenchmarkFixture, BM_WireframeRendering)->Range(10, 500);

// Бенчмарк различных разрешений
static void BM_DifferentResolutions(benchmark::State& state) {
    const int width = static_cast<int>(state.range(0));
    const int height = static_cast<int>(state.range(1));

    auto renderer = std::make_unique<RendererAdapter>();
    auto testMesh = std::make_shared<Mesh3D>();
    auto testShader = std::make_shared<Shader3D>();
    auto testCamera = std::make_shared<Camera3D>();

    testCamera->setPosition(Engine3D::Math::Vector3(0, 0, 5));
    testCamera->lookAt(Engine3D::Math::Vector3(0, 0, 5),
                       Engine3D::Math::Vector3(0, 0, 0),
                       Engine3D::Math::Vector3(0, 1, 0));

    renderer->initialize(RenderBackend::OPENGL, width, height);
    renderer->setMainCamera(testCamera);

    for (auto _ : state) {
        renderer->beginFrame();
        renderer->clear();
        renderer->setViewport(0, 0, width, height);

        // Рендерим несколько объектов для реалистичной нагрузки
        for (int i = 0; i < 10; ++i) {
            Engine3D::Math::Matrix4 transform =
                Engine3D::Math::Matrix4::translation(static_cast<float>(i - 5), 0.0f, 0.0f);
            renderer->renderMesh(testMesh, transform, testShader);
        }

        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations());
    state.SetLabel(std::to_string(width) + "x" + std::to_string(height));

    // Вычисляем пиксели в секунду
    int64_t pixels_per_frame = static_cast<int64_t>(width) * height;
    state.counters["Pixels/sec"] = benchmark::Counter(
        static_cast<double>(pixels_per_frame * state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_DifferentResolutions)
    ->Args({800, 600})
    ->Args({1920, 1080})
    ->Args({2560, 1440})
    ->Args({3840, 2160});

// Бенчмарк переключения состояний рендеринга
BENCHMARK_F(RenderingBenchmarkFixture, BM_StateChanges)(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    for (auto _ : state) {
        renderer->beginFrame();

        // Множественные изменения состояния
        renderer->enableDepthTest(true);
        renderer->enableBlending(true);
        renderer->setClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        renderer->setViewport(0, 0, 1920, 1080);
        renderer->enableWireframe(true);
        renderer->enableBackfaceCulling(false);

        renderer->renderMesh(testMesh, testTransform, testShader);

        renderer->enableDepthTest(false);
        renderer->enableBlending(false);
        renderer->enableWireframe(false);
        renderer->enableBackfaceCulling(true);

        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations());
}

// Бенчмарк управления камерой
BENCHMARK_F(RenderingBenchmarkFixture, BM_CameraManagement)(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);

    std::vector<std::shared_ptr<Camera3D>> cameras;
    for (int i = 0; i < 10; ++i) {
        auto camera = std::make_shared<Camera3D>();
        camera->setPosition(Engine3D::Math::Vector3(static_cast<float>(i), 0.0f, 5.0f));
        cameras.push_back(camera);
    }

    for (auto _ : state) {
        for (auto& camera : cameras) {
            renderer->setMainCamera(camera);
            auto retrievedCamera = renderer->getMainCamera();
            benchmark::DoNotOptimize(retrievedCamera);
        }
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations() * cameras.size());
}

// Бенчмарк создания и уничтожения рендерера
static void BM_RendererLifecycle(benchmark::State& state) {
    for (auto _ : state) {
        auto renderer = std::make_unique<RendererAdapter>();

        bool initResult = renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
        benchmark::DoNotOptimize(initResult);

        renderer->cleanup();
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_RendererLifecycle);

// Бенчмарк переключения backend'ов
static void BM_BackendSwitching(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();

    // Инициализируем с OpenGL
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);

    for (auto _ : state) {
        // Переключаемся между backend'ами (если доступны)
        if (renderer->isBackendAvailable(RenderBackend::VULKAN)) {
            bool switchResult = renderer->switchBackend(RenderBackend::VULKAN);
            benchmark::DoNotOptimize(switchResult);

            switchResult = renderer->switchBackend(RenderBackend::OPENGL);
            benchmark::DoNotOptimize(switchResult);
        } else {
            // Если Vulkan недоступен, просто переинициализируем OpenGL
            renderer->cleanup();
            bool initResult = renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
            benchmark::DoNotOptimize(initResult);
        }
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BackendSwitching);

// Стресс-тест для долгосрочной стабильности
static void BM_LongRunningRendering(benchmark::State& state) {
    auto renderer = std::make_unique<RendererAdapter>();
    auto testMesh = std::make_shared<Mesh3D>();
    auto testShader = std::make_shared<Shader3D>();
    auto testCamera = std::make_shared<Camera3D>();

    testCamera->setPosition(Engine3D::Math::Vector3(0, 0, 5));

    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);
    renderer->setMainCamera(testCamera);

    const int objectsPerFrame = 50;

    for (auto _ : state) {
        renderer->beginFrame();

        for (int i = 0; i < objectsPerFrame; ++i) {
            Engine3D::Math::Matrix4 transform =
                Engine3D::Math::Matrix4::translation(std::sin(static_cast<float>(i) * 0.1f) * 5.0f,
                                                     std::cos(static_cast<float>(i) * 0.1f) * 5.0f,
                                                     0.0f);
            renderer->renderMesh(testMesh, transform, testShader);
        }

        renderer->endFrame();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations() * objectsPerFrame);
    state.SetLabel("Stress test");
}
BENCHMARK(BM_LongRunningRendering)->Iterations(10000);

// Бенчмарк потребления памяти
static void BM_MemoryUsage(benchmark::State& state) {
    const int objectCount = static_cast<int>(state.range(0));

    auto renderer = std::make_unique<RendererAdapter>();
    renderer->initialize(RenderBackend::OPENGL, 1920, 1080);

    std::vector<std::shared_ptr<Mesh3D>> meshes(objectCount);
    std::vector<std::shared_ptr<Shader3D>> shaders(objectCount);

    for (auto _ : state) {
        state.PauseTiming();

        // Создаем объекты
        for (int i = 0; i < objectCount; ++i) {
            meshes[i] = std::make_shared<Mesh3D>();
            shaders[i] = std::make_shared<Shader3D>();
        }

        state.ResumeTiming();

        // Используем объекты
        renderer->beginFrame();
        for (int i = 0; i < objectCount; ++i) {
            Engine3D::Math::Matrix4 transform = Engine3D::Math::Matrix4::identity();
            renderer->renderMesh(meshes[i], transform, shaders[i]);
        }
        renderer->endFrame();

        state.PauseTiming();

        // Очищаем объекты
        for (int i = 0; i < objectCount; ++i) {
            meshes[i].reset();
            shaders[i].reset();
        }

        state.ResumeTiming();
    }

    renderer->cleanup();

    state.SetItemsProcessed(state.iterations() * objectCount);
    state.SetLabel(std::to_string(objectCount) + " temp objects");
}
BENCHMARK(BM_MemoryUsage)->Range(1, 1000);

// Настройка для запуска бенчмарков
int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);

    // Добавляем контекст о системе
    benchmark::AddCustomContext("Engine", "HyperEngine");
    benchmark::AddCustomContext("Graphics API", "OpenGL/Vulkan");
    benchmark::AddCustomContext("Test Type", "Rendering Performance");

    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}
