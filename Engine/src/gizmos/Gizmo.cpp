#include "gizmo.h"

namespace Engine
{
	void Gizmo::Configure( const GizmoConfiguration* configuration )
	{
		r = configuration->r;
		g = configuration->g;
		b = configuration->b;
		a = configuration->a;
		_type = configuration->type;

		ConfigureConcrete( configuration );
	}

	Gizmo::Gizmo( uint8 r, uint8 g, uint8 b, uint8 a, GizmoType type )
	    : r( r )
	    , g( g )
	    , b( b )
	    , a( a )
	    , _type( type )
	{
	}
}
