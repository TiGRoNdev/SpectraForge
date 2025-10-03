#include <gtest/gtest.h>
#include <shaderc/shaderc.hpp>
#include <string>
#include <cstring>
#include <vector>

TEST(ShaderRuntimeCompileTest, CompileSimpleComputeShader_Succeeds) {
    const char* src = R"(
        #version 450
        layout(local_size_x=1, local_size_y=1) in;
        void main() {}
    )";

    shaderc::Compiler compiler;
    shaderc::CompileOptions opts;
    opts.SetSourceLanguage(shaderc_source_language_glsl);
    opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    opts.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto result = compiler.CompileGlslToSpv(src, std::strlen(src), shaderc_compute_shader, "simple.comp", opts);
    ASSERT_EQ(result.GetCompilationStatus(), shaderc_compilation_status_success) << result.GetErrorMessage();

    std::vector<uint32_t> spirv(result.cbegin(), result.cend());
    ASSERT_FALSE(spirv.empty());
}
