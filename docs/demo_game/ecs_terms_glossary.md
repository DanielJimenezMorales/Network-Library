# ECS terms glossary
Last time updated: 16/02/2025

## Introduction:
In this page we will explain the different terms used along these doc pages to fully understand what are the ECS concepts used in this approach.

## Index:
- [Components](#components)
- [Game entities](#game-entities)
- [Global components](#global-components)
- [Entity container](#entity-container)
- [Archetypes](#archetypes)
- [Prefabs](#prefabs)
- [Simple systems](#simple-systems)
- [System coordinators](#system-coordinators)
- [Events](#events)

## Components:
Components are data-only containers with no logical code inside. They only contain variables and simple methods like Getters or Setters. Each component represents a specific aspect of an entity. For example, the HealthComponent represents the health of an entity while the Collider2DComponent represents the collider's shape of an entity. Each component is responsible of ensuring the correct copy and move behaviours for all the fields that it contains.

## Game entities:
Game entities are basically an utility class that helps human programmers work easily with components. Each game entity contains an ID and a pointer to a world that the entity belongs to (where all its components live in). Each entity will have assigned a set of different components. It is possible to assign as many components as you want to an entity. However, you can not assign two components of the same type to the same entity.

## Global components:
There are certain types of data structures that need to live inside our World but we only need one instance of them, contrary to the rest of components that can be attached to multiple entities at the same time. These are known as Global Components.

## Entity container:
An Entity container is where all components from all entities are stored. Instead of letting each Game Entity stores all its components, like in an OOP paradigm, the entity container acts as a centralized storage for all the componentes of the same type. Internally, it uses pools of components of the same type. This data-oriented approach has a lot of benefits such as reducing memory fragmentation and cache misses, improving as a consequence access performance.

## Archetype:
An archetype is a layout that contains a set of components that different game entities will share.

## Prefabs:
A prefab represents a template for an already configured entity based on an archetype. A prefab will contain the configuration for the components of an archetype.

## Simple systems:
Simple systems corresponds to the S in ECS. These systems are logic-only classes that will manipulate components and global components during one stage of the main game loop (UPDATE, TICK, RENDER, PRETICK...). Although simple systems are logic-only classes, they are not stateless, as they could need to handle an internal state. For example, the GizmoRendererSystem will contain a GizmoResourceHandler that will act as a centralized storage for all gizmo resources. This GizmoResourceHandler follows the [Handler pattern](https://floooh.github.io/2018/06/17/handles-vs-pointers.html) and will be explained in another [post](##ADD_POST_HERE).

## System coordinators:
Sometimes, there can be cases where we want one simple system to execute before another one. In order to do that, the game loop stages were created. For example, we know that a simple system added to the UPDATE stage will be executed before that a simple system added to the RENDER stage. However, this is not the perfect solution since within the same stage we could run into the same problem. In order to fix this, the system coordinator was created. The coordinator will contain an ordered set of simple systems of the same stage that will be executed following that order.

## Events:
Events are functions that notify other parts of the code about different ECS-actions happening in a flexible and scalable way. For example, when an entity has spawned. In order to get notified of these events you will need first to subscribe to them. In order to know all the different types of events, visit this [post](ecs_events.md).