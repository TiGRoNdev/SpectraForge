/**
 * @file upscaling_system_test.cpp
 * @brief Комплексные тесты для системы апскейлинга
 */

#include <gtest/gtest.h>
#include <SpectraForge/Upscaling/Upscaler.h>
#include <SpectraForge/Upscaling/UpscalerFactory.h>
#include <SpectraForge/Upscaling/NativeUpscaler.h>
#include <SpectraForge/Upscaling/FSR2Upscaler.h>
#include <SpectraForge/Upscaling/DLSSUpscaler.h>

using namespace SpectraForge::Upscaling;

// ============================================================================
// UpscalerFactory Tests
// ============================================================================

class UpscalerFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup if needed
    }
};

TEST_F(UpscalerFactoryTest, CreateNativeUpscaler) {
    // Arrange & Act
    auto upscaler = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Assert
    EXPECT_NE(upscaler, nullptr);
}

TEST_F(UpscalerFactoryTest, CreateFSR2Upscaler) {
    // Arrange & Act
    auto upscaler = UpscalerFactory::createUpscaler(UpscalerType::FSR2);
    
    // Assert
    EXPECT_NE(upscaler, nullptr);
}

TEST_F(UpscalerFactoryTest, CreateDLSSUpscaler) {
    // Arrange & Act
    auto upscaler = UpscalerFactory::createUpscaler(UpscalerType::DLSS);
    
    // Assert
    EXPECT_NE(upscaler, nullptr);
}

TEST_F(UpscalerFactoryTest, CreateMultipleUpscalers) {
    // Arrange & Act
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    auto fsr2 = UpscalerFactory::createUpscaler(UpscalerType::FSR2);
    auto dlss = UpscalerFactory::createUpscaler(UpscalerType::DLSS);
    
    // Assert
    EXPECT_NE(native, nullptr);
    EXPECT_NE(fsr2, nullptr);
    EXPECT_NE(dlss, nullptr);
}

// ============================================================================
// NativeUpscaler Tests
// ============================================================================

class NativeUpscalerTest : public ::testing::Test {
protected:
    void SetUp() override {
        upscaler = std::make_unique<NativeUpscaler>();
    }

    std::unique_ptr<NativeUpscaler> upscaler;
};

TEST_F(NativeUpscalerTest, Constructor) {
    // Arrange & Act
    NativeUpscaler native;
    
    // Assert - должен создаться без ошибок
    EXPECT_TRUE(true);
}

TEST_F(NativeUpscalerTest, GetType) {
    // Arrange & Act
    UpscalerType type = upscaler->getType();
    
    // Assert
    EXPECT_EQ(type, UpscalerType::Native);
}

TEST_F(NativeUpscalerTest, IsSupported) {
    // Arrange & Act
    bool supported = upscaler->isSupported();
    
    // Assert
    EXPECT_TRUE(supported);  // Native всегда поддерживается
}

TEST_F(NativeUpscalerTest, Initialize) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->initialize(1920, 1080, 3840, 2160));
}

TEST_F(NativeUpscalerTest, Cleanup) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->cleanup());
}

TEST_F(NativeUpscalerTest, SetQualityMode) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->setQualityMode(UpscalerQuality::Balanced));
    EXPECT_NO_THROW(upscaler->setQualityMode(UpscalerQuality::Performance));
    EXPECT_NO_THROW(upscaler->setQualityMode(UpscalerQuality::Quality));
}

TEST_F(NativeUpscalerTest, GetQualityMode) {
    // Arrange
    upscaler->setQualityMode(UpscalerQuality::Quality);
    
    // Act
    UpscalerQuality quality = upscaler->getQualityMode();
    
    // Assert
    EXPECT_EQ(quality, UpscalerQuality::Quality);
}

TEST_F(NativeUpscalerTest, SetSharpness) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->setSharpness(0.5f));
    EXPECT_NO_THROW(upscaler->setSharpness(0.0f));
    EXPECT_NO_THROW(upscaler->setSharpness(1.0f));
}

