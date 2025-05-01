#pragma once
#include "ecs/i_simple_system.h"

#include "resource_handlers/gizmo_resource_handler.h"

#include "SDL_image.h"

#include <unordered_map>

#include "gizmos/Gizmo.h"

class GizmoRenderer;

class GizmoRendererSystem : public ECS::ISimpleSystem
{
	public:
		GizmoRendererSystem( SDL_Renderer* renderer );
		~GizmoRendererSystem();

		GizmoResourceHandler& GetGizmoResourceHandler() { return _gizmoResourceHandler; }

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void AllocateGizmoRendererComponent( ECS::GameEntity& entity );
		void DeallocateGizmoRendererComponent( ECS::GameEntity& entity );

	private:
		void InitGizmoRenderers();
		void DeallocateGizmoRenderers();

		SDL_Renderer* _renderer;
		GizmoResourceHandler _gizmoResourceHandler;

		std::unordered_map< GizmoType, GizmoRenderer* > _gizmoRenderers;
};
