#include "coordinates_conversion_utils.h"

#include <cassert>

#include "GameEntity.hpp"

#include "components/transform_component.h"
#include "components/camera_component.h"

Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const ECS::GameEntity& camera_entity )
{
	assert( camera_entity.HasComponent< CameraComponent >() );
	assert( camera_entity.HasComponent< TransformComponent >() );

	const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
	const TransformComponent& camera_transform = camera_entity.GetComponent< TransformComponent >();

	return ConvertFromWorldPositionToScreenPosition( worldPosition, camera, camera_transform );
}

Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const CameraComponent& camera,
                                                const TransformComponent& camera_transform )
{
	// Convert from world coordinates to uncentered screen coordinates
	worldPosition = ( worldPosition - camera_transform.GetPosition() ) * CameraComponent::PIXELS_PER_WORLD_UNIT;

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
	assert( camera_entity.HasComponent< TransformComponent >() );

	const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
	const TransformComponent& camera_transform = camera_entity.GetComponent< TransformComponent >();

	return ConvertFromScreenPositionToWorldPosition( screenPosition, camera, camera_transform );
}

Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const CameraComponent& camera,
                                                const TransformComponent& camera_transform )
{
	screenPosition.AddToX( -static_cast< float >( camera.width / 2 ) );
	screenPosition.AddToY( -static_cast< float >( camera.height / 2 ) );

	// This is due to SDL2 tracks positive Y downward, so I need to invert it. I want positive Y to go up by
	// default
	screenPosition = Vec2f( screenPosition.X(), -screenPosition.Y() );

	screenPosition /= CameraComponent::PIXELS_PER_WORLD_UNIT;
	const Vec2f camera_position = camera_transform.GetPosition();
	screenPosition.AddToX( camera_position.X() );
	screenPosition.AddToY( camera_position.Y() );
	return screenPosition;
}