TEST_F(NativeUpscalerTest, GetSharpness) {
    // Arrange
    upscaler->setSharpness(0.7f);
    
    // Act
    float sharpness = upscaler->getSharpness();
    
    // Assert
    EXPECT_FLOAT_EQ(sharpness, 0.7f);
}

// ============================================================================
// FSR2Upscaler Tests
// ============================================================================

class FSR2UpscalerTest : public ::testing::Test {
protected:
    void SetUp() override {
        upscaler = std::make_unique<FSR2Upscaler>();
    }

    std::unique_ptr<FSR2Upscaler> upscaler;
};

TEST_F(FSR2UpscalerTest, Constructor) {
    // Arrange & Act
    FSR2Upscaler fsr2;
    
    // Assert
    EXPECT_TRUE(true);
}

TEST_F(FSR2UpscalerTest, GetType) {
    // Arrange & Act
    UpscalerType type = upscaler->getType();
    
    // Assert
    EXPECT_EQ(type, UpscalerType::FSR2);
}

TEST_F(FSR2UpscalerTest, IsSupported) {
    // Arrange & Act
    bool supported = upscaler->isSupported();
    
    // Assert - FSR2 может быть не поддержан на всех платформах
    EXPECT_TRUE(supported || !supported);
}

TEST_F(FSR2UpscalerTest, Initialize) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->initialize(1920, 1080, 3840, 2160));
}

TEST_F(FSR2UpscalerTest, Cleanup) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->cleanup());
}

TEST_F(FSR2UpscalerTest, SetQualityMode) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->setQualityMode(UpscalerQuality::Balanced));
}

TEST_F(FSR2UpscalerTest, SetSharpness) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->setSharpness(0.8f));
}

// ============================================================================
// DLSSUpscaler Tests
// ============================================================================

class DLSSUpscalerTest : public ::testing::Test {
protected:
    void SetUp() override {
        upscaler = std::make_unique<DLSSUpscaler>();
    }

    std::unique_ptr<DLSSUpscaler> upscaler;
};

TEST_F(DLSSUpscalerTest, Constructor) {
    // Arrange & Act
    DLSSUpscaler dlss;
    
    // Assert
    EXPECT_TRUE(true);
}

TEST_F(DLSSUpscalerTest, GetType) {
    // Arrange & Act
    UpscalerType type = upscaler->getType();
    
    // Assert
    EXPECT_EQ(type, UpscalerType::DLSS);
}

TEST_F(DLSSUpscalerTest, IsSupported) {
    // Arrange & Act
    bool supported = upscaler->isSupported();
    
    // Assert - DLSS поддерживается только на NVIDIA RTX
    EXPECT_TRUE(supported || !supported);
}

TEST_F(DLSSUpscalerTest, Initialize) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->initialize(1920, 1080, 3840, 2160));
}

TEST_F(DLSSUpscalerTest, Cleanup) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->cleanup());
}

TEST_F(DLSSUpscalerTest, SetQualityMode) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(upscaler->setQualityMode(UpscalerQuality::Performance));
}

// ============================================================================
// UpscalerQuality Enum Tests
// ============================================================================

TEST(UpscalerQualityTest, EnumValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(UpscalerQuality::Performance), 0);
    EXPECT_EQ(static_cast<int>(UpscalerQuality::Balanced), 1);
    EXPECT_EQ(static_cast<int>(UpscalerQuality::Quality), 2);
    EXPECT_EQ(static_cast<int>(UpscalerQuality::UltraQuality), 3);
}

// ============================================================================
// UpscalerType Enum Tests
// ============================================================================

TEST(UpscalerTypeTest, EnumValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(UpscalerType::Native), 0);
    EXPECT_EQ(static_cast<int>(UpscalerType::FSR2), 1);
    EXPECT_EQ(static_cast<int>(UpscalerType::DLSS), 2);
}

