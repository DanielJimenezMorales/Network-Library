#include "gizmo_renderer_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "gizmos/ray_gizmo.h"
#include "gizmos/circle_gizmo_renderer.h"
#include "gizmos/ray_gizmo_renderer.h"

#include "global_components/render_global_component.h"

#include "components/gizmo_renderer_component.h"
#include "components/camera_component.h"
#include "components/transform_component.h"

#include "components/collider_2d_component.h"
#include "components/raycast_component.h"

namespace Engine
{
	GizmoRendererSystem::GizmoRendererSystem()
	    : ECS::ISimpleSystem()
	    , _gizmoRenderers()
	{
		InitGizmoRenderers();
	}

	GizmoRendererSystem::~GizmoRendererSystem()
	{
		DeallocateGizmoRenderers();
	}

	void GizmoRendererSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		RenderGlobalComponent& render_global_component = world.GetGlobalComponent< RenderGlobalComponent >();

		const ECS::GameEntity& camera_entity = world.GetFirstEntityOfType< CameraComponent >();
		const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
		const TransformComponent& cameraTransform = camera_entity.GetComponent< TransformComponent >();

		std::vector< ECS::GameEntity > entities =
		    world.GetEntitiesOfBothTypes< GizmoRendererComponent, TransformComponent >();
		for ( auto it = entities.begin(); it != entities.end(); ++it )
		{
			const TransformComponent& transform = it->GetComponent< TransformComponent >();
			const GizmoRendererComponent& gizmo_renderer = it->GetComponent< GizmoRendererComponent >();

			const Gizmo* gizmo = _gizmoResourceHandler.TryGetGizmoFromHandler( gizmo_renderer.gizmoHandler );
			if ( gizmo == nullptr )
			{
				continue;
			}

			auto gizmo_renderer_found = _gizmoRenderers.find( gizmo->GetType() );
			if ( gizmo_renderer_found != _gizmoRenderers.end() )
			{
				const GizmoRenderer* gizmo_renderer = gizmo_renderer_found->second;
				assert( gizmo_renderer != nullptr );
				gizmo_renderer->Render( *gizmo, camera, cameraTransform, transform, render_global_component.renderer );
			}

			// TODO cache the color in a variable so I dont need to hardcode it in different places of the code
			SDL_SetRenderDrawColor( render_global_component.renderer, 255, 0, 0, 255 );
		}
	}

	void GizmoRendererSystem::AllocateGizmoRendererComponent( ECS::GameEntity& entity )
	{
		LOG_WARNING( "ALLOCATE GIZMO!" );
		const bool has_raycast = entity.HasComponent< RaycastComponent >();
		const bool has_collider = entity.HasComponent< RaycastComponent >();
		if ( entity.HasComponent< RaycastComponent >() )
		{
			const RaycastComponent& raycast = entity.GetComponent< RaycastComponent >();
			RayGizmoConfiguration ray_gizmo_config;
			ray_gizmo_config.type = GizmoType::RAY;
			ray_gizmo_config.length = raycast.distance;

			const GizmoHandler gizmo_handler = _gizmoResourceHandler.CreateGizmo( &ray_gizmo_config );
			entity.AddComponent< GizmoRendererComponent >( gizmo_handler );
		}
		else if ( entity.HasComponent< Collider2DComponent >() )
		{
			const Collider2DComponent& collider = entity.GetComponent< Collider2DComponent >();
			const GizmoHandler gizmo_handler = _gizmoResourceHandler.CreateGizmo( collider.GetGizmo().get() );
			entity.AddComponent< GizmoRendererComponent >( gizmo_handler );
		}
		else
		{
			return;
		}
	}

	void GizmoRendererSystem::DeallocateGizmoRendererComponent( ECS::GameEntity& entity )
	{
		if ( !entity.HasComponent< GizmoRendererComponent >() )
		{
			return;
		}

		const GizmoRendererComponent& gizmo_renderer = entity.GetComponent< GizmoRendererComponent >();
		const bool remove_successfully = _gizmoResourceHandler.RemoveGizmo( gizmo_renderer.gizmoHandler );
		assert( remove_successfully );
	}

	void GizmoRendererSystem::InitGizmoRenderers()
	{
		_gizmoRenderers[ GizmoType::RAY ] = new RayGizmoRenderer();
		_gizmoRenderers[ GizmoType::CIRCLE2D ] = new CircleGizmoRenderer();
	}

	void GizmoRendererSystem::DeallocateGizmoRenderers()
	{
		auto it = _gizmoRenderers.begin();
		for ( ; it != _gizmoRenderers.end(); ++it )
		{
			if ( it->second != nullptr )
			{
				delete it->second;
				it->second = nullptr;
			}
		}

		_gizmoRenderers.clear();
	}
} // namespace Engine
