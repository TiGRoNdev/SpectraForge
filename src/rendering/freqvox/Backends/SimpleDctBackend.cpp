/**
 * @file SimpleDctBackend.cpp
 */

#include "HyperEngine/Rendering/FreqVox/Backends/SimpleDctBackend.h"

namespace HyperEngine::Rendering::FreqVox::Backends {

bool SimpleDctBackend::initialize(const DctBlockConfig& config) {
    cfg_ = config;
    initialized_ = true;
    return true;
}

void SimpleDctBackend::shutdown() { initialized_ = false; }

bool SimpleDctBackend::transform_forward(std::vector<float>& io_block_batched) {
    (void)io_block_batched;
    return initialized_;
}

bool SimpleDctBackend::transform_inverse(std::vector<float>& io_block_batched) {
    (void)io_block_batched;
    return initialized_;
}

}  // namespace HyperEngine::Rendering::FreqVox::Backends


