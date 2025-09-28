/**
 * @file IConfigurable.h
 * @brief Интерфейс для конфигурируемых компонентов
 * 
 * Следует принципу ISP - разделяет обязанности конфигурации
 */

#pragma once

#include <string>
#include <unordered_map>
#include <any>

namespace HyperEngine {
namespace Core {
namespace Interfaces {

/**
 * @brief Интерфейс для компонентов, которые могут быть сконфигурированы
 * 
 * Применяет принцип ISP - клиенты зависят только от методов конфигурации
 */
class IConfigurable {
public:
    virtual ~IConfigurable() = default;

    /**
     * @brief Установка параметра конфигурации
     * @param key Ключ параметра
     * @param value Значение параметра
     */
    virtual void setConfigParameter(const std::string& key, const std::any& value) = 0;

    /**
     * @brief Получение параметра конфигурации
     * @param key Ключ параметра
     * @return Значение параметра
     */
    virtual std::any getConfigParameter(const std::string& key) const = 0;

    /**
     * @brief Проверка наличия параметра
     * @param key Ключ параметра
     * @return true если параметр существует
     */
    virtual bool hasConfigParameter(const std::string& key) const = 0;

    /**
     * @brief Загрузка конфигурации из файла
     * @param configPath Путь к файлу конфигурации
     * @return true если загрузка успешна
     */
    virtual bool loadConfig(const std::string& configPath) = 0;

    /**
     * @brief Сохранение конфигурации в файл
     * @param configPath Путь к файлу конфигурации
     * @return true если сохранение успешно
     */
    virtual bool saveConfig(const std::string& configPath) const = 0;
};

}  // namespace Interfaces
}  // namespace Core
}  // namespace HyperEngine
