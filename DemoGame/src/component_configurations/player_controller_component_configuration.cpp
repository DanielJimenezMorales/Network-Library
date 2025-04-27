#include "player_controller_component_configuration.h"

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration()
    : ECS::ComponentConfiguration( "Player" )
    , movementSpeed( 0 )
    , fireRatePerSecond( 0 )
{
}

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration( uint32 movement_speed,
                                                                                uint32 fire_rate_per_second )
    : ECS::ComponentConfiguration( "PlayerController" )
    , movementSpeed( movement_speed )
    , fireRatePerSecond( fire_rate_per_second )
{
}

PlayerControllerComponentConfiguration* PlayerControllerComponentConfiguration::Clone() const
{
	return new PlayerControllerComponentConfiguration( *this );
}
