# Transform Module
The transform module handles **entity position, rotation, and scale** in 2D.
It supports:
- Parent-child hierarchies
- Local and global transform distinction
- On demand update system

## Index:
- [Representation](#Representation)
- [Parent-Child Hierarchies](#Parent-Child-Hierarchies)
- [Local vs Global Transforms](#Local-vs-Global-Transforms)
- [Transform Component Proxies](#Transform-Component-Proxies)
- [On Demand Hierarchy Updates Propagation](#On-Demand-Hierarchy-Updates-Propagation)

## Representation
- **Position**: Stored as a 2D vector (X, Y) (float values)
- **Rotation**: Stored as an angle in degrees, ranging from 0 to 360 (float value)
- **Scale**: Stored as a 2D vector (X, Y) (float values)

## Parent-Child Hierarchies
In many cases, entities are not isolated. They exist in **hierarchies**, where one entity (the *parent*) influences the transform of another (the *child*). This is the foundation for building structured scenes like characters holding weapons, vehicles with wheels.

### How does it work
An entity transform component can optionally have a parent entity and an array of children entities. When an entity has no parent attached it is called **root entity**

This means that:
- Moving, rotation or scaling the parent entity automatically affects all of its children and the children of the children and so on.
- Modifying a child's local transform keeps it positioned relative to the parent.

### Features supported
- **Set a new parent**: Attach an entity to be the paent of another.
- **Remove a parent**: Detach an entity's parent, making it a root entity.
- **Check if has a parent or children**

## Local vs Global Transforms
When working with the transform component of an entity, it’s important to distinguish between **local** and **global** transforms. These define how an entity is positioned, rotated, and scaled, either relative to its parent or in world-space.

### Local Transform
- Represents the **position, rotation, and scale relative to the entity’s parent**.
- If the entity is a root one (has **no parent**), its local transform is identical to its global transform.
- Useful for building hierarchies where child entities should move and rotate along with their parent.

### Global Transform
- Represents the **absolute position, rotation, and scale in world-space**.
- It is calculated as the entity’s local transform plus its parent’s global transform.

## Transform Component Proxies
The `TransformComponent` is the only component intentionally restricted to only a few limited classes, called proxies.

### Types of Transform Proxies
If developers want to interact with transforms they need to use one of the following classes:
- **TransformComponentProxy**: It allows querying and modifying the different properties of the `TransformComponent`.
- **ReadOnlyTransformComponentProxy**: It only allows querying the different properties of the `TransformComponent` but not modifying them.

Any of the classes above can't be stored as a member variable as the entity can be removed or the transform component be re-allocated elsewhere. If developers need to use it multiple times within different functions of a class or module consider querying it multiple times.

### Purpose of Transform Proxies
The reason why the `TransformComponent` needs to be consumed through a proxy class is because of the features it provides:
- **On demand propagation of updates through the hierarchy**: Transform hierarchies are not updated right after being changed but in a lazy way. This solution is explained in a later section.
- **Safety on its hierarchy operations**: The proxy adds a few checks to avoid any potential issue from devs manipulating the raw parent and children values.

### Example of use
Everytime we want to query or modify any properties from an entity's `TransformComponent` the following needs to be done:
```cpp
using namespace Engine;

//Get the entity you want to consume its TransformComponent
ECS::GameEntity entity = world.GetFirstEntityOfType<TransformComponent>();

// Create a proxy based on the entity
TransformComponentProxy transformProxy(entity);

// Now we can use it
transformProxy.RemoveParent();
transformProxy.SetGlobalPosition(Vec2f(20.f, 3.f));
```

If a constant `GameEntity` is passed because only access is granted, we can use the `ReadOnlyTransformComponentProxy` as follow
```cpp
using namespace Engine;

//Get the entity you want to consume its TransformComponent
const ECS::GameEntity entity = world.GetFirstEntityOfType<TransformComponent>();

// Create a proxy based on the entity
ReadOnlyTransformComponentProxy transformProxy(entity);

// Now we can use it
bool hasParent = transformProxy.HasParent();
float32 localRot = transformProxy.GetLocalRotationAngle();
```

## On Demand Hierarchy Updates Propagation
In complex scenes, especially with entities with a deep hierarchy, updating all transforms every frame can be expensive. To optimize this, the transform module follows a **lazy propagation technique** using **dirty flags**. In that way values are only updated on demand when they are being queried or modified.

### How does it work
1. **Dirty Flag per Entity**
	- Each entity's transform component has a dirty flag indicating whether its global transform needs recalculation.
	- Flags are set automatically when:
		- The entity's global transform changes.
		- The entity is reparented.
		- A parent's transform (local and global) changes.
2. **Lazy evaluation**
	- Global transforms are not recalculated inmediately when a change occurs. Instead, the proxy recalculates them only when they are accessed for the next time or right before reparenting in order to avoid loosing the previous parent changes.
3. **Hierarchical Propagation**
	- When a parent is dirty, all children are marked dirty recursively.
	- When a child's global transform is accessed through the proxy
		- The proxy checks the entity's dirty flag.
		- If it is dirty, it recomputes the global position and rotation recursively as `Global(child) = Global(parent) + Local(child)`, except for the scale which is `Global(child) = Global(parent) * Local(child)`.
		- Then clears the dirty flag for that entity.

### Benefits
- **Efficiency**: Only entities that actually need updated transforms are recalculated, and only when it is required.
- **Consistency**: Children always reflect their parent's latest transform changes whenever they are accessed.
- **Scalability**: Deep hierarchies do not incur unnecessary computation every frame.