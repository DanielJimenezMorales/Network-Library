#pragma once
#include "numeric_types.h"

#include "SDL.h"

struct CameraComponent;
struct TransformComponent;

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

		void Configure( const GizmoConfiguration* configuration );
		void Render( const CameraComponent& camera, const TransformComponent& camera_transform,
		             const TransformComponent& transform, SDL_Renderer* renderer ) const;

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

		// TODO Decouple this function from here. I think it's better to create a BaseGizmoRenderer and a
		// ConcreteGizmoRenderer classes. This should only contain data in order to remove the SDL include
		virtual void RenderConcrete( const CameraComponent& camera, const TransformComponent& camera_transform,
		                             const TransformComponent& transform, SDL_Renderer* renderer ) const = 0;

	private:
		uint8 r, g, b, a;
		GizmoType _type;
};