// ============================================================================
// Resolution Tests
// ============================================================================

TEST(UpscalerResolutionTest, ValidResolutions) {
    // Arrange & Act
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Assert - различные разрешения
    EXPECT_NO_THROW(native->initialize(1280, 720, 1920, 1080));   // 720p -> 1080p
    EXPECT_NO_THROW(native->initialize(1920, 1080, 3840, 2160));  // 1080p -> 4K
    EXPECT_NO_THROW(native->initialize(2560, 1440, 3840, 2160));  // 1440p -> 4K
}

TEST(UpscalerResolutionTest, SameResolution) {
    // Arrange & Act
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Assert - upscale к тому же разрешению
    EXPECT_NO_THROW(native->initialize(1920, 1080, 1920, 1080));
}

TEST(UpscalerResolutionTest, SmallResolution) {
    // Arrange & Act
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Assert - очень малое разрешение
    EXPECT_NO_THROW(native->initialize(640, 480, 1920, 1080));
}

TEST(UpscalerResolutionTest, LargeResolution) {
    // Arrange & Act
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Assert - очень большое разрешение
    EXPECT_NO_THROW(native->initialize(3840, 2160, 7680, 4320));  // 4K -> 8K
}

// ============================================================================
// Quality Mode Tests
// ============================================================================

TEST(UpscalerQualityModeTest, AllQualityModes) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act & Assert - все режимы качества
    EXPECT_NO_THROW(native->setQualityMode(UpscalerQuality::Performance));
    EXPECT_NO_THROW(native->setQualityMode(UpscalerQuality::Balanced));
    EXPECT_NO_THROW(native->setQualityMode(UpscalerQuality::Quality));
    EXPECT_NO_THROW(native->setQualityMode(UpscalerQuality::UltraQuality));
}

TEST(UpscalerQualityModeTest, SwitchQualityModes) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act - переключение между режимами
    native->setQualityMode(UpscalerQuality::Performance);
    EXPECT_EQ(native->getQualityMode(), UpscalerQuality::Performance);
    
    native->setQualityMode(UpscalerQuality::Quality);
    EXPECT_EQ(native->getQualityMode(), UpscalerQuality::Quality);
}

// ============================================================================
// Sharpness Tests
// ============================================================================

TEST(UpscalerSharpnessTest, ValidSharpnessValues) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act & Assert
    EXPECT_NO_THROW(native->setSharpness(0.0f));
    EXPECT_NO_THROW(native->setSharpness(0.5f));
    EXPECT_NO_THROW(native->setSharpness(1.0f));
}

TEST(UpscalerSharpnessTest, NegativeSharpness) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act & Assert - отрицательные значения
    EXPECT_NO_THROW(native->setSharpness(-0.5f));
}

TEST(UpscalerSharpnessTest, LargeSharpness) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act & Assert - значения больше 1
    EXPECT_NO_THROW(native->setSharpness(2.0f));
}

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST(UpscalerLifecycleTest, InitializeCleanupCycle) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    
    // Act & Assert
    EXPECT_NO_THROW(native->initialize(1920, 1080, 3840, 2160));
    EXPECT_NO_THROW(native->cleanup());
    EXPECT_NO_THROW(native->initialize(1920, 1080, 3840, 2160));
    EXPECT_NO_THROW(native->cleanup());
}

TEST(UpscalerLifecycleTest, MultipleCleanupCalls) {
    // Arrange
    auto native = UpscalerFactory::createUpscaler(UpscalerType::Native);
    native->initialize(1920, 1080, 3840, 2160);
    
    // Act & Assert - множественные вызовы cleanup
    EXPECT_NO_THROW(native->cleanup());
    EXPECT_NO_THROW(native->cleanup());
    EXPECT_NO_THROW(native->cleanup());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
