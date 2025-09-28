#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace HyperEngine::Core::DI {

/**
 * @brief Контейнер для Dependency Injection
 *
 * Применяет Dependency Inversion Principle -
 * высокоуровневые модули не зависят от низкоуровневых,
 * оба зависят от абстракций.
 *
 * Поддерживает различные жизненные циклы объектов:
 * - Singleton: одинарный экземпляр на всё приложение
 * - Transient: новый экземпляр при каждом запросе
 * - Scoped: один экземпляр в рамках scope
 */
class Container {
  public:
    /**
     * @brief Жизненные циклы объектов
     */
    enum class Lifetime {
        Singleton,  ///< Одинарный экземпляр
        Transient,  ///< Новый экземпляр каждый раз
        Scoped      ///< Один экземпляр в рамках scope
    };

  private:
    /**
     * @brief Базовый класс для фабрик
     */
    class IFactory {
      public:
        virtual ~IFactory() = default;
        virtual std::shared_ptr<void> create(Container& container) = 0;
        virtual std::type_index getType() const = 0;
        virtual Lifetime getLifetime() const = 0;
    };

    /**
     * @brief Типизированная фабрика
     */
    template <typename T>
    class Factory : public IFactory {
      private:
        std::function<std::shared_ptr<T>(Container&)> factory_;
        Lifetime lifetime_;

      public:
        Factory(std::function<std::shared_ptr<T>(Container&)> factory, Lifetime lifetime)
            : factory_(std::move(factory)), lifetime_(lifetime) {}

        std::shared_ptr<void> create(Container& container) override {
            return std::static_pointer_cast<void>(factory_(container));
        }

        std::type_index getType() const override { return std::type_index(typeid(T)); }

        Lifetime getLifetime() const override { return lifetime_; }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IFactory>> factories_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> singletons_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> scopedInstances_;
    std::string scopeId_;
    mutable std::mutex mutex_;

  public:
    Container() = default;
    ~Container() = default;

    // Запретить копирование
    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;

    // Разрешить перемещение
    Container(Container&&) = default;
    Container& operator=(Container&&) = default;

    /**
     * @brief Зарегистрировать тип как Singleton
     * @tparam Interface Интерфейс или базовый тип
     * @tparam Implementation Конкретная реализация
     */
    template <typename Interface, typename Implementation>
    void registerSingleton() {
        static_assert(std::is_base_of_v<Interface, Implementation>,
                      "Implementation должен наследоваться от Interface");

        auto factory = [](Container& container) -> std::shared_ptr<Interface> {
            return std::make_shared<Implementation>();
        };

        registerSingleton<Interface>(factory);
    }

