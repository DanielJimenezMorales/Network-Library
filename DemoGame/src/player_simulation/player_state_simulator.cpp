#include "player_state_simulator.h"

#include "ecs/world.h"

#include "InputState.h"

#include "player_simulation/player_state.h"

PlayerStateSimulator::PlayerStateSimulator()
    : _movementController()
    , _rotationController()
    , _shootingController()
{
}

void PlayerStateSimulator::Configure( const ECS::World* world, const ECS::GameEntity& player_entity )
{
}

PlayerState PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state, const PlayerStateConfiguration& configuration,
                                     float32 elapsed_time )
{
	PlayerState playerState;

	_movementController.Simulate( inputs, current_state, playerState, elapsed_time, configuration );
	_rotationController.Simulate( inputs, current_state, playerState, elapsed_time, configuration );
	_shootingController.Simulate( inputs, current_state, playerState, elapsed_time, configuration );

	return playerState;
}

void PlayerStateSimulator::UnsubscribeFromOnShotPerformed( uint32 id )
{
	_shootingController.OnShotPerformed.DeleteSubscriber( id );
}
