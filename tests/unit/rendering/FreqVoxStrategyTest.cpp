#include <gtest/gtest.h>
#include "HyperEngine/Rendering/FreqVox/FreqVoxStrategy.h"

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Rendering::FreqVox;

TEST(FreqVoxStrategy, Lifecycle) {
    FreqVoxStrategy strat;
    EXPECT_TRUE(strat.initialize());
    FrameData fd{};
    strat.render(fd);
    EXPECT_EQ(strat.getName(), std::string("FreqVoxStrategy"));
    strat.shutdown();
}


