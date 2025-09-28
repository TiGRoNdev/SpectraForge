/**
 * @file OptiXRayTracer.cpp
 * @brief Реализация OptiX Ray Tracer для вторичных эффектов
 * 
 * Этот модуль реализует ray tracing для отражений, теней и глобального освещения
 * с использованием NVIDIA OptiX 7.x и поддержкой Shader Execution Reordering.
 */

#include "HyperEngine/OptiX/OptiXRayTracer.h"
#include <iostream>
#include <fstream>
#include <cuda_runtime.h>
#include <algorithm>
#include <cstring>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT

using namespace HyperEngine::OptiX;
using namespace HyperEngine::Core;

namespace HyperEngine::OptiX {

// PTX код для ray generation shader (будет загружаться из файла)
extern "C" const char embedded_ptx_code[];

// Реализация ShaderBindingTable
ShaderBindingTable::ShaderBindingTable() {
    SAFE_PRINT_LINE("[ShaderBindingTable] Конструктор");
}

ShaderBindingTable::~ShaderBindingTable() {
    if (initialized) {
        shutdown();
    }
}

bool ShaderBindingTable::create(OptixPipeline pipeline) {
    SAFE_PRINT_LINE("[ShaderBindingTable] Создание SBT...");
    
    try {
        // Размеры записей SBT
        const size_t raygenRecordSize = sizeof(void*);
        const size_t missRecordSize = sizeof(void*);
        const size_t hitgroupRecordSize = sizeof(void*);
        
        // Выделяем память для записей
        CUDA_CHECK(cudaMalloc(&raygenRecord, raygenRecordSize));
        CUDA_CHECK(cudaMalloc(&missRecords, missRecordSize));
        CUDA_CHECK(cudaMalloc(&hitgroupRecords, hitgroupRecordSize));
        
        // Настраиваем SBT
        sbt.raygenRecord = raygenRecord;
        sbt.missRecordBase = missRecords;
        sbt.missRecordStrideInBytes = missRecordSize;
        sbt.missRecordCount = 1;
        sbt.hitgroupRecordBase = hitgroupRecords;
        sbt.hitgroupRecordStrideInBytes = hitgroupRecordSize;
        sbt.hitgroupRecordCount = 1;
        
        initialized = true;
        SAFE_PRINT_LINE("[ShaderBindingTable] SBT создан успешно");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ShaderBindingTable] Ошибка создания SBT: " << e.what() << std::endl;
        return false;
    }
}

void ShaderBindingTable::shutdown() {
    if (!initialized) {
        return;
    }
    
    SAFE_PRINT_LINE("[ShaderBindingTable] Освобождение SBT...");
    
    if (raygenRecord) {
        cudaFree(reinterpret_cast<void*>(raygenRecord));
        raygenRecord = 0;
    }
    if (missRecords) {
        cudaFree(reinterpret_cast<void*>(missRecords));
        missRecords = 0;
    }
    if (hitgroupRecords) {
        cudaFree(reinterpret_cast<void*>(hitgroupRecords));
        hitgroupRecords = 0;
    }
    
    initialized = false;
    SAFE_PRINT_LINE("[ShaderBindingTable] SBT освобожден");
}

// Реализация AccelerationStructure
AccelerationStructure::AccelerationStructure() {
    SAFE_PRINT_LINE("[AccelerationStructure] Конструктор");
}

AccelerationStructure::~AccelerationStructure() {
    if (initialized) {
        shutdown();
    }
}

bool AccelerationStructure::init(OptixDeviceContext ctx) {
    context = ctx;
    SAFE_PRINT_LINE("[AccelerationStructure] Инициализация AS...");
    
    initialized = true;
    return true;
}

