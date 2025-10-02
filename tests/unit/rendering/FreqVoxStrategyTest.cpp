#include <gtest/gtest.h>
#include "SpectraForge/Rendering/FreqVox/FreqVoxStrategy.h"

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Rendering::FreqVox;

TEST(FreqVoxStrategy, Lifecycle) {
    FreqVoxStrategy strat;
    EXPECT_TRUE(strat.initialize());
    FrameData fd{};
    strat.render(fd);
    EXPECT_EQ(strat.getName(), std::string("FreqVoxStrategy"));
    strat.shutdown();
}


