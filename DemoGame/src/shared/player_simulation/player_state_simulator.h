#pragma once
#include "numeric_types.h"

#include "shared/player_simulation/player_movement_controller.h"
#include "shared/player_simulation/player_rotation_controller.h"
#include "shared/player_simulation/player_shooting_controller.h"

namespace ECS
{
	class World;
	class GameEntity;
}

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerStateSimulator
{
	public:
		PlayerStateSimulator();

		PlayerState Simulate( const InputState& inputs, const PlayerState& current_state,
		                      const PlayerStateConfiguration& configuration, float32 elapsed_time );

		template < typename Functor >
		Common::Delegate<>::SubscriptionHandler SubscribeToOnShotPerformed( Functor&& functor );
		bool UnsubscribeFromOnShotPerformed( const Common::Delegate<>::SubscriptionHandler& handler );

	private:
		PlayerMovementController _movementController;
		PlayerRotationController _rotationController;
		PlayerShootingController _shootingController;
};

template < typename Functor >
inline Common::Delegate<>::SubscriptionHandler PlayerStateSimulator::SubscribeToOnShotPerformed( Functor&& functor )
{
	return _shootingController.OnShotPerformed.AddSubscriber( std::forward< Functor >( functor ) );
}
