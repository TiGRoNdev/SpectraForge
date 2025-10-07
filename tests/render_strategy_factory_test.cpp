#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "SpectraForge/Rendering/IRenderStrategy.h"
#include "SpectraForge/Rendering/StrategyFactory.h"

using namespace SpectraForge::Rendering;

// AAA: Arrange, Act, Assert

TEST(StrategyFactory, CreatesFreGSByCanonicalName) {
    // Arrange
    const std::string name = "freqs";

    // Act
    auto strategy = create_strategy_by_name(name);

    // Assert
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->strategy_name(), std::string("FreGS"));
}

TEST(StrategyFactory, CreatesFreGSByAliasNames) {
    // Arrange + Act
    auto s1 = create_strategy_by_name("fregs");
    auto s2 = create_strategy_by_name("gaussian");

    // Assert
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    EXPECT_EQ(s1->strategy_name(), std::string("FreGS"));
    EXPECT_EQ(s2->strategy_name(), std::string("FreGS"));
}

TEST(StrategyFactory, ReturnsNullptrOnUnknownName) {
    // Arrange
    const std::string name = "unknown_strategy";

    // Act
    auto strategy = create_strategy_by_name(name);

    // Assert
    EXPECT_EQ(strategy, nullptr);
}