    /**
     * @brief Зарегистрировать Singleton с пользовательской фабрикой
     * @tparam Interface Тип интерфейса
     * @param factory Функция создания объекта
     */
    template <typename Interface>
    void registerSingleton(std::function<std::shared_ptr<Interface>(Container&)> factory) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));
        factories_[typeIndex] =
            std::make_unique<Factory<Interface>>(std::move(factory), Lifetime::Singleton);
    }

    /**
     * @brief Зарегистрировать тип как Transient
     * @tparam Interface Интерфейс или базовый тип
     * @tparam Implementation Конкретная реализация
     */
    template <typename Interface, typename Implementation>
    void registerTransient() {
        static_assert(std::is_base_of_v<Interface, Implementation>,
                      "Implementation должен наследоваться от Interface");

        auto factory = [](Container& container) -> std::shared_ptr<Interface> {
            return std::make_shared<Implementation>();
        };

        registerTransient<Interface>(factory);
    }

    /**
     * @brief Зарегистрировать Transient с пользовательской фабрикой
     * @tparam Interface Тип интерфейса
     * @param factory Функция создания объекта
     */
    template <typename Interface>
    void registerTransient(std::function<std::shared_ptr<Interface>(Container&)> factory) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));
        factories_[typeIndex] =
            std::make_unique<Factory<Interface>>(std::move(factory), Lifetime::Transient);
    }

    /**
     * @brief Зарегистрировать тип как Scoped
     * @tparam Interface Интерфейс или базовый тип
     * @tparam Implementation Конкретная реализация
     */
    template <typename Interface, typename Implementation>
    void registerScoped() {
        static_assert(std::is_base_of_v<Interface, Implementation>,
                      "Implementation должен наследоваться от Interface");

        auto factory = [](Container& container) -> std::shared_ptr<Interface> {
            return std::make_shared<Implementation>();
        };

        registerScoped<Interface>(factory);
    }

    /**
     * @brief Зарегистрировать Scoped с пользовательской фабрикой
     * @tparam Interface Тип интерфейса
     * @param factory Функция создания объекта
     */
    template <typename Interface>
    void registerScoped(std::function<std::shared_ptr<Interface>(Container&)> factory) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));
        factories_[typeIndex] =
            std::make_unique<Factory<Interface>>(std::move(factory), Lifetime::Scoped);
    }

    /**
     * @brief Зарегистрировать экземпляр напрямую
     * @tparam Interface Тип интерфейса
     * @param instance Готовый экземпляр
     */
    template <typename Interface>
    void registerInstance(std::shared_ptr<Interface> instance) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));
        singletons_[typeIndex] = std::static_pointer_cast<void>(instance);
    }

    /**
     * @brief Получить экземпляр типа
     * @tparam Interface Тип интерфейса
     * @return Умный указатель на объект
     * @throws std::runtime_error если тип не зарегистрирован
     */
    template <typename Interface>
    std::shared_ptr<Interface> resolve() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));

        // Проверить, есть ли прямой экземпляр
        if (auto it = singletons_.find(typeIndex); it != singletons_.end()) {
            return std::static_pointer_cast<Interface>(it->second);
        }

        // Найти фабрику
        auto factoryIt = factories_.find(typeIndex);
        if (factoryIt == factories_.end()) {
            throw std::runtime_error("Тип " + std::string(typeid(Interface).name())
                                     + " не зарегистрирован в контейнере");
        }

        auto& factory = factoryIt->second;
        auto lifetime = factory->getLifetime();

        switch (lifetime) {
            case Lifetime::Singleton: {
                // Проверить, есть ли уже созданный singleton
                if (auto it = singletons_.find(typeIndex); it != singletons_.end()) {
                    return std::static_pointer_cast<Interface>(it->second);
                }

                // Создать новый singleton
                auto instance = factory->create(*this);
                singletons_[typeIndex] = instance;
                return std::static_pointer_cast<Interface>(instance);
            }

            case Lifetime::Transient: {
                // Всегда создавать новый экземпляр
                auto instance = factory->create(*this);
                return std::static_pointer_cast<Interface>(instance);
            }

            case Lifetime::Scoped: {
                // Проверить scoped экземпляр
                if (auto it = scopedInstances_.find(typeIndex); it != scopedInstances_.end()) {
                    return std::static_pointer_cast<Interface>(it->second);
                }

                // Создать новый scoped экземпляр
                auto instance = factory->create(*this);
                scopedInstances_[typeIndex] = instance;
                return std::static_pointer_cast<Interface>(instance);
            }
        }

        throw std::runtime_error("Неизвестный lifetime для типа "
                                 + std::string(typeid(Interface).name()));
    }

    /**
     * @brief Проверить, зарегистрирован ли тип
     * @tparam Interface Тип для проверки
     * @return true если тип зарегистрирован
     */
    template <typename Interface>
    bool isRegistered() const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeIndex = std::type_index(typeid(Interface));
        return factories_.find(typeIndex) != factories_.end()
               || singletons_.find(typeIndex) != singletons_.end();
    }

    /**
     * @brief Создать новый scope
     * @param scopeId Идентификатор scope
     */
    void beginScope(const std::string& scopeId = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        scopeId_ = scopeId;
        scopedInstances_.clear();
    }

    /**
     * @brief Завершить текущий scope
     */
    void endScope() {
        std::lock_guard<std::mutex> lock(mutex_);
        scopedInstances_.clear();
        scopeId_.clear();
    }

    /**
     * @brief Получить текущий scope ID
     * @return Идентификатор текущего scope
     */
    std::string getCurrentScopeId() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return scopeId_;
    }

    /**
     * @brief Очистить все регистрации
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_.clear();
        singletons_.clear();
        scopedInstances_.clear();
        scopeId_.clear();
    }

    /**
     * @brief Получить список зарегистрированных типов
     * @return Вектор имен типов
     */
    std::vector<std::string> getRegisteredTypes() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> types;

        for (const auto& [typeIndex, factory] : factories_) {
            types.push_back(typeIndex.name());
        }

        for (const auto& [typeIndex, instance] : singletons_) {
            if (factories_.find(typeIndex) == factories_.end()) {
                types.push_back(typeIndex.name());
            }
        }

        return types;
    }

    /**
     * @brief Получить статистику контейнера
     * @return Структура со статистикой
     */
    struct ContainerStats {
        size_t registeredFactories = 0;
        size_t activeSingletons = 0;
        size_t activeScopedInstances = 0;
        std::string currentScope;
    };

    ContainerStats getStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {factories_.size(), singletons_.size(), scopedInstances_.size(), scopeId_};
    }
};

/**
 * @brief RAII scope guard для автоматического управления scope
 */
class ScopeGuard {
  private:
    Container& container_;

  public:
    explicit ScopeGuard(Container& container, const std::string& scopeId = "")
        : container_(container) {
        container_.beginScope(scopeId);
    }

    ~ScopeGuard() { container_.endScope(); }

    // Запретить копирование и перемещение
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;
};

/**
 * @brief Глобальный контейнер для удобства использования
 *
 * Предоставляет простой доступ к DI контейнеру без необходимости
 * передавать его по всему приложению.
 */
class ServiceLocator {
  private:
    static std::unique_ptr<Container> instance_;
    static std::mutex mutex_;

  public:
    /**
     * @brief Получить глобальный контейнер
     * @return Ссылка на контейнер
     */
    static Container& getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_ = std::make_unique<Container>();
        }
        return *instance_;
    }

    /**
     * @brief Установить пользовательский контейнер
     * @param container Новый контейнер
     */
    static void setInstance(std::unique_ptr<Container> container) {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_ = std::move(container);
    }

    /**
     * @brief Очистить глобальный контейнер
     */
    static void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_.reset();
    }

    /**
     * @brief Упрощенный доступ к resolve
     * @tparam T Тип для получения
     * @return Умный указатель на объект
     */
    template <typename T>
    static std::shared_ptr<T> get() {
        return getInstance().resolve<T>();
    }
};

// Статические члены определены в Container.cpp

}  // namespace HyperEngine::Core::DI
