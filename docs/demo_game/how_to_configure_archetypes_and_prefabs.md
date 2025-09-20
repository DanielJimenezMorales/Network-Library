# Create new archetypes and prefabs
Last time updated: 12/02/2025

## Introduction:
In this page we will discuss how to set up new archetypes and prefabs in order to be able to use them later in the code to create entities.

## Index:
- [Setting up archetypes](#setting-up-archetypes)
- [Setting up prefabs](#setting-up-prefabs)

## Setting up archetypes
Archetypes represents the skeleton of an entity as they will be containing a set of components that will be attached to an entity during its creation process. The engine will load archetypes from `.../NetworkLibrary/DemoGame/config_files/entity_archetypes` using a JSON loader, so we need to define them in this language.

An archetype have the following fields:
- Name: A unique string-based name to identify the archetype.
- Components: An array of string-based component names.

For exampple, the following code snippet represents the archetype of a wall, which contains three components:
```json
{
	"name": "wall",
	"components":
	[
		"Transform",
		"SpriteRenderer",
		"Collider2D"
	]
}
```

## Setting up prefabs
Prefabs represents a specific configuration for the components of an archetype. As with archetypes, the engine will also use a JSON loader to load them from `.../NetworkLibrary/DemoGame/config_files/entity_prefabs`.

A prefab have the following fields:
|**Name**|**Type**|**Required**|**Description**|
|---|---|---|---|
|name|String|Yes|The unique name of the prefab|
|archetype_name|String|Yes|The archetype name the prefab is associated with|
|component_configs|Objects Array|No|The configurations for the different components defined in the archetype. Component configurations are never mandatory and will be defaulted if there is not one defined here|
|children_prefab_names|Strings Array|No|The children prefab names that the prefab is parent of. During the prefab instantiation, all children prefabs will be created along with the parent prefab|

For exampple, the following code snippet represents the archetype of a wall, which contains three components:
```json
{
	"name": "Wall",
	"archetype": "wall",
	"component_configs":
	[
		{
			"name": "SpriteRenderer",
			"texture_path": "sprites/PlayerSprites/playerHead.png"
		},
		{
			"name": "Collider2D",
			"collision_response_type": "Static",
			"is_trigger": false,
			"bounds_config":
			{
				"type":"Circle",
				"radius":5
			}
		}
	]
}
```
For prefabs, the engine's JSON loader will also need to know how to parse each component configuration from its Json object to an instance of type `IComponentConfiguration`. Thgat means that, when creating a new component in the codebase devs will also need to create their associated configuration, inheriting from the `IComponentConfiguration` base struct and also the neccessary code within the JSON loader to parse a Json object to that configuration.