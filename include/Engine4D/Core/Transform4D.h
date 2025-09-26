#pragma once

#include "Component.h"
#include "Interfaces.h"
#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include "../Math/Quaternion4D.h"
#include <vector>

namespace Engine4D {
namespace Core {

/**
 * @brief Transform component for 4D objects following SRP
 */
class Transform4D : public Component4D, public ITransformable {
public:
    Transform4D();
    virtual ~Transform4D();

    // Component interface
    std::string getComponentType() const override { return "Transform4D"; }
    void cleanup() override;

    // ITransformable interface
    Math::Vector4 getPosition() const override { return position; }
    void setPosition(const Math::Vector4& pos) override;
    Math::Matrix4 getTransformMatrix() const override { return getWorldMatrix(); }

    // Local transform operations
    Math::Matrix4 getLocalMatrix() const;
    Math::Matrix4 getWorldMatrix() const;
    
    // Local getters
    const Math::Quaternion4D& getRotation() const { return rotation; }
    const Math::Vector4& getScale() const { return scale; }
    
    // Local setters
    void setRotation(const Math::Quaternion4D& rot);
    void setScale(const Math::Vector4& scl);
    
    // Transform operations
    void translate(const Math::Vector4& translation);
    void rotate(const Math::Quaternion4D& rotation);
    void scaleBy(const Math::Vector4& scale);
    
    // World coordinate getters
    Math::Vector4 getWorldPosition() const;
    Math::Quaternion4D getWorldRotation() const;
    Math::Vector4 getWorldScale() const;
    
    // Direction vectors
    Math::Vector4 forward() const;
    Math::Vector4 right() const;
    Math::Vector4 up() const;
    Math::Vector4 wAxis() const;
    
    // Hierarchy management
    void setParent(Transform4D* parent);
    Transform4D* getParent() const { return parent; }
    void addChild(Transform4D* child);
    void removeChild(Transform4D* child);
    const std::vector<Transform4D*>& getChildren() const { return children; }

private:
    // Transform data
    Math::Vector4 position;
    Math::Quaternion4D rotation;
    Math::Vector4 scale;
    
    // Hierarchy
    Transform4D* parent;
    std::vector<Transform4D*> children;
    
    // Cache for performance
    mutable Math::Matrix4 cachedWorldMatrix;
    mutable bool worldMatrixDirty;
    
    // Helper methods
    void markWorldMatrixDirty();
    void updateWorldMatrix() const;
};

} // namespace Core
} // namespace Engine4D
