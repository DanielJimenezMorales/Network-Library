#include "PlayerControllerConfiguration.h"

PlayerControllerConfiguration::PlayerControllerConfiguration()
    : ECS::ComponentConfiguration( "Player" )
    , movementSpeed( 0.f )
{
}

PlayerControllerConfiguration::PlayerControllerConfiguration( uint32 movement_speed )
    : ECS::ComponentConfiguration( "PlayerController" )
    , movementSpeed( movement_speed )
{
}

PlayerControllerConfiguration* PlayerControllerConfiguration::Clone() const
{
	return new PlayerControllerConfiguration( *this );
}
