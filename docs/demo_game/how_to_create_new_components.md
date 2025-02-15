# Create new components
Last time updated: 15/02/2025

## Introduction:
In this page we will discuss how to create and use new components within the ECS architecture.

## Index:
- [Create the component](#create-the-component)
- [Register the component](#register-the-component)

## Create the component
As the glossary explains, components are data-only structures that only contain variables and simple methods like getters and setters. Due to this, components are defined as C++ structs. As an example, the following code snippet shows an example of a component responsible for handling the health of a game entity:
```cpp
struct HealthComponent
{
	uint32 maxHealth;
	uint32 currentHealth;
};
```

## Register the component
Once the component has been defined in its own file, before using it devs will need to register that component in order to the world to be aware of it. To register a component, the function `bool Scene::RegisterComponent<T>(const std::string& name)` is used. This is a really important step as we will be able to attach components to a game entity based on its name during the [game entity creation process](how_to_create_and_destroy_game_entities.md). The following code snippet shows an example of how to use this function:
```cpp
void RegisterHealthComponent(Scene& scene)
{
	const bool result = scene.RegisterComponent<HealthComponent>("HealthComponent");
	assert(result);
}
```
> Note: Make sure that when [creating the archetype and prefab configuration files](how_to_configure_archetypes_and_prefabs.md) you use the same name as the one used to register the component, otherwise the system won't be able to find the association.