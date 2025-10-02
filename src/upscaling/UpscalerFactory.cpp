/**
 * @file UpscalerFactory.cpp
 * @brief Implementation of upscaler factory
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/upscaling/UpscalerFactory.h"
#include "SpectraForge/upscaling/NativeUpscaler.h"
// #include "SpectraForge/upscaling/DLSSUpscaler.h"      // Phase 4 TODO
// #include "SpectraForge/upscaling/FSR2Upscaler.h"      // Phase 4 TODO
#include <iostream>

namespace spectraforge {
namespace upscaling {

std::unique_ptr<IUpscaler> UpscalerFactory::create(
    const UpscalerConfig& config,
    uint32_t gpuVendorId)
{
    UpscalerType type = config.type;
    
    // Auto-detect upscaler based on GPU vendor
    if (type == UpscalerType::AUTO) {
        type = getRecommended(gpuVendorId);
    }
    
    // Check availability
    if (!isAvailable(type, gpuVendorId)) {
        std::cerr << "Upscaler type " << getTypeName(type) 
                  << " not available, falling back to native\n";
        type = UpscalerType::NONE;
    }
    
    // Create upscaler
    switch (type) {
        case UpscalerType::DLSS:
            #ifdef SPECTRAFORGE_DLSS_AVAILABLE
            std::cout << "Creating DLSS upscaler\n";
            return std::make_unique<DLSSUpscaler>(config);
            #else
            std::cerr << "DLSS not compiled in, falling back to FSR2\n";
            type = UpscalerType::FSR2;
            // Fall through to FSR2
            #endif
            
        case UpscalerType::FSR2:
            #ifdef SPECTRAFORGE_FSR2_AVAILABLE
            std::cout << "Creating FSR2 upscaler\n";
            return std::make_unique<FSR2Upscaler>(config);
            #else
            std::cerr << "FSR2 not compiled in, falling back to native\n";
            // Fall through to NONE
            #endif
            
        case UpscalerType::NONE:
        default:
            std::cout << "Using native resolution (no upscaling)\n";
            return std::make_unique<NativeUpscaler>();
    }
}

bool UpscalerFactory::isAvailable(UpscalerType type, uint32_t gpuVendorId) {
    switch (type) {
        case UpscalerType::DLSS:
            // DLSS requires NVIDIA RTX GPU
            #ifdef SPECTRAFORGE_DLSS_AVAILABLE
            return isNVIDIA(gpuVendorId);
            #else
            return false;
            #endif
            
        case UpscalerType::FSR2:
            // FSR2 is vendor-agnostic
            #ifdef SPECTRAFORGE_FSR2_AVAILABLE
            return true;
            #else
            return false;
            #endif
            
        case UpscalerType::NONE:
        case UpscalerType::AUTO:
            return true;
    }
    
    return false;
}

UpscalerType UpscalerFactory::getRecommended(uint32_t gpuVendorId) {
    // NVIDIA → DLSS (if available)
    if (isNVIDIA(gpuVendorId)) {
        #ifdef SPECTRAFORGE_DLSS_AVAILABLE
        return UpscalerType::DLSS;
        #endif
    }
    
    // AMD → FSR2
    if (isAMD(gpuVendorId)) {
        #ifdef SPECTRAFORGE_FSR2_AVAILABLE
        return UpscalerType::FSR2;
        #endif
    }
    
    // Intel/ARM/Qualcomm → FSR2 (vendor-agnostic)
    if (isIntel(gpuVendorId) || isARM(gpuVendorId) || isQualcomm(gpuVendorId)) {
        #ifdef SPECTRAFORGE_FSR2_AVAILABLE
        return UpscalerType::FSR2;
        #endif
    }
    
    // Fallback: native (no upscaling)
    return UpscalerType::NONE;
}

const char* UpscalerFactory::getTypeName(UpscalerType type) {
    switch (type) {
        case UpscalerType::AUTO:    return "Auto";
        case UpscalerType::DLSS:    return "NVIDIA DLSS";
        case UpscalerType::FSR2:    return "AMD FSR2";
        case UpscalerType::NONE:    return "Native";
        default:                     return "Unknown";
    }
}

} // namespace upscaling
} // namespace spectraforge

