#include "gizmo.h"

namespace Engine
{
	void Gizmo::Configure( const GizmoConfiguration* configuration )
	{
		_color = configuration->color;
		_type = configuration->type;

		ConfigureConcrete( configuration );
	}

	Gizmo::Gizmo( const Color& color, GizmoType type )
	    : _color( color )
	    , _type( type )
	{
	}
}
