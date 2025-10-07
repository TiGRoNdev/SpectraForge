#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "SpectraForge/Rendering/IRenderStrategy.h"
#include "SpectraForge/Rendering/RenderStrategy/IPassScheduler.h"
#include "SpectraForge/Rendering/RenderStrategy/RenderContextView.h"

using namespace SpectraForge::Rendering;

// === Fakes for testing ===
class FakeContextView final : public IRenderContextView {
  public:
    std::string backend_name() const override { return "Vulkan"; }
    void get_framebuffer_size(uint32_t& out_width, uint32_t& out_height) const override {
        out_width = 1280;
        out_height = 720;
    }
    uint32_t frames_in_flight() const override { return 2u; }
    uint32_t current_frame_index() const override { return 0u; }
};

class FakePassScheduler final : public IPassScheduler {
  public:
    struct PassInfo {
        int priority = 0;
        std::function<void()> execute_fn;
        std::vector<std::string> dependencies;
    };

    bool register_pass(const std::string& pass_name,
                       int priority,
                       std::function<void()> execute_fn) override {
        if (passes_.count(pass_name) != 0) return false;
        passes_[pass_name] = PassInfo{priority, std::move(execute_fn), {}};
        order_.push_back(pass_name);
        return true;
    }

    bool add_dependency(const std::string& pass_name, const std::string& depends_on) override {
        auto it = passes_.find(pass_name);
        if (it == passes_.end()) return false;
        it->second.dependencies.push_back(depends_on);
        return true;
    }

    void execute_all() override {
        for (const auto& name : order_) {
            auto it = passes_.find(name);
            if (it != passes_.end() && it->second.execute_fn) {
                it->second.execute_fn();
                executed_.push_back(name);
            }
        }
    }

    size_t registered_pass_count() const { return passes_.size(); }
    size_t executed_pass_count() const { return executed_.size(); }
    const std::vector<std::string>& executed_order() const { return executed_; }
    const PassInfo* get_pass(const std::string& name) const {
        auto it = passes_.find(name);
        return it == passes_.end() ? nullptr : &it->second;
    }

  private:
    std::unordered_map<std::string, PassInfo> passes_;
    std::vector<std::string> order_;
    std::vector<std::string> executed_;
};

class DummyStrategy final : public IRenderStrategy {
  public:
    bool initialize(std::shared_ptr<IRenderContextView> context_view) override {
        context_ = std::move(context_view);
        initialized_ = (context_ != nullptr);
        return initialized_;
    }
    void shutdown() override { initialized_ = false; }
    bool prepare_pipelines(IPassScheduler& scheduler) override {
        bool ok = true;
        ok = ok && scheduler.register_pass("geometry", 0, [this]() { geometry_ran_ = true; });
        ok = ok && scheduler.register_pass("shading", 1, [this]() { shading_ran_ = true; });
        ok = ok && scheduler.add_dependency("shading", "geometry");
        prepared_ = ok;
        return ok;
    }
    bool update_for_frame(uint64_t frame_index) override {
        last_frame_ = frame_index;
        updated_ = true;
        return true;
    }
    void record_commands() override { recorded_ = true; }
    void render_frame() override { rendered_ = true; }
    std::string strategy_name() const override { return "Dummy"; }

    // Accessors for assertions
    bool is_initialized() const { return initialized_; }
    bool is_prepared() const { return prepared_; }
    bool geometry_ran() const { return geometry_ran_; }
    bool shading_ran() const { return shading_ran_; }
    bool updated() const { return updated_; }
    bool recorded() const { return recorded_; }
    bool rendered() const { return rendered_; }
    uint64_t last_frame() const { return last_frame_; }

  private:
    std::shared_ptr<IRenderContextView> context_;
    bool initialized_ = false;
    bool prepared_ = false;
    bool geometry_ran_ = false;
    bool shading_ran_ = false;
    bool updated_ = false;
    bool recorded_ = false;
    bool rendered_ = false;
    uint64_t last_frame_ = 0;
};

// === Tests (AAA) ===

TEST(RenderStrategyInterfaces, PreparePipelinesRegistersPasses) {
    // Arrange
    auto context = std::make_shared<FakeContextView>();
    DummyStrategy strategy;
    FakePassScheduler scheduler;

    // Act
    const bool init_ok = strategy.initialize(context);
    const bool prep_ok = strategy.prepare_pipelines(scheduler);
    scheduler.execute_all();

    // Assert
    EXPECT_TRUE(init_ok);
    EXPECT_TRUE(prep_ok);
    EXPECT_TRUE(strategy.is_initialized());
    EXPECT_TRUE(strategy.is_prepared());
    EXPECT_EQ(scheduler.registered_pass_count(), 2u);
    const auto* shading = scheduler.get_pass("shading");
    ASSERT_NE(shading, nullptr);
    ASSERT_FALSE(shading->dependencies.empty());
    EXPECT_EQ(shading->dependencies[0], "geometry");
    EXPECT_TRUE(strategy.geometry_ran());
    EXPECT_TRUE(strategy.shading_ran());
}

TEST(RenderStrategyInterfaces, UpdateRecordRenderFlow) {
    // Arrange
    auto context = std::make_shared<FakeContextView>();
    DummyStrategy strategy;
    FakePassScheduler scheduler;
    ASSERT_TRUE(strategy.initialize(context));
    ASSERT_TRUE(strategy.prepare_pipelines(scheduler));

    // Act
    const bool upd_ok = strategy.update_for_frame(7);
    strategy.record_commands();
    strategy.render_frame();

    // Assert
    EXPECT_TRUE(upd_ok);
    EXPECT_TRUE(strategy.updated());
    EXPECT_EQ(strategy.last_frame(), 7u);
    EXPECT_TRUE(strategy.recorded());
    EXPECT_TRUE(strategy.rendered());
    EXPECT_EQ(strategy.strategy_name(), std::string("Dummy"));
}


