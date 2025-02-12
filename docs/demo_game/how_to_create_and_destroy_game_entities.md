# Create entities
Last time updated: 12/02/2025

## Introduction:
In this page we will discuss how to create and destroy ECS game entities.

## Creating a game entity
The game entity creation is an instant process. That means that entities will be spawned as soon as the creation function is called. There won't be any kind of deferred creation process.
>> Dev notes: My initial thought was to make the game entity creation deferred in order to improve determinism for the network system. Creating game entities in the middle of a tick could be dangerous as it could lead into potential desyncs between different peers. However, the reason why I've not move forward with this approach is due to the World replication module from the Network Library requires an in-game entity ID inmediately after spawning a network entity in order to associate a Network Entity ID with an In-game Enity ID. And why does the Network Library need to do this association? Because whenever a DESTROY_ENTITY {NETWORK_ENTITY_ID} message arrives to the peer, the network library will need to notify this event to the engine indicating which In-game entity should be destroyed. This operation requires a conversion from the network entity id from the incoming message to the associated in-game entity id.

In order to create a Game Entity there is only one way to do it. Devs will need to call to `Scene::CreateGameEntity` method. This method asks for the following parameters:
- `const std::string& prefab_name`: The name of the prefab that will be used during the creation process.
- `const Vec2f& position`: The position where the Game Entity will be created.

The method will return the created game entity.

The high level process of creating a Game Entity follows these steps:
1. **Get the associated prefab.** The prefab must be already registered.
2. **Get the associated archetype.** The archetype must be already registered.
3. **Create an empty game entity.**
4. **Attach all components from the associated archetype to the new game entity.** All components must be already registered.
5. **Apply all the components configurations defined within the associated prefab to the attached components.** This will be done through the OnConfigureEntity delegate. Systems responsible for configuring each component based on its configuration must be subscribed to the delegate.
6. **Call OnEntityCreate delegate for interested listeners.**

## Destroying a game entity
The game entity destruction is a deferred process. That means that when entities are called to be destroyed that destruction won't happen inmediately. Instead, game entities to be destroyed will be stored in an internal data structure until the end of the current frame. Once it has arrived, all game entities marked to be destroyed during the current frame will dissapear.

In order to destroy an entity, there are two ways of doing it:
### Using `Scene::DestroyGameEntity`
This function can be used in two different ways:
1. By passing as a parameter a `const GameEntity& entity` of the game entity to be destroyed.
2. By passing as a parameter a `const ECS::EntityId entity_id` of the game entity to be destroyed.
### Using `GameEntity::Destroy`
This function will destroy the instance of the entity from where the function has been called.

The high level process of destroying a game entity during the End of frame follows these steps:
1. **Get the game entity instance from the game entity id.** The engine stores only the IDs of the game entities marked to be destroyed as storing a copy of the entire game entity is a bit more expensive.
2. **Call OnEntityDestroy delegate.** This delegate is mostly used for freeing different resources.
3. **Remove the gam entity from the entities container.**