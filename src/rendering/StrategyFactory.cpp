/**
 * @file StrategyFactory.cpp
 */

#include "HyperEngine/Rendering/StrategyFactory.h"
#include "HyperEngine/Rendering/FreqVox/FreqVoxStrategy.h"

namespace HyperEngine::Rendering {

std::shared_ptr<IRenderStrategy> create_strategy_by_name(const std::string& name) {
    if (name == "freqvox") {
    #ifdef HyperEngine_ENABLE_FREQVOX
        return std::make_shared<FreqVox::FreqVoxStrategy>();
    #else
        return nullptr;
    #endif
    }
    return nullptr;
}

}  // namespace HyperEngine::Rendering


