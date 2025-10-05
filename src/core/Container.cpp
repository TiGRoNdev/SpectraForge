#include "SpectraForge/Core/DependencyInjection/Container.h"

namespace SpectraForge::Core::DI {

// Определения статических членов ServiceLocator
std::unique_ptr<Container> ServiceLocator::instance_ = nullptr;
std::mutex ServiceLocator::mutex_;

}  // namespace SpectraForge::Core::DI
