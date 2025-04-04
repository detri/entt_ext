#pragma once
#include "config/types.hpp"
#include "entt/entt.hpp"

namespace entt_ext
{
    /**
     * @brief A game system that updates based on a fixed or dynamic time delta.
     * Receives and holds a non-owning registry pointer that can be used for queries.
     * @code{.cpp}
     *  class my_system : public entt_ext::system {
     *  public:
     *      explicit my_system(entt::registry* registry) : entt_ext::system{registry} {}
     *
     *      void update(const float dt) override
     *      {
     *          for (auto&& [ent, comp] : registry->view<my_component>().each())
     *          {
     *              // do stuff
     *          }
     *      }
     *  }
     * @endcode
     */
    class system
    {
    public:
        explicit system(entt::registry* registry) : registry{registry}
        {
            if (!registry)
            {
                throw std::invalid_argument("Pointed-to registry must not be null");
            }
        }

        virtual ~system() = default;

        system(const system&) = delete;
        system& operator=(const system&) = delete;

        system(system&& other) noexcept : registry{other.registry}
        {
            other.registry = nullptr;
        }

        system& operator=(system&& other) noexcept
        {
            if (this != &other)
            {
                registry = other.registry;
                other.registry = nullptr;
            }
            return *this;
        }

        virtual void update(float_type delta_time) = 0;

    protected:
        entt::registry* registry;
    };

    template<typename T>
    concept system_class = std::derived_from<T, system>;
}
