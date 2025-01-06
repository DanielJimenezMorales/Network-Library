#include "gizmo_renderer_system.h"

#include "ecs/entity_container.h"

#include "GameEntity.hpp"
#include "Gizmo.h"

#include "components/gizmo_renderer_component.h"
#include "components/transform_component.h"
#include "components/camera_component.h"

#include "components/collider_2d_component.h"

GizmoRendererSystem::GizmoRendererSystem( SDL_Renderer* renderer )
    : ECS::ISimpleSystem()
    , _renderer( renderer )
{
}

void GizmoRendererSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	const ECS::EntityContainer* entity_container = entity.GetEntityContainer();
	const CameraComponent& camera = entity_container->GetFirstComponentOfType< CameraComponent >();

	const TransformComponent& transform = entity.GetComponent< TransformComponent >();
	const GizmoRendererComponent& gizmo_renderer = entity.GetComponent< GizmoRendererComponent >();

	gizmo_renderer.gizmo->Render( camera, transform, _renderer );

	// TODO cache the color in a variable so I dont need to hardcode it in different places of the code
	SDL_SetRenderDrawColor( _renderer, 255, 0, 0, 255 );
}

void GizmoRendererSystem::AddGizmoRendererComponent( GameEntity& entity )
{
	if ( !entity.HasComponent< Collider2DComponent >() )
	{
		return;
	}

	const Collider2DComponent& collider = entity.GetComponent< Collider2DComponent >();
	Gizmo* gizmo = collider.GetGizmo();
	entity.AddComponent< GizmoRendererComponent >( gizmo );
}