bool AccelerationStructure::build(const SceneGeometry& geometry) {
    if (!initialized) {
        SAFE_ERROR("[AccelerationStructure] Ошибка: AS не инициализирован");
        return false;
    }
    
    SAFE_PRINT_LINE("[AccelerationStructure] Построение GAS...");
    std::cout << "  Вершин: " << geometry.vertexCount << ", Треугольников: " << geometry.triangleCount << std::endl;
    
    try {
        // Настройка входных данных для GAS
        OptixBuildInput buildInput = {};
        buildInput.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
        
        // Настройка массива вершин
        buildInput.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
        buildInput.triangleArray.vertexStrideInBytes = geometry.vertexStride;
        buildInput.triangleArray.numVertices = geometry.vertexCount;
        
        // Настройка массива индексов
        buildInput.triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
        buildInput.triangleArray.indexStrideInBytes = 3 * sizeof(uint32_t);
        buildInput.triangleArray.numIndexTriplets = geometry.triangleCount;
        
        // Копируем геометрию на GPU
        CUdeviceptr d_vertices, d_indices;
        size_t verticesSize = geometry.vertexCount * geometry.vertexStride;
        size_t indicesSize = geometry.triangleCount * 3 * sizeof(uint32_t);
        
        CUDA_CHECK(cudaMalloc(&d_vertices, verticesSize));
        CUDA_CHECK(cudaMalloc(&d_indices, indicesSize));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertices), geometry.vertices, verticesSize, cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_indices), geometry.indices, indicesSize, cudaMemcpyHostToDevice));
        
        buildInput.triangleArray.vertexBuffers = &d_vertices;
        buildInput.triangleArray.indexBuffer = d_indices;
        
        // Флаги построения
        uint32_t triangleInputFlags[1] = { OPTIX_GEOMETRY_FLAG_NONE };
        buildInput.triangleArray.flags = triangleInputFlags;
        buildInput.triangleArray.numSbtRecords = 1;
        
        // Опции построения
        OptixAccelBuildOptions accelOptions = {};
        accelOptions.buildFlags = OPTIX_BUILD_FLAG_PREFER_FAST_TRACE;
        accelOptions.operation = OPTIX_BUILD_OPERATION_BUILD;
        
        // Получаем размеры буферов
        OptixAccelBufferSizes gasBufferSizes;
        OPTIX_CHECK(optixAccelComputeMemoryUsage(context, &accelOptions, &buildInput, 1, &gasBufferSizes));
        
        // Выделяем буферы
        CUdeviceptr d_tempBuffer;
        CUDA_CHECK(cudaMalloc(&d_tempBuffer, gasBufferSizes.tempSizeInBytes));
        CUDA_CHECK(cudaMalloc(&gasBuffer, gasBufferSizes.outputSizeInBytes));
        gasBufferSize = gasBufferSizes.outputSizeInBytes;
        
        // Строим GAS
        OPTIX_CHECK(optixAccelBuild(
            context,
            0, // CUDA stream
            &accelOptions,
            &buildInput,
            1, // количество build inputs
            d_tempBuffer,
            gasBufferSizes.tempSizeInBytes,
            gasBuffer,
            gasBufferSizes.outputSizeInBytes,
            &traversableHandle,
            nullptr, // properties
            0 // количество properties
        ));
        
        // Освобождаем временные буферы
        CUDA_CHECK(cudaFree(d_tempBuffer));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_vertices)));
        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_indices)));
        
        SAFE_PRINT_LINE("[AccelerationStructure] GAS построен успешно");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[AccelerationStructure] Ошибка построения GAS: " << e.what() << std::endl;
        return false;
    }
}

void AccelerationStructure::shutdown() {
    if (!initialized) {
        return;
    }
    
    SAFE_PRINT_LINE("[AccelerationStructure] Освобождение AS...");
    
    if (gasBuffer) {
        cudaFree(reinterpret_cast<void*>(gasBuffer));
        gasBuffer = 0;
    }
    
    traversableHandle = 0;
    initialized = false;
    SAFE_PRINT_LINE("[AccelerationStructure] AS освобожден");
}

OptiXRayTracer::OptiXRayTracer() {
    SAFE_PRINT_LINE("[OptiXRayTracer] Конструктор");
}

OptiXRayTracer::~OptiXRayTracer() {
    if (initialized) {
        shutdown();
    }
}

