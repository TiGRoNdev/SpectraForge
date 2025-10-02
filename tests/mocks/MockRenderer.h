#pragma once
#include <gmock/gmock.h>
#include "SpectraForge/Core/GameObject3D.h"
#include "SpectraForge/Core/Interfaces.h"
#include "SpectraForge/Rendering/RendererAdapter.h"

namespace SpectraForge::Testing::Mocks {

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Core;

/**
 * @brief Mock объект для IRendererAdapter
 *
 * Используется для изоляции тестов от реальной реализации рендеринга
 */
class MockRendererAdapter : public IRendererAdapter {
  public:
    // Основные методы жизненного цикла
    MOCK_METHOD(bool, initialize, (int width, int height), (override));
    MOCK_METHOD(void, cleanup, (), (override));
    MOCK_METHOD(bool, isInitialized, (), (const override));

    // Управление кадром
    MOCK_METHOD(void, beginFrame, (), (override));
    MOCK_METHOD(void, endFrame, (), (override));
    MOCK_METHOD(void, clear, (), (override));

    // Настройки рендеринга
    MOCK_METHOD(void, setClearColor, (float r, float g, float b, float a), (override));
    MOCK_METHOD(void, setViewport, (int x, int y, int width, int height), (override));
    MOCK_METHOD(void, enableDepthTest, (bool enable), (override));
    MOCK_METHOD(void, enableBlending, (bool enable), (override));
    MOCK_METHOD(void, enableWireframe, (bool enable), (override));
    MOCK_METHOD(void, enableBackfaceCulling, (bool enable), (override));

    // Управление камерой
    MOCK_METHOD(void, setMainCamera, (std::shared_ptr<Camera3D> camera), (override));
    MOCK_METHOD(std::shared_ptr<Camera3D>, getMainCamera, (), (const override));

    // Поддержка функций
    MOCK_METHOD(bool, supportsFeature, (const std::string& feature), (const override));

    // Рендеринг объектов
    MOCK_METHOD(void,
                renderMesh,
                (const Mesh3D& mesh,
                 const SpectraForge::Math::Matrix4& transform,
                 const Shader3D& shader),
                (override));
    MOCK_METHOD(void,
                renderMesh,
                (std::shared_ptr<Mesh3D> mesh,
                 const SpectraForge::Math::Matrix4& transform,
                 std::shared_ptr<Shader3D> shader),
                (override));
    MOCK_METHOD(void,
                renderWireframe,
                (const Mesh3D& mesh,
                 const SpectraForge::Math::Matrix4& transform,
                 const Shader3D& shader),
                (override));

    // Информация о backend
    MOCK_METHOD(RenderBackend, getBackendType, (), (const override));
    MOCK_METHOD(const char*, getBackendName, (), (const override));
};

/**
 * @brief Mock объект для IRenderable компонентов
 */
class MockRenderable : public IRenderable {
  public:
    MOCK_METHOD(void, render, (), (override));
};

/**
 * @brief Mock объект для IUpdatable компонентов
 */
class MockUpdatable : public IUpdatable {
  public:
    MOCK_METHOD(void, update, (float deltaTime), (override));
};

/**
 * @brief Mock объект для ILifecycle компонентов
 */
class MockLifecycle : public ILifecycle {
  public:
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, cleanup, (), (override));
};

/**
 * @brief Mock объект для ITransformable компонентов
 */
class MockTransformable : public ITransformable {
  public:
    MOCK_METHOD(SpectraForge::Math::Vector3, getPosition, (), (const override));
    MOCK_METHOD(void, setPosition, (const SpectraForge::Math::Vector3& position), (override));
    MOCK_METHOD(SpectraForge::Math::Matrix4, getTransformMatrix, (), (const override));
};

/**
 * @brief Mock объект для IDrawable компонентов
 */
class MockDrawable : public IDrawable {
  public:
    MOCK_METHOD(void, draw, (), (const override));
};

/**
 * @brief Mock объект для IPrimitiveFactory
 */
class MockPrimitiveFactory : public IPrimitiveFactory {
  public:
    MOCK_METHOD(std::shared_ptr<GameObject3D>,
                createPrimitive,
                (const std::string& type),
                (override));
};

/**
 * @brief Mock объект для IProjectionStrategy
 */
class MockProjectionStrategy : public IProjectionStrategy {
  public:
    MOCK_METHOD(SpectraForge::Math::Matrix4, getProjectionMatrix, (), (const override));
    MOCK_METHOD(SpectraForge::Math::Vector3,
                project,
                (const SpectraForge::Math::Vector3& point),
                (const override));
};

/**
 * @brief Утилиты для создания mock объектов с предустановленными ожиданиями
 */
class MockFactory {
  public:
    /**
     * @brief Создает mock рендерер с базовыми ожиданиями
     */
    static std::unique_ptr<MockRendererAdapter> createBasicRenderer() {
        auto mock = std::make_unique<MockRendererAdapter>();

        // Настройка базовых ожиданий
        EXPECT_CALL(*mock, initialize(testing::_, testing::_))
            .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, isInitialized()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getBackendType()).WillRepeatedly(testing::Return(RenderBackend::OPENGL));
        EXPECT_CALL(*mock, getBackendName()).WillRepeatedly(testing::Return("MockRenderer"));

        // Разрешаем все остальные вызовы
        EXPECT_CALL(*mock, cleanup()).Times(testing::AnyNumber());
        EXPECT_CALL(*mock, beginFrame()).Times(testing::AnyNumber());
        EXPECT_CALL(*mock, endFrame()).Times(testing::AnyNumber());
        EXPECT_CALL(*mock, clear()).Times(testing::AnyNumber());

        return mock;
    }

    /**
     * @brief Создает mock рендерер с отказом инициализации
     */
    static std::unique_ptr<MockRendererAdapter> createFailingRenderer() {
        auto mock = std::make_unique<MockRendererAdapter>();

        EXPECT_CALL(*mock, initialize(testing::_, testing::_))
            .WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, isInitialized()).WillRepeatedly(testing::Return(false));

        return mock;
    }

    /**
     * @brief Создает mock трансформируемый объект с базовой позицией
     */
    static std::unique_ptr<MockTransformable> createBasicTransformable() {
        auto mock = std::make_unique<MockTransformable>();

        SpectraForge::Math::Vector3 defaultPosition(0.0f, 0.0f, 0.0f);
        SpectraForge::Math::Matrix4 identityMatrix = SpectraForge::Math::Matrix4::identity();

        EXPECT_CALL(*mock, getPosition()).WillRepeatedly(testing::Return(defaultPosition));
        EXPECT_CALL(*mock, getTransformMatrix()).WillRepeatedly(testing::Return(identityMatrix));
        EXPECT_CALL(*mock, setPosition(testing::_)).Times(testing::AnyNumber());

        return mock;
    }
};

}  // namespace SpectraForge::Testing::Mocks
