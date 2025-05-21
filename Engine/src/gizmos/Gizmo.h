#pragma once
#include "numeric_types.h"

enum class GizmoType : uint32
{
	NONE = 0,
	CIRCLE2D = 1,
	RAY = 2
};

struct GizmoConfiguration
{
		virtual ~GizmoConfiguration() {}

		uint8 r, g, b, a;
		GizmoType type;
};

class Gizmo
{
	public:
		virtual ~Gizmo() {}

		Gizmo& operator=( const Gizmo& other ) = default;
		Gizmo& operator=( Gizmo&& other ) noexcept = default;

		GizmoType GetType() const { return _type; }
		uint8 GetR() const { return r; }
		uint8 GetG() const { return g; }
		uint8 GetB() const { return b; }
		uint8 GetA() const { return a; }

		void Configure( const GizmoConfiguration* configuration );

		virtual Gizmo* Clone() const = 0;

	protected:
		Gizmo( const GizmoConfiguration* configuration )
		    : r( configuration->r )
		    , g( configuration->g )
		    , b( configuration->b )
		    , a( configuration->a )
		    , _type( configuration->type )
		{
		}

		Gizmo( uint8 r, uint8 g, uint8 b, uint8 a, GizmoType type );
		Gizmo( const Gizmo& other ) = default;
		Gizmo( Gizmo&& other ) noexcept = default;

		virtual void ConfigureConcrete( const GizmoConfiguration* configuration ) = 0;

	private:
		uint8 r, g, b, a;
		GizmoType _type;
};
