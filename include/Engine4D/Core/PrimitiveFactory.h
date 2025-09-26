#pragma once

#include "Interfaces.h"
#include <memory>
#include <unordered_map>
#include <functional>

namespace Engine4D {
namespace Core {

class GameObject4D;

/**
 * @brief Factory for creating primitive game objects following OCP
 */
class PrimitiveFactory : public IPrimitiveFactory {
public:
    using PrimitiveCreator = std::function<std::shared_ptr<GameObject4D>()>;

    PrimitiveFactory();
    virtual ~PrimitiveFactory() = default;

    // IPrimitiveFactory implementation
    std::shared_ptr<GameObject4D> createPrimitive(const std::string& type) override;

    // Factory management
    void registerPrimitive(const std::string& type, PrimitiveCreator creator);
    void unregisterPrimitive(const std::string& type);
    bool isPrimitiveRegistered(const std::string& type) const;

    // Static convenience method
    static PrimitiveFactory& getInstance();

private:
    std::unordered_map<std::string, PrimitiveCreator> primitiveCreators;

    // Default primitive creators
    void registerDefaultPrimitives();
    std::shared_ptr<GameObject4D> createTesseract();
    std::shared_ptr<GameObject4D> createSphere();
    std::shared_ptr<GameObject4D> createCube();
    std::shared_ptr<GameObject4D> createHypercube();
};

} // namespace Core
} // namespace Engine4D
