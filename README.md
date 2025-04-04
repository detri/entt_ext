# entt_ext - Unofficial EnTT Extensions
This library adds some features to [EnTT](https://github.com/skypjack/entt) that I find useful,
but may not necessarily meet the goals of the EnTT project, or are not a one-size-fits-all solution for a particular problem
that you would expect from EnTT. Nonetheless, I hope this can be useful to others.

## Features

- Entity relationships (via component and registry wrapper)
- Systems (alternative to entt::process)
- Groups (ordered schedulers for "always-run" systems)

## Use in your project

I recommend using git submodules with CMake. This project was designed for that paradigm.

#### Example
##### Shell
Add the repo somewhere as a submodule in your own repo:
```shell
$ cd myproject
$ mkdir -p vendored && cd vendored
$ git submodule add https://github.com/detri/entt_ext
```

##### Your CMakeLists.txt
```cmake
# Example project with executable target
project(myproject CXX)
add_executable(myproject)

# Add entt_ext targets
add_subdirectory(vendored/entt_ext)
```

entt_ext does not automatically link EnTT, it just makes use of its interfaces.
However, the CMake file will fetch it and make it available by default:
```cmake
target_link_libraries(myproject PRIVATE EnTT::EnTT entt_ext)
```
You can provide the `EnTT::EnTT` target yourself by disabling `ENTT_EXT_PROVIDE_ENTT`.

## Relationships

Inspired by (stolen from) these incredible articles by the creator of EnTT:

[ECS Back and Forth Part 4 - Hierarchies](https://skypjack.github.io/2019-06-25-ecs-baf-part-4/)

[ECS Back and Forth Part 4 Insights](https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights/)

(Note that I didn't add the sorting mentioned in the insights article. I couldn't even figure out how to write tests that fragmented entities in such a way
that pre-sorting was consistently more performant for accessing entities parent-down, though that's probably a skill issue.
You can sort the relationship storage manually if needed, though!)

Sometimes one component per entity is not enough. An entity might be able to have more than one of something: animations, abilities, inventory items, etc.

This can be modeled by a `relationship` component that acts as both a link back to the parent and a linked list node of siblings:

```c++
struct relationship
{
    entt::entity parent{entt::null};
    entt::entity first_child{entt::null};
    entt::entity last_child{entt::null};
    entt::entity prev_sibling{entt::null};
    entt::entity next_sibling{entt::null};
    std::size_t child_count{0};
};
```

Manually managing a linked list structure is cumbersome, so `entt_ext` also provides a non-owning utility
wrapper around `entt::registry` called `entt::relational_registry`:

```c++
#include "entt_ext.hpp"

entt::registry normal_registry;
entt_ext::relational_registry rel_registry{normal_registry};

struct comp1 {};
struct comp2 {};

// this is created with a relationship component:
entt::entity parent = rel_registry.create();
// creates a new entity and makes it a child of another relational entity:
entt::entity child = rel_registry.create_child(parent);
entt::entity child2 = rel_registry.create_child(parent);

// normal entities will never appear in child views (rel_registry.children(parent))
entt::entity orphan = normal_registry.create();

// using the standard registry to add components
normal_registry.emplace<comp1>(child);
// this also works if the wrapped registry is alive but out of scope
rel_registry.raw()->emplace<comp2>(child2);

for (entt::entity child_ent : rel_registry.children(parent)) {
    assert(child_ent == child || child_ent == child2); // passes
}
```

I chose to make the relational registry a wrapper because there may be parts of the app
that don't use relationships. You may not want to uproot an existing registry just to use relationships.
You can add relational features by creating entities via temporarily wrapping a registry
wherever needed, then query the children later with a different wrapper.

## Systems and Groups
### Systems
Systems take an alternate philosophy to `entt::process`.
They are virtual classes with an overrideable `update` method which takes a `float`.
The base constructor takes and stores a non-owning `entt::registry` pointer.
They assume the registry will always outlive them. The advantage of this assumption is
that systems can define and manage their own views and access registry context data
while still being encapsulated, without needing void pointer arguments for arbitrary data.

#### Example
```c++
#include "entt_ext.hpp"

class gravity_system : public entt_ext::system {
    using super = entt_ext::system;
public:
    explicit gravity_system(entt::registry* registry) : super{registry} {}
    
    void update(float dt) const override {
        for (auto&& [ent, vel] : registry->view<velocity>().each()) {
            vel.y -= 9.8f * dt;
        }
    }
}
```

### Groups
Systems often need to run in a deterministic order. Groups help facilitate that.
A Group will run systems placed in them, in the order placed, whenever the group is run.

#### Example (using Raylib functions)
```c++
#include "raylib.h"
#include "entt_ext.hpp"

int main() {
    entt::registry registry;

    using group = entt_ext::group;
    
    group setup;
    group update;
    group render;
    
    // example systems
    setup.emplace<create_entities_system>(&registry);
    update.emplace<update_velocity_system>(&registry);
    render.emplace<render_system>(&registry);
    
    InitWindow(640, 480, "Test Window");
    
    setup.run(0.f);
    setup.clear();
    
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        update.run(dt);
        render.run(dt);
    }
    
    return 0;
}
```