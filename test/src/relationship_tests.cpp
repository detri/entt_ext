#include "gtest/gtest.h"
#include "entt_ext.hpp"

TEST(Relationship, ParentChildCreation)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);
    int child_count = 0;
    for (auto child : registry.children(parent_ent))
    {
        ASSERT_EQ(child, child_ent);
        child_count++;
    }
    ASSERT_EQ(child_count, 1);
}

TEST(Relationship, ParentChildDestruction)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    std::vector<entt::entity> children;
    for (int i = 0; i < 10; i++)
    {
        children.emplace_back(registry.create_child(parent_ent));
    }
    for (const auto child : children)
    {
        registry.destroy(child);
    }

    int child_count = 0;
    for (auto child : registry.children(parent_ent))
    {
        child_count++;
    }
    ASSERT_EQ(child_count, 0);
}

TEST(Relationship, UnparentChild)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);
    registry.unparent(child_ent);
    int child_count = 0;
    for (auto child : registry.children(parent_ent))
    {
        child_count++;
    }
    ASSERT_EQ(child_count, 0);
    ASSERT_TRUE(registry.raw()->valid(child_ent));
}


TEST(Relationship, NestedChildren)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);
    const auto grandchild_ent = registry.create_child(child_ent);
    int total_child_count = 0;
    for (auto child : registry.children(parent_ent))
    {
        ASSERT_EQ(child, child_ent);
        total_child_count++;
        for (auto grandchild : registry.children(child))
        {
            ASSERT_EQ(grandchild, grandchild_ent);
            total_child_count++;
        }
    }
    ASSERT_EQ(total_child_count, 2);
}

TEST(Relationship, CircularReferencePrevention)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);

    // Manually set an invalid circular relationship
    auto& child_rel = registry.get_relationship(child_ent);
    child_rel.next_sibling = parent_ent;

    int child_count = 0;
    for (auto child : registry.children(parent_ent))
    {
        child_count++;
        if (child_count > 10) // Prevent infinite loop in case of circular reference
        {
            FAIL() << "Circular reference caused infinite iteration";
        }
    }
    SUCCEED();
}

TEST(Relationship, ParentEntityDestruction)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);

    registry.destroy(parent_ent);
    ASSERT_FALSE(registry.raw()->valid(parent_ent));
    ASSERT_FALSE(registry.raw()->valid(child_ent));
}

TEST(Relationship, ReparentingChild)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};

    const auto parent1 = registry.create();
    const auto parent2 = registry.create();
    const auto child_ent = registry.create_child(parent1);

    registry.reparent(child_ent, parent2);

    int child_count_parent1 = 0;
    for (auto child : registry.children(parent1)) {
        child_count_parent1++;
    }
    ASSERT_EQ(child_count_parent1, 0);

    int child_count_parent2 = 0;
    for (auto child : registry.children(parent2)) {
        ASSERT_EQ(child, child_ent);
        child_count_parent2++;
    }
    ASSERT_EQ(child_count_parent2, 1);
}

TEST(Relationship, IteratorEndBehavior)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto parent_ent = registry.create();
    const auto child_ent = registry.create_child(parent_ent);

    auto it = registry.children(parent_ent).begin();
    ASSERT_EQ(*it, child_ent);
    ++it;
    ASSERT_EQ(it, registry.children(parent_ent).end());
}

TEST(Relationship, LargeHierarchy)
{
    entt::registry entt_reg;
    entt_ext::relational_registry registry{entt_reg};
    const auto root = registry.create();
    constexpr int depth = 1000;

    auto current = root;
    for (int i = 0; i < depth; i++) {
        current = registry.create_child(current);
    }

    int child_count = 0;
    current = root;
    while (registry.get_relationship(current).first_child != entt::null) {
        child_count++;
        current = registry.get_relationship(current).first_child;
    }

    ASSERT_EQ(child_count, depth);
}
