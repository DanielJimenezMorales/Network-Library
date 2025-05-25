#pragma once
#include <cstdint>

struct NetworkEntityComponent
{
		NetworkEntityComponent()
		    : networkEntityId( 0 )
		    , controlledByPeerId( 0 )
		{
		}

		NetworkEntityComponent( uint32 entityId, uint32 controlledByPeerId )
		    : networkEntityId( entityId )
		    , controlledByPeerId( controlledByPeerId )
		{
		}

		uint32 networkEntityId;
		uint32 controlledByPeerId;
};
