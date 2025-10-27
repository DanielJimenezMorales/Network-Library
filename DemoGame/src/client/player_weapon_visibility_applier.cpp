#include "player_weapon_visibility_applier.h"

#include "ecs/game_entity.hpp"

#include "render/sprite_renderer_component.h"

#include "client/components/player_interpolated_state_component.h"

void PlayerWeaponVisibilityApplier::Execute( Engine::ECS::GameEntity& entity, const PlayerInterpolatedState& state,
                                             float32 elapsed_time )
{
	Engine::SpriteRendererComponent& spriteRenderer = entity.GetComponent< Engine::SpriteRendererComponent >();
	spriteRenderer.isDisabled = !state.isAiming;
}
