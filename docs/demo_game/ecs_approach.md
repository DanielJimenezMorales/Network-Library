# ECS Approach
Last time updated: 12/02/2025

## Index:
- Introduction
- Dependencies
- [ECS terms glossary](ecs_terms_glossary.md)
- [How to configure archetypes and prefabs](how_to_configure_archetypes_and_prefabs.md)
- [How to create and destroy game entities](how_to_create_and_destroy_game_entities.md)
- Components
- Game entities
- Global components
- Entity container
- Archetypes
- Prefabs
- Entity factories
- Simple systems
- System coordinators
- Events
- A multithread friendly approach (FUTURE FEATURE)
- References

## Dependencies:
This ECS approach uses [Entt](https://github.com/skypjack/entt) for a low level components management. I would like to switch to a custom ECS approach in the future. However I've not found the time/priority to work on it.

## Components:
Components are data-only containers with no logical code inside. They only contain variables and simple methods like Getters or Setters. Each component represents a specific aspect of an entity. For example, the HealthComponent represents the health of an entity while the Collider2DComponent represents the collider's shape of an entity. Each component is responsible of ensuring the correct copy and move behaviours for all the fields that it contains.

## Game entities:
Game entities are basically an utility class that helps human programmers work easily with components. Each game entity contains an ID and a pointer to a world that the entity belongs to (where all its components live in). Each entity will have assigned a set of different components. It is possible to assign as many components as you want to an entity. However, you can not assign two components of the same type to the same entity.

## Global components:
There are certain types of data structures that need to live inside our World but we only need one instance of them, contrary to the rest of components that can be attached to multiple entities at the same time. These are known as Global Components.

## Entity container:
An Entity container is where all components from all entities are stored. Instead of letting each Game Entity stores all its components, like in an OOP paradigm, the entity container acts as a centralized storage for all the componentes of the same type. Internally, it uses pools of components of the same type. This data-oriented approach has a lot of benefits such as reducing memory fragmentation and cache misses, improving as a consequence access performance.

## Archetype:
An archetype is a layout that contains a set of components that different game entities share.

## Prefabs:
A prefab represents a template for an already configured entity based on an archetype. A prefab will contain the configuration for the components of an archetype.

## Entity factories:
Entities can only be created through special classes called factories (From the factory pattern). These factories will be responsible for attaching and initializing all the components that an entity of a certain type needs. In order to perform this first initialization, factories will receive an Entity Configu ration structure for flexibility purposes. Also, for symmetry, entity factories will also be responsible of de-initializing all the components of an entity, if neccessary.

## Simple systems:
Simple systems corresponds to the S in ECS. These systems are logic-only classes that will manipulate components and global components during one stage of the main game loop (UPDATE, TICK, RENDER, PRETICK...). Although simple systems are logic-only classes, they are not stateless, as they could need to handle an internal state. For example, the GizmoRendererSystem will contain a GizmoResourceHandler that will act as a centralized storage for all gizmo resources. This GizmoResourceHandler follows the [Handler pattern](https://floooh.github.io/2018/06/17/handles-vs-pointers.html) and will be explained in another [post](##ADD_POST_HERE).

## System coordinators:
Sometimes, there can be cases where we want one simple system to execute before another one. In order to do that, the game loop stages were created. For example, we know that a simple system added to the UPDATE stage will be executed before that a simple system added to the RENDER stage. However, this is not the perfect solution since within the same stage we could run into the same problem. In order to fix this, the system coordinator was created. The coordinator will contain an ordered set of simple systems of the same stage that will be executed following that order.

## Events:
This approach offers a few ECS-related events.
- OnEntityCreate<GameEntity>: A new entity has been created.
- OnEntityDestroy<GameEntity>: An entity is about to get destroyed.

## A multithread friendly approach (FUTURE FEATURE)
I am aiming for implementing a multithreaded ECS approach. In order to do this, I've thought in three different ways:
1. Parallelize simple systems: As we have been discussing above, simple systems contain logic that reads and writes data from components and/or global components. If we are able to, somehow parallelize this execution in a way that there are no systems reading and/or writting to the same component at the same time we could speed up a bit this process. The system orchestrator is planned to be an algorithm for ordering systems in order to satisfy this concept.

The only safe case would be when two systems reads from the same component at the same time. Because READ WRITE or WRITE WRITE use cases could create race conditions.

2. Parallelize entities within the same system: Another idea is to, instead of implementing an approach where systems are parallelized, implement a SIMD paralellism where for each system, we create different threads for manipulating each entity.

3. A mix of options 1 and 2

At the moment these are just high level ideas and this feature needs more work and investigation from my end before concluding with a final decision. If you have any idea and wouldn't mind sharing it with me, feel free to reach out to me through Linkedin, Twitter or simply by email.

## References
- https://austinmorlan.com/posts/entity_component_system/
- https://gaijinentertainment.github.io/DagorEngine/api-references/dagor-ecs/dagor_ecs.html
- https://www.richardlord.net/blog/ecs/what-is-an-entity-framework
- https://docs.spacestation14.com/en/robust-toolbox/ecs.html