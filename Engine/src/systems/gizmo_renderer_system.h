#pragma once
#include "ecs/i_simple_system.h"

#include "resource_handlers/gizmo_resource_handler.h"

#include <unordered_map>

#include "gizmos/gizmo.h"

namespace Engine
{
	class GizmoRenderer;

	class GizmoRendererSystem : public ECS::ISimpleSystem
	{
		public:
			GizmoRendererSystem();
			~GizmoRendererSystem();

			GizmoResourceHandler& GetGizmoResourceHandler() { return _gizmoResourceHandler; }

			void Execute( ECS::World& world, float32 elapsed_time ) override;

			void AllocateGizmoRendererComponent( ECS::GameEntity& entity );
			void DeallocateGizmoRendererComponent( ECS::GameEntity& entity );

		private:
			void InitGizmoRenderers();
			void DeallocateGizmoRenderers();

			GizmoResourceHandler _gizmoResourceHandler;

			std::unordered_map< GizmoType, GizmoRenderer* > _gizmoRenderers;
	};
}
