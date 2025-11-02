#include "player_controller_component_configuration.h"

PlayerControllerComponentConfiguration::PlayerControllerComponentConfiguration()
    : Engine::ECS::ComponentConfiguration( "PlayerController" )
{
}

PlayerControllerComponentConfiguration* PlayerControllerComponentConfiguration::Clone() const
{
	return new PlayerControllerComponentConfiguration( *this );
}
