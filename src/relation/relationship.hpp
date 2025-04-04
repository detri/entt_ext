#pragma once
#include "entt/entt.hpp"

namespace entt_ext
{
    /**
     * Relationship component tying an entity to a parent and any siblings.
     * Child access is linked-list style.
     */
    struct relationship
    {
        entt::entity parent{entt::null};
        entt::entity first_child{entt::null};
        entt::entity last_child{entt::null};
        entt::entity prev_sibling{entt::null};
        entt::entity next_sibling{entt::null};
        std::size_t child_count{0};
    };

    class child_iterator
    {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = entt::entity;
        using reference = const entt::entity&;
        using pointer = const entt::entity*;

        child_iterator(entt::registry* reg, const value_type current)
            : registry(reg), current_entity(current)
        {
        }

        reference operator*() const { return current_entity; }

        child_iterator& operator++()
        {
            const auto& rel = registry->get<relationship>(current_entity);
            current_entity = rel.next_sibling;
            return *this;
        }

        bool operator==(const child_iterator& other) const
        {
            return current_entity == other.current_entity;
        }

        bool operator!=(const child_iterator& other) const = default;

    private:
        entt::registry* registry;
        entt::entity current_entity;
    };

    class child_range
    {
    public:
        child_range(entt::registry* registry, const entt::entity first)
            : registry(registry), first_child(first)
        {
        }

        [[nodiscard]] child_iterator begin() const { return {registry, first_child}; }
        [[nodiscard]] child_iterator end() const { return {registry, entt::null}; }

    private:
        entt::registry* registry;
        entt::entity first_child;
    };

    /**
     * Wraps an EnTT registry to provide automatic relationship management.
     * Enables the use of relationships only where necessary.
     * Be sure to create and destroy entities that need relationships
     * via this wrapper (or update the relationships manually).
     */
    class relational_registry
    {
        entt::registry* registry;

        void update_sibling_links(const entt::entity prev, const entt::entity next) const
        {
            if (prev != entt::null)
            {
                get_relationship(prev).next_sibling = next;
            }
            if (next != entt::null)
            {
                get_relationship(next).prev_sibling = prev;
            }
        }

        void attach_child(const entt::entity parent_entity, const entt::entity child_entity) const
        {
            auto& parent_rel = get_relationship(parent_entity);
            auto& child_rel = get_relationship(child_entity);

            child_rel.parent = parent_entity;
            ++parent_rel.child_count;

            if (parent_rel.last_child == entt::null)
            {
                parent_rel.first_child = child_entity;
                parent_rel.last_child = child_entity;
            }
            else
            {
                auto& last_child_rel = get_relationship(parent_rel.last_child);
                last_child_rel.next_sibling = child_entity;
                child_rel.prev_sibling = parent_rel.last_child;
                parent_rel.last_child = child_entity;
            }
        }

    public:
        /**
         * Construct a relational registry wrapper from a registry pointer
         * @param registry Pointer to an EnTT registry
         */
        explicit relational_registry(entt::registry* registry) : registry(registry) {}

        /**
         * Construct a relational registry wrapper from a registry reference
         * @param registry Reference to an EnTT registry
         */
        explicit relational_registry(entt::registry& registry) : registry(&registry) {}

        relational_registry() = delete;
        relational_registry(const relational_registry&) = delete;
        relational_registry& operator=(const relational_registry&) = delete;
        relational_registry(relational_registry&&) = delete;
        relational_registry& operator=(relational_registry&&) = delete;

        /**
         * @return Pointer to the underlying registry.
         */
        entt::registry* raw() { return registry; }

        /**
         * @copydoc raw
         */
        [[nodiscard]] const entt::registry* raw() const { return registry; }

        /**
         * @brief Creates an entity with a relationship inside the registry
         * @return The newly created entity
         */
        [[nodiscard]] entt::entity create() const
        {
            const auto entity = registry->create();
            registry->emplace<relationship>(entity);
            return entity;
        }

        /**
         * @brief Get the relationship component of an entity
         * @param entity Entity
         * @return The relationship component associated with the entity
         */
        [[nodiscard]] relationship& get_relationship(const entt::entity entity) const
        {
            return registry->get<relationship>(entity);
        }

        /**
         * @brief Create a new child of a parent entity
         * @param parent_entity Parent to add a child to
         * @return The new child entity
         */
        [[nodiscard]] entt::entity create_child(const entt::entity parent_entity) const
        {
            if (!registry->valid(parent_entity))
            {
                return entt::null;
            }
            const auto child_entity = create();
            attach_child(parent_entity, child_entity);
            return child_entity;
        }

        /**
         * @brief Removes a child entity from its parent without destroying it
         * @param child_entity Child to unparent
         */
        void unparent(const entt::entity child_entity) const
        {
            if (!registry->valid(child_entity))
            {
                return;
            }

            auto& child_rel = get_relationship(child_entity);
            auto& parent_rel = get_relationship(child_rel.parent);

            if (child_rel.parent == entt::null)
            {
                return;
            }

            if (parent_rel.first_child == child_entity)
            {
                parent_rel.first_child = child_rel.next_sibling;
            }
            if (parent_rel.last_child == child_entity)
            {
                parent_rel.last_child = child_rel.prev_sibling;
            }

            update_sibling_links(child_rel.prev_sibling, child_rel.next_sibling);

            child_rel.parent = entt::null;
            --parent_rel.child_count;
        }

        /**
         * @brief Reassigns an already-parented child entity to a new parent
         * @param child_entity Child entity to reparent
         * @param new_parent_entity New parent entity of the child
         */
        void reparent(const entt::entity child_entity, const entt::entity new_parent_entity) const
        {
            if (!registry->valid(child_entity) || !registry->valid(new_parent_entity))
            {
                return;
            }

            unparent(child_entity);
            attach_child(new_parent_entity, child_entity);
        }

        /**
         * @brief Destroys an entity and all of its related children recursively
         * @param entity Entity to destroy
         */
        void destroy(const entt::entity entity)
        {
            if (!registry->valid(entity))
            {
                return;
            }

            const auto& rel = registry->get<relationship>(entity);
            auto curr = rel.first_child;
            while (curr != entt::null)
            {
                const auto next = get_relationship(curr).next_sibling;
                destroy(curr);
                curr = next;
            }

            if (rel.parent != entt::null)
            {
                unparent(entity);
            }

            registry->remove<relationship>(entity);
            registry->destroy(entity);
        }

        /**
         * @brief Get a range of a parent entity's children.
         * @param parent_entity Parent with children to iterate through
         * @return Child entity range iterator
         */
        [[nodiscard]] child_range children(const entt::entity parent_entity) const
        {
            const auto first_child = registry->valid(parent_entity)
                                         ? get_relationship(parent_entity).first_child
                                         : entt::null;
            return child_range{registry, first_child};
        }
    };
}
