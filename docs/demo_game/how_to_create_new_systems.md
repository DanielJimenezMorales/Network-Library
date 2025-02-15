# Create new systems
Last time updated: 15/02/2025

## Introduction:
In this page we will discuss how to create and use new systems within the ECS architecture.

## Index:
- [Create the simple system](#create-the-simple-system)
- [Create the system coordinator](#create-the-system-coordinator)
- [Subscribe to events](#subscribe-to-events)

## Create the simple system
The first step is to implement the logic that the new system is going to have. In order to do that, we will need to create a new C++ class that implements the `ECS::ISimpleSystem` interface. The implementation requirements will be to fill out the following methods:
- **void ECS::ISimplySystem::Execute(ECS::EntityContainer& entity_container, float32 elapsed_time)**: In this method we will implement all the logic. `ECS::EntityContainer entity_container` will be the class where all the different global components and entities along with their components will live in. `float32 elapsed_time` is the time that has passed since the last time that method was called.

As an example, the following code snippet implements a system that gets all the entities with a `HealthComponent` and takes 10 points of health:
```cpp
class DamageEntitiesSystem : public ECS::ISimpleSystem
{
public:
	DamageEntitiesSystem() : ECS::ISimpleSystem() {}

	void Execute(ECS::EntityContainer& entity_container, float32 elapsed_time) override
	{
		//Get all damageable entities
		std::vector<ECS::GameEntity> entities = entity_container.GetEntitiesOfType<HealthComponent>();
		for(auto it = entities.cbegin(); it != entities.end(); ++it)
		{
			//Get Health component
			HealthComponent& health_cmp =it->GetComponent<HelathComponent>();
			//Decrease current health
			health_cmp.currentHealth = (health_cmp.currentHealth < 10) ? 0 : health_cmp.currentHealth - 10;
		}
	}
};
```

## Create the system coordinator
The next step will be instantiating a `ECS::SystemCoordinator` in order to add our simple system. The system coordinator will act as a wrapper containing an ordered list of one or more simple systems. Since this is a naive example, we would not be using all the benefits of the system coordinator. However, if we have two different simple systems that, within the same execution stage, we want one of them to execute before than the other one, we will have a system coordinator to register the simple systems in the desired execution order.

When creating a system coordinator, you will need to specify in which `ECS::ExecutionStage` its simple systems will be executed. The `ECS::ExecutionStage` is an enum that it could have values like UPDATE, PRETICK, TICK, POSTICK, RENDER...
> Note to myself: In order to increase the flexibility of this solution, consider making the `ECS::ExecutionStage` to be a string instead of an enum. In that way, devs can add new execution stages without needing to re-compile the API again.

The following code snippet will show how to create a system coordinator:
```cpp
ECS::SystemCoordinator* CreateHealthSystemCoordinator()
{
	DamageEntitiesSystem damage_entities_system = new DamageEntitiesSystem();

	//Create the system coordinator. For this example we will be telling the coordinator to execute during the Tick stage.
	ECS::SystemCoordinator* system_coordinator = new ECS::SystemCoordinator( ECS::ExecutionStage::TICK );

	//Configure the execution order of simple systems. In this case we only have one so there is no order.
	system_coordinator->AddSystemToTail( damage_entities_system );

	return system_coordinator;
}
```

## Subscribe to events
TODO