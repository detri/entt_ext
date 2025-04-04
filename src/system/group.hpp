#pragma once
#include "system.hpp"
#include "entt/entt.hpp"

namespace entt_ext
{
    class group
    {
    public:
        group() = default;

        /**
         * Schedule a system at the end of this group.
         * @tparam System The new system to add to the group.
         * @tparam Args Argument types forwarded to system construction
         * @param args System args to forward to construction
         * @return A reference to the system within the group
         */
        template<system_class System, typename... Args>
        System& emplace(Args&&... args)
        {
            auto sys = std::make_unique<System>(std::forward<Args>(args)...);
            System& ref = *sys;
            systems.push_back(std::move(sys));
            return ref;
        }

        /**
         * Run all systems within this group in the order they were added
         * @param delta_time Amount of time passed since the last frame
         */
        void update(const float delta_time) const
        {
            for (auto& sys : systems)
            {
                sys->update(delta_time);
            }
        }

        [[nodiscard]] size_t size() const { return systems.size(); }
        [[nodiscard]] bool empty() const { return systems.empty(); }
        void clear() { systems.clear(); }

        auto begin() { return systems.begin(); }
        auto end() { return systems.end(); }
        [[nodiscard]] auto begin() const { return systems.begin(); }
        [[nodiscard]] auto end() const { return systems.end(); }
    private:
        std::vector<std::unique_ptr<system>> systems;
    };
}
