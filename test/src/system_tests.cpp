#include "gtest/gtest.h"
#include "entt_ext.hpp"

class test_system final : public entt_ext::system
{
    using super = system;
    std::function<void(float)> update_func;
public:
    explicit test_system(entt::registry* registry) : super{registry}
    {
    }

    void set_update_func(std::function<void(float)>&& update_func)
    {
        this->update_func = update_func;
    }

    void update(const float delta_time) override
    {
        update_func(delta_time);
    }
};

TEST(System, GroupOrder)
{
    entt::registry registry;
    entt_ext::group group;
    std::vector<int> order;
    constexpr int NUM_SYSTEMS = 10;
    for (int i = 0; i < NUM_SYSTEMS; ++i)
    {
        auto& sys = group.emplace<test_system>(&registry);
        sys.set_update_func([&order, i](float) { order.push_back(i); });
    }
    group.update(0.0f);
    const auto target_order = std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(order, target_order);
}
