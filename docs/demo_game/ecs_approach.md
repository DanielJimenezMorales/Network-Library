# ECS Approach
Last time updated: 12/02/2025

## Index:
- Introduction
- Dependencies
- [ECS terms glossary](ecs_terms_glossary.md)
- [How to configure archetypes and prefabs](how_to_configure_archetypes_and_prefabs.md)
- [How to create and destroy game entities](how_to_create_and_destroy_game_entities.md)
- [How to create new components](how_to_create_new_components.md)
- A multithread friendly approach (FUTURE FEATURE)
- References

## Dependencies:
This ECS approach uses [Entt](https://github.com/skypjack/entt) for a low level components management. I would like to switch to a custom ECS approach in the future. However I've not found the time/priority to work on it.

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