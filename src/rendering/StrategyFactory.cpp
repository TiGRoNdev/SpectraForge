/**
 * @file StrategyFactory.cpp
 */

#include "SpectraForge/Rendering/StrategyFactory.h"
#include "SpectraForge/Rendering/RenderStrategy/FreGSRenderStrategy.h"
#ifdef HyperEngine_ENABLE_FREQVOX
#include "SpectraForge/Rendering/FreqVox/FreqVoxStrategy.h"
#endif

namespace SpectraForge::Rendering {

std::shared_ptr<IRenderStrategy> create_strategy_by_name(const std::string& name) {
    if (name == "freqs" || name == "fregs" || name == "gaussian") {
        return std::make_shared<FreGSRenderStrategy>();
    }
    if (name == "freqvox") {
    #ifdef HyperEngine_ENABLE_FREQVOX
        return std::make_shared<FreqVox::FreqVoxStrategy>();
    #else
        return nullptr;
    #endif
    }
    return nullptr;
}

}  // namespace SpectraForge::Rendering


