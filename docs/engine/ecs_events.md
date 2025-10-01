# ECS events
Last time updated: 16/02/2025

## Introduction:
In this page we will list the different ecs related events that users can subscribe to.

## Index:
- [OnEntityCreate](#onentitycreate)
- [OnEntityDestroy](#onentitydestroy)
- [OnEntityConfigure](#onentityconfigure)

## OnEntityCreate
This event is called inmediately after a game entity is created. It is located within the `ECS::World` class and subscribers will receive a `ECS::GameEntity&` with the new entity created as a parameter.

In order to subscribe to this event, you will need to use `uint32 ECS::World::SubscribeToOnEntityCreate(Functor&& functor)`.

## OnEntityDestroy
This event is called inmediately before a game entity is destroyed. Due to game entities are destroyed in a deferred way, this event will alwways bee called during the end of frame. It is located within the `ECS::World` class and subscribers will receive a `ECS::GameEntity&` with the entity to be destroyed as a parameter.

In order to subscribe to this event, you will need to use `uint32 ECS::World::SubscribeToOnEntityDestroy(Functor&& functor)`.

## OnEntityConfigure
This event is called during the game entity creation process, before the `OnEntityCreate` event. This event is useful to subscribe all the functions responsible for configuring components. It is located within the `ECS::World` class and subscribers will receive as parameters a `ECS::GameEntity&` with the entity being configured and a `const ECS::Prefab&` with the input prefab where all the component configurations.

In order to subscribe to this event, you will need to use `uint32 ECS::World::SubscribeToOnEntityConfigure(Functor&& functor)`.

An example of a sprite renderer component configuration function would be:
```cpp
void ConfigureSpriteRendererCmp(ECS::GameEntity& entity, const ECS::Prefab& prefab)
{
	//Check if this prefab has a Sprite renderer config
	auto cmp_config_found = prefab.componentConfigurations.find("SpriteRenderer");
	if (cmp_config_found == prefab.componentConfigurations.end())
	{
		return;
	}

	//Check if this entity has a Sprite renderer cmp attached
	if (!entity.HasComponent<SpriteRendererComponent>())
	{
		return;
	}

	//Get sprite renderer cmp and config
	const SpriteRendererComponentConfiguration& config = static_cast<const SpriteRendererComponentConfiguration&>(*cmp_config_found->second);
	SpriteRendererComponent& sprite_renderer = entity.GetComponent<SpriteRendererComponent>();

	//Configure cmp by loading the texture from the config and passing a handler to the sprite resource to the cmp
	sprite_renderer.textureHandler = LoadTexture(sprite_renderer_config.texturePath.c_str());
}

int main()
{
	ECS::World world;
	world.SubscribeToOnEntityConfigure( ConfigureSpriteRendererCmp );
}
```