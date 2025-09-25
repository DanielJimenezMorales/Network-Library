# Prefab configuration fields
Last time updated: 16/02/2025

## Introduction:
In this page we will discuss how to create and use new systems within the ECS architecture.

## JSON fields:
|**Name**|**Type**|**Required**|**Description**|
|---|---|---|---|
|name|String|Yes|The name of the prefab|
|archetype_name|String|Yes|The name of the archetype associated to this prefab|
|component_configs|Objects Array|No|The configurations from the different components defined in the archetype. Component configurations are never mandatory and will be defaulted if there is not one defined here|
|children_prefab_names|Strings Array|No|The names of the children prefabs that this prefab is parent of. During the prefab instantiation, all children prefabs will be created along with the parent prefab|
