#include "player_interpolated_position_and_rotation_applier.h"

#include "transform/transform_hierarchy_helper_functions.h"
#include "transform/transform_component.h"

#include "ecs/game_entity.hpp"

#include "client/player_interpolation/player_interpolated_state_component.h"

void PlayerInterpolationPositionAndRotationApplier::Execute( Engine::ECS::GameEntity& position_entity,
                                                             Engine::ECS::GameEntity& rotation_entity,
                                                             const PlayerInterpolatedState& state,
                                                             float32 elapsed_time )
{
	Engine::TransformComponentProxy transformComponentProxy;
	Engine::TransformComponent& interpolatedPositionTransform =
	    position_entity.GetComponent< Engine::TransformComponent >();
	Engine::TransformComponent& interpolatedRotationTransform =
	    rotation_entity.GetComponent< Engine::TransformComponent >();

	transformComponentProxy.SetGlobalPosition( interpolatedPositionTransform, state.position );
	transformComponentProxy.SetGlobalRotationAngle( interpolatedRotationTransform, state.rotationAngle );
}
