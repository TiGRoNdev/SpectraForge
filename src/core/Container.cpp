#include "HyperEngine/Core/DependencyInjection/Container.h"

namespace HyperEngine::Core::DI {

// Определения статических членов ServiceLocator
std::unique_ptr<Container> ServiceLocator::instance_ = nullptr;
std::mutex ServiceLocator::mutex_;

}  // namespace HyperEngine::Core::DI
