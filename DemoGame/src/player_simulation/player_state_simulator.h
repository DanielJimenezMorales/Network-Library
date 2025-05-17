#pragma once
#include "numeric_types.h"

#include "player_simulation/player_movement_controller.h"
#include "player_simulation/player_rotation_controller.h"
#include "player_simulation/player_shooting_controller.h"

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
		uint32 SubscribeToOnShotPerformed( Functor&& functor );
		void UnsubscribeFromOnShotPerformed( uint32 id );

	private:
		PlayerMovementController _movementController;
		PlayerRotationController _rotationController;
		PlayerShootingController _shootingController;
};

template < typename Functor >
inline uint32 PlayerStateSimulator::SubscribeToOnShotPerformed( Functor&& functor )
{
	return _shootingController.OnShotPerformed.AddSubscriber( std::forward< Functor >( functor ) );
}
