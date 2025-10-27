#include "player_controller_component_configuration.h"

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration()
    : Engine::ECS::ComponentConfiguration( "Player" )
    , movementSpeed( 0 )
    , aimingMovementSpeedMultiplier( 0.f )
    , fireRatePerSecond( 0 )
{
}

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration(
    uint32 movement_speed, float32 aiming_movement_speed_multiplier, uint32 fire_rate_per_second )
    : Engine::ECS::ComponentConfiguration( "PlayerController" )
    , movementSpeed( movement_speed )
    , aimingMovementSpeedMultiplier( aiming_movement_speed_multiplier )
    , fireRatePerSecond( fire_rate_per_second )
{
}

PlayerControllerComponentConfiguration* PlayerControllerComponentConfiguration::Clone() const
{
	return new PlayerControllerComponentConfiguration( *this );
}
