#include "coordinates_conversion_utils.h"

#include <cassert>

#include "ecs/game_entity.hpp"

#include "components/camera_component.h"

#include "read_only_transform_component_proxy.h"

namespace Engine
{
	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const ECS::GameEntity& camera_entity )
	{
		assert( camera_entity.HasComponent< CameraComponent >() );

		const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
		ReadOnlyTransformComponentProxy cameraTransform( camera_entity );

		return ConvertFromWorldPositionToScreenPosition( worldPosition, camera, cameraTransform );
	}

	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const CameraComponent& camera,
	                                                ReadOnlyTransformComponentProxy& camera_transform )
	{
		// Convert from world coordinates to uncentered screen coordinates
		worldPosition =
		    ( worldPosition - camera_transform.GetGlobalPosition() ) * CameraComponent::PIXELS_PER_WORLD_UNIT;

		// This is due to SDL2 tracks positive Y downward, so I need to invert it. I want positive Y to go up by
		// default
		worldPosition = Vec2f( worldPosition.X(), -worldPosition.Y() );

		// Center position based on windows size (SDL2's center of coordinates is the top left position of the
		// screen)
		worldPosition.AddToX( static_cast< float >( camera.width / 2 ) );
		worldPosition.AddToY( static_cast< float >( camera.height / 2 ) );
		return worldPosition;
	}

	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const ECS::GameEntity& camera_entity )
	{
		assert( camera_entity.HasComponent< CameraComponent >() );

		const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
		ReadOnlyTransformComponentProxy cameraTransform( camera_entity );

		return ConvertFromScreenPositionToWorldPosition( screenPosition, camera, cameraTransform );
	}

	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const CameraComponent& camera,
	                                                ReadOnlyTransformComponentProxy& camera_transform )
	{
		screenPosition.AddToX( -static_cast< float >( camera.width / 2 ) );
		screenPosition.AddToY( -static_cast< float >( camera.height / 2 ) );

		// This is due to SDL2 tracks positive Y downward, so I need to invert it. I want positive Y to go up by
		// default
		screenPosition = Vec2f( screenPosition.X(), -screenPosition.Y() );

		screenPosition /= CameraComponent::PIXELS_PER_WORLD_UNIT;
		const Vec2f camera_position = camera_transform.GetGlobalPosition();
		screenPosition.AddToX( camera_position.X() );
		screenPosition.AddToY( camera_position.Y() );
		return screenPosition;
	}
} // namespace Engine