bool OptiXRayTracer::init(CUcontext cudaContext) {
    try {
        SAFE_PRINT_LINE("[OptiXRayTracer] Инициализация OptiX ray tracer...");
        
        // Инициализация OptiX
        OPTIX_CHECK(optixInit());
        
        // Настройка лог коллбека
        OptixDeviceContextOptions options = {};
        options.logCallbackFunction = &OptiXRayTracer::logCallback;
        options.logCallbackLevel = 4; // INFO уровень
        
        // Создание OptiX device context
        OPTIX_CHECK(optixDeviceContextCreate(cudaContext, &options, &context));
        
        SAFE_PRINT_LINE("[OptiXRayTracer] OptiX device context создан");
        
        // Создание модуля из PTX
        if (!createModule()) {
            SAFE_ERROR("[OptiXRayTracer] Ошибка создания OptiX модуля");
            return false;
        }
        
        // Создание pipeline
        if (!createPipeline()) {
            SAFE_ERROR("[OptiXRayTracer] Ошибка создания OptiX pipeline");
            return false;
        }
        
        // Создание Shader Binding Table
        sbt = std::make_unique<ShaderBindingTable>();
        if (!sbt->create(pipeline)) {
            SAFE_ERROR("[OptiXRayTracer] Ошибка создания SBT");
            return false;
        }
        
        // Создание Acceleration Structure
        as = std::make_unique<AccelerationStructure>();
        if (!as->init(context)) {
            SAFE_ERROR("[OptiXRayTracer] Ошибка инициализации AS");
            return false;
        }
        
        // Выделение буферов
        if (!allocateBuffers()) {
            SAFE_ERROR("[OptiXRayTracer] Ошибка выделения буферов");
            return false;
        }
        
        initialized = true;
        SAFE_PRINT_LINE("[OptiXRayTracer] Инициализация завершена успешно");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[OptiXRayTracer] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

void OptiXRayTracer::shutdown() {
    if (!initialized) {
        return;
    }
    
    SAFE_PRINT_LINE("[OptiXRayTracer] Завершение работы...");
    
    // Освобождаем буферы
    freeBuffers();
    
    // Очищаем ресурсы OptiX
    as.reset();
    sbt.reset();
    
    if (pipeline) {
        optixPipelineDestroy(pipeline);
        pipeline = nullptr;
    }
    
    if (module) {
        optixModuleDestroy(module);
        module = nullptr;
    }
    
    if (context) {
        optixDeviceContextDestroy(context);
        context = nullptr;
    }
    
    initialized = false;
    SAFE_PRINT_LINE("[OptiXRayTracer] Завершение работы завершено");
}

void OptiXRayTracer::buildAccelerationStructures(const SceneGeometry& geo) {
    if (!initialized || !as) {
        SAFE_ERROR("[OptiXRayTracer] Ошибка: Ray tracer не инициализирован");
        return;
    }
    
    SAFE_PRINT_LINE("[OptiXRayTracer] Построение acceleration structures...");
    std::cout << "  Вершин: " << geo.vertexCount << ", Треугольников: " << geo.triangleCount << std::endl;
    
    as->build(geo);
    
    SAFE_PRINT_LINE("[OptiXRayTracer] Acceleration structures построены");
}

RawEffects OptiXRayTracer::traceRays(const LaunchParams& params) {
    if (!initialized) {
        SAFE_ERROR("[OptiXRayTracer] Ошибка: Ray tracer не инициализирован");
        return RawEffects{};
    }
    
    std::cout << "[OptiXRayTracer] Трассировка лучей " << params.width << "x" << params.height << std::endl;
    
    // Обновляем размеры если необходимо
    if (params.width != imageWidth || params.height != imageHeight) {
        imageWidth = params.width;
        imageHeight = params.height;
        
        // Переаллоцируем буферы с новым размером
        freeBuffers();
        allocateBuffers();
    }
    
    // Копируем параметры на GPU
    LaunchParams* d_params;
    size_t paramsSize = sizeof(LaunchParams);
    CUDA_CHECK(cudaMalloc(&d_params, paramsSize));
    CUDA_CHECK(cudaMemcpy(d_params, &params, paramsSize, cudaMemcpyHostToDevice));
    
    // Запуск OptiX kernel
    OPTIX_CHECK(optixLaunch(
        pipeline,
        0, // CUDA stream
        reinterpret_cast<CUdeviceptr>(d_params),
        paramsSize,
        &sbt->getSBT(),
        params.width,
        params.height,
        1 // depth
    ));
    
    // Синхронизация
    CUDA_CHECK(cudaDeviceSynchronize());
    
    // Освобождаем временные ресурсы
    CUDA_CHECK(cudaFree(d_params));
    
    // Подготавливаем результат
    RawEffects result = {};
    result.reflections = reinterpret_cast<float*>(d_reflections);
    result.shadows = reinterpret_cast<float*>(d_shadows);
    result.globalIllumination = reinterpret_cast<float*>(d_globalIllumination);
    result.motionVectors = reinterpret_cast<float*>(d_motionVectors);
    result.albedo = reinterpret_cast<float*>(d_albedo);
    result.normals = reinterpret_cast<float*>(d_normals);
    result.width = params.width;
    result.height = params.height;
    result.stream = 0; // Default stream
    
    SAFE_PRINT_LINE("[OptiXRayTracer] Трассировка лучей завершена");
    return result;
}

void OptiXRayTracer::applySER(const CoherencyHints& hints) {
    SAFE_PRINT_LINE("[OptiXRayTracer] Применение Shader Execution Reordering");
    std::cout << "  Когерентность лучей: " << hints.rayCoherence << std::endl;
    std::cout << "  Когерентность материалов: " << hints.materialCoherence << std::endl;
    std::cout << "  Когерентность геометрии: " << hints.geometryCoherence << std::endl;
    
    // SER реализуется на уровне шейдеров, здесь мы можем настроить параметры
    // В реальной реализации здесь будет настройка SER hints для OptiX
    
    SAFE_PRINT_LINE("[OptiXRayTracer] SER настроен");
}

// Приватные методы

bool OptiXRayTracer::createModule() {
    SAFE_PRINT_LINE("[OptiXRayTracer] Создание OptiX модуля...");
    
    // В реальной реализации PTX код будет загружаться из файла
    // Для демонстрации используем простую заглушку
    const char* ptx_code = R"(
        .version 7.0
        .target sm_75
        .address_size 64
        
        .entry __raygen__rg() {
            // Простой ray generation shader
            ret;
        }
        
        .entry __miss__ms() {
            // Miss shader
            ret;
        }
        
        .entry __closesthit__ch() {
            // Closest hit shader
            ret;
        }
    )";
    
    OptixModuleCompileOptions moduleCompileOptions = {};
    moduleCompileOptions.maxRegisterCount = 50;
    moduleCompileOptions.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
    moduleCompileOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_LINEINFO;
    
    OptixPipelineCompileOptions pipelineCompileOptions = {};
    pipelineCompileOptions.usesMotionBlur = false;
    pipelineCompileOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pipelineCompileOptions.numPayloadValues = 3;
    pipelineCompileOptions.numAttributeValues = 3;
    pipelineCompileOptions.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipelineCompileOptions.pipelineLaunchParamsVariableName = "params";
    
    char log[2048];
    size_t sizeof_log = sizeof(log);
    
    OPTIX_CHECK_LOG(optixModuleCreateFromPTX(
        context,
        &moduleCompileOptions,
        &pipelineCompileOptions,
        ptx_code,
        strlen(ptx_code),
        log,
        &sizeof_log,
        &module
    ));
    
    SAFE_PRINT_LINE("[OptiXRayTracer] OptiX модуль создан");
    if (sizeof_log > 1) {
        std::cout << "[OptiXRayTracer] Лог компиляции: " << log << std::endl;
    }
    
    return true;
}

bool OptiXRayTracer::createPipeline() {
    SAFE_PRINT_LINE("[OptiXRayTracer] Создание OptiX pipeline...");
    
    // Настройка program groups
    OptixProgramGroupOptions programGroupOptions = {};
    
    // Ray generation program
    OptixProgramGroupDesc raygenPG = {};
    raygenPG.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
    raygenPG.raygen.module = module;
    raygenPG.raygen.entryFunctionName = "__raygen__rg";
    
    // Miss program
    OptixProgramGroupDesc missPG = {};
    missPG.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
    missPG.miss.module = module;
    missPG.miss.entryFunctionName = "__miss__ms";
    
    // Hit group program
    OptixProgramGroupDesc hitgroupPG = {};
    hitgroupPG.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
    hitgroupPG.hitgroup.moduleCH = module;
    hitgroupPG.hitgroup.entryFunctionNameCH = "__closesthit__ch";
    
    OptixProgramGroupDesc programGroups[] = { raygenPG, missPG, hitgroupPG };
    
    char log[2048];
    size_t sizeof_log = sizeof(log);
    
    OptixProgramGroup programGroupsArray[3];
    OPTIX_CHECK_LOG(optixProgramGroupCreate(
        context,
        programGroups,
        3, // количество program groups
        &programGroupOptions,
        log,
        &sizeof_log,
        programGroupsArray
    ));
    
    // Создание pipeline
    OptixPipelineCompileOptions pipelineCompileOptions = {};
    pipelineCompileOptions.usesMotionBlur = false;
    pipelineCompileOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pipelineCompileOptions.numPayloadValues = 3;
    pipelineCompileOptions.numAttributeValues = 3;
    pipelineCompileOptions.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipelineCompileOptions.pipelineLaunchParamsVariableName = "params";
    
    OptixPipelineLinkOptions pipelineLinkOptions = {};
    pipelineLinkOptions.maxTraceDepth = maxTraceDepth;
    
    sizeof_log = sizeof(log);
    OPTIX_CHECK_LOG(optixPipelineCreate(
        context,
        &pipelineCompileOptions,
        &pipelineLinkOptions,
        programGroupsArray,
        3, // количество program groups
        log,
        &sizeof_log,
        &pipeline
    ));
    
    SAFE_PRINT_LINE("[OptiXRayTracer] OptiX pipeline создан");
    if (sizeof_log > 1) {
        std::cout << "[OptiXRayTracer] Лог линковки: " << log << std::endl;
    }
    
    return true;
}

bool OptiXRayTracer::allocateBuffers() {
    SAFE_PRINT_LINE("[OptiXRayTracer] Выделение CUDA буферов...");
    
    size_t pixelCount = imageWidth * imageHeight;
    size_t bufferSize = pixelCount * 4 * sizeof(float); // RGBA
    
    // Выделяем буферы для различных эффектов
    CUDA_CHECK(cudaMalloc(&d_reflections, bufferSize));
    CUDA_CHECK(cudaMalloc(&d_shadows, bufferSize));
    CUDA_CHECK(cudaMalloc(&d_globalIllumination, bufferSize));
    CUDA_CHECK(cudaMalloc(&d_motionVectors, bufferSize));
    CUDA_CHECK(cudaMalloc(&d_albedo, bufferSize));
    CUDA_CHECK(cudaMalloc(&d_normals, bufferSize));
    
    // Инициализируем буферы нулями
    CUDA_CHECK(cudaMemset(d_reflections, 0, bufferSize));
    CUDA_CHECK(cudaMemset(d_shadows, 0, bufferSize));
    CUDA_CHECK(cudaMemset(d_globalIllumination, 0, bufferSize));
    CUDA_CHECK(cudaMemset(d_motionVectors, 0, bufferSize));
    CUDA_CHECK(cudaMemset(d_albedo, 0, bufferSize));
    CUDA_CHECK(cudaMemset(d_normals, 0, bufferSize));
    
    std::cout << "[OptiXRayTracer] Буферы выделены для " << imageWidth << "x" << imageHeight << std::endl;
    return true;
}

void OptiXRayTracer::freeBuffers() {
    SAFE_PRINT_LINE("[OptiXRayTracer] Освобождение CUDA буферов...");
    
    if (d_reflections) { cudaFree(reinterpret_cast<void*>(d_reflections)); d_reflections = 0; }
    if (d_shadows) { cudaFree(reinterpret_cast<void*>(d_shadows)); d_shadows = 0; }
    if (d_globalIllumination) { cudaFree(reinterpret_cast<void*>(d_globalIllumination)); d_globalIllumination = 0; }
    if (d_motionVectors) { cudaFree(reinterpret_cast<void*>(d_motionVectors)); d_motionVectors = 0; }
    if (d_albedo) { cudaFree(reinterpret_cast<void*>(d_albedo)); d_albedo = 0; }
    if (d_normals) { cudaFree(reinterpret_cast<void*>(d_normals)); d_normals = 0; }
    
    SAFE_PRINT_LINE("[OptiXRayTracer] Буферы освобождены");
}

void OptiXRayTracer::logCallback(unsigned int level, const char* tag, const char* message, void* cbdata) {
    std::cout << "[OptiX][" << tag << "][" << level << "] " << message << std::endl;
}

} // namespace HyperEngine::OptiX

#endif // VULKAN_RENDERER_OPTIX_SUPPORT

