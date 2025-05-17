#pragma once
#include "ecs/i_simple_system.h"

/// <summary>
/// This system is client-side only.
/// It reconciles the local player state with the server state.
/// </summary>
class ClientLocalPlayerServerReconciliatorSystem : public ECS::ISimpleSystem
{
	public:
		ClientLocalPlayerServerReconciliatorSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;
};
