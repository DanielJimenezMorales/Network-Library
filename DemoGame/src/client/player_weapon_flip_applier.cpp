#include "player_weapon_flip_applier.h"

#include "ecs/game_entity.hpp"

#include "render/sprite_renderer_component.h"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "math_utils.h"

#include "client/components/player_visual_weapon_tag_component.h"
#include "client/components/player_interpolated_state_component.h"

void PlayerWeaponFlipApplier::Execute( Engine::ECS::GameEntity& entity, const PlayerInterpolatedState& state,
                                       float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformComponentProxy;

	Engine::SpriteRendererComponent& spriteRenderer = entity.GetComponent< Engine::SpriteRendererComponent >();

	const Engine::TransformComponent transform( state.position, state.rotationAngle );
	const Vec2f forwardDirection = transformComponentProxy.GetForwardVector( transform );

	spriteRenderer.flipY = forwardDirection.X() < 0.f;
}
