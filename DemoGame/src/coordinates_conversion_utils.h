#pragma once
#include "Vec2f.h"

class GameEntity;
struct CameraComponent;
struct TransformComponent;

Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const GameEntity& camera_entity );
Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const CameraComponent& camera,
                                                const TransformComponent& camera_transform );
Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const GameEntity& camera_entity );
Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const CameraComponent& camera,
                                                const TransformComponent& camera_transform );
