// CudaInterop.cpp - CUDA-Vulkan interop
// Временная заглушка для этапа 1.2 настройки SDK

#include <iostream>

namespace Engine3D {
namespace CUDA {

class CudaInterop {
public:
    bool initializeInterop() {
        std::cout << "CudaInterop::initializeInterop() - заглушка" << std::endl;
        return true;
    }
    
    void cleanup() {
        std::cout << "CudaInterop::cleanup() - заглушка" << std::endl;
    }
};

} // namespace CUDA
} // namespace Engine3D

// Экспортируемая функция для тестирования
extern "C" {
    void cuda_interop_test() {
        Engine3D::CUDA::CudaInterop interop;
        interop.initializeInterop();
        interop.cleanup();
    }
}
