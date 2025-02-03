#include "player_controller_component_configuration.h"

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration()
    : ECS::ComponentConfiguration( "Player" )
    , movementSpeed( 0.f )
{
}

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration( uint32 movement_speed )
    : ECS::ComponentConfiguration( "PlayerController" )
    , movementSpeed( movement_speed )
{
}

PlayerControllerComponentConfiguration* PlayerControllerComponentConfiguration::Clone() const
{
	return new PlayerControllerComponentConfiguration( *this );
}
