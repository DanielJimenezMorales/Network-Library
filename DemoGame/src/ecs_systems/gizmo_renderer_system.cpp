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

void GizmoRendererSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	const GameEntity& camera_entity = entity_container.GetFirstEntityOfType< CameraComponent >();
	const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
	const TransformComponent& camera_transform = camera_entity.GetComponent< TransformComponent >();

	std::vector< GameEntity > entities =
	    entity_container.GetEntitiesOfBothTypes< GizmoRendererComponent, TransformComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		const TransformComponent& transform = it->GetComponent< TransformComponent >();
		const GizmoRendererComponent& gizmo_renderer = it->GetComponent< GizmoRendererComponent >();

		const Gizmo* gizmo = _gizmoResourceHandler.TryGetGizmoFromHandler( gizmo_renderer.gizmoHandler );
		if ( gizmo == nullptr )
		{
			continue;
		}

		gizmo->Render( camera, camera_transform, transform, _renderer );

		// TODO cache the color in a variable so I dont need to hardcode it in different places of the code
		SDL_SetRenderDrawColor( _renderer, 255, 0, 0, 255 );
	}
}

void GizmoRendererSystem::AllocateGizmoRendererComponentIfHasCollider( GameEntity& entity )
{
	if ( !entity.HasComponent< Collider2DComponent >() )
	{
		return;
	}

	const Collider2DComponent& collider = entity.GetComponent< Collider2DComponent >();
	const GizmoHandler gizmo_handler = _gizmoResourceHandler.CreateGizmo( collider.GetGizmo().get() );
	entity.AddComponent< GizmoRendererComponent >( gizmo_handler );
}

void GizmoRendererSystem::DeallocateGizmoRendererComponentIfHasCollider( GameEntity& entity )
{
	if ( !entity.HasComponent< Collider2DComponent >() )
	{
		return;
	}

	GizmoRendererComponent& gizmo_renderer = entity.GetComponent< GizmoRendererComponent >();
	const bool remove_successfully = _gizmoResourceHandler.RemoveGizmo( gizmo_renderer.gizmoHandler );
	assert( remove_successfully );
}
