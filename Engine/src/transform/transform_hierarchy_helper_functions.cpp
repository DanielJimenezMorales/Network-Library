#include "transform_hierarchy_helper_functions.h"

#include "components/transform_component.h"

#include "math_utils.h"

namespace Engine
{
	Vec2f Engine::TransformHierarchyHelperFunctions::GetGlobalPosition( const TransformComponent& transform ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		return transform._position;
	}

	void TransformHierarchyHelperFunctions::SetGlobalPosition( TransformComponent& transform,
	                                                           const Vec2f& new_position ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		transform._position = new_position;
		SetChildrenDirty( transform );
	}

	Vec2f TransformHierarchyHelperFunctions::GetLocalPosition( const TransformComponent& transform ) const
	{
		Vec2f result;
		if ( HasParent( transform ) )
		{
			result = transform._localPosition;
		}
		else
		{
			result = transform._position;
		}

		return result;
	}

	void TransformHierarchyHelperFunctions::SetLocalPosition( TransformComponent& transform,
	                                                          const Vec2f& new_local_position ) const
	{
		if ( HasParent( transform ) )
		{
			transform._localPosition = new_local_position;
			transform._isDirty = true;
		}
		else
		{
			transform._position = new_local_position;
		}

		SetChildrenDirty( transform );
	}

	float32 TransformHierarchyHelperFunctions::GetGlobalRotation( const TransformComponent& transform ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		return transform._rotationAngle;
	}

	void TransformHierarchyHelperFunctions::SetGlobalRotationAngle( TransformComponent& transform,
	                                                                float32 new_angle ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		transform._rotationAngle = std::fmodf( new_angle, 360.0f );
		if ( transform._rotationAngle < 0.f )
		{
			transform._rotationAngle += 360.f; // Ensure the angle is always positive
		}

		SetChildrenDirty( transform );
	}

	float32 TransformHierarchyHelperFunctions::GetLocalRotationAngle( const TransformComponent& transform ) const
	{
		float32 result;
		if ( HasParent( transform ) )
		{
			result = transform._localRotationAngle;
		}
		else
		{
			result = transform._rotationAngle;
		}

		return result;
	}

	void TransformHierarchyHelperFunctions::SetLocalRotationAngle( TransformComponent& transform,
	                                                               float32 new_local_angle ) const
	{
		if ( HasParent( transform ) )
		{
			transform._localRotationAngle = std::fmodf( new_local_angle, 360.0f );
			if ( transform._localRotationAngle < 0.f )
			{
				transform._localRotationAngle += 360.f; // Ensure the angle is always positive
			}

			transform._isDirty = true;
		}
		else
		{
			transform._rotationAngle = std::fmodf( new_local_angle, 360.0f );
			if ( transform._rotationAngle < 0.f )
			{
				transform._rotationAngle += 360.f; // Ensure the angle is always positive
			}
		}

		SetChildrenDirty( transform );
	}

	void TransformHierarchyHelperFunctions::SetRotationLookAt( TransformComponent& transform,
	                                                           Vec2f look_at_direction ) const
	{
		look_at_direction.Normalize();

		SetGlobalRotationAngle( transform, ConvertNormalizedDirectionToAngle( look_at_direction ) );
	}

	void TransformHierarchyHelperFunctions::LookAt( TransformComponent& transform, const Vec2f& position ) const
	{
		const Vec2f direction = position - GetGlobalPosition( transform );
		if ( direction == Vec2f( 0, 0 ) )
		{
			return;
		}

		const Vec2f forward = GetForwardVector( transform );

		// FORMULA of angle between two vectors: Cos(angle) = dotProduct(V1, V1) / (Mag(V1) * Mag(V2)) --> angle =
		// arcos(dotProduct(V1, V1) / (Mag(V1) * Mag(V2)))

		const float32 dotProduct = ( direction.X() * forward.X() ) + ( direction.Y() * forward.Y() );
		float32 angleCosine = dotProduct / ( direction.Magnitude() * forward.Magnitude() );

		// Check it because due to floating point precision error, it could happen.
		if ( angleCosine > 1.f )
		{
			angleCosine = 1.f;
		}
		else if ( angleCosine < -1.f )
		{
			angleCosine = -1.f;
		}

		float32 angleInRadians = std::acosf( angleCosine );

		// Calculate rotation direction. Since our system has an anti-clockwise rotation direction, we need to invert
		// the angle in case the cross product is positive.
		const float32 crossProduct = ( direction.X() * forward.Y() ) - ( forward.X() * direction.Y() );
		if ( crossProduct > 0.f )
		{
			angleInRadians = -angleInRadians;
		}

		const float32 angleInDegrees = angleInRadians * ( 180.f / PI );

		SetGlobalRotationAngle( transform, GetGlobalRotation( transform ) + angleInDegrees );
	}

	Vec2f TransformHierarchyHelperFunctions::GetForwardVector( const TransformComponent& transform ) const
	{
		return ConvertAngleToNormalizedDirection( GetGlobalRotation( transform ) );
	}

	Vec2f TransformHierarchyHelperFunctions::GetGlobalScale( const TransformComponent& transform ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		return transform._scale;
	}

	void TransformHierarchyHelperFunctions::SetGlobalScale( TransformComponent& transform,
	                                                        const Vec2f& new_scale ) const
	{
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		transform._scale = new_scale;
		SetChildrenDirty( transform );
	}

	Vec2f TransformHierarchyHelperFunctions::GetLocalScale( const TransformComponent& transform ) const
	{
		Vec2f result;
		if ( HasParent( transform ) )
		{
			result = transform._localScale;
		}
		else
		{
			result = transform._scale;
		}

		return result;
	}

	void TransformHierarchyHelperFunctions::SetLocalScale( TransformComponent& transform,
	                                                       const Vec2f& new_local_scale ) const
	{
		if ( HasParent( transform ) )
		{
			transform._localScale = new_local_scale;
			transform._isDirty = true;
		}
		else
		{
			transform._localScale = new_local_scale;
		}

		SetChildrenDirty( transform );
	}

	void TransformHierarchyHelperFunctions::RemoveParent( TransformComponent& transform,
	                                                      ECS::GameEntity& transform_entity ) const
	{
		assert( transform_entity.IsValid() );

		if ( transform._parent.IsValid() )
		{
			ResolveDirty( transform );

			// Remove child from parent
			TransformComponent& parentTransform = transform._parent.GetComponent< TransformComponent >();
			auto cit = parentTransform._children.cbegin();
			for ( ; cit != parentTransform._children.cend(); ++cit )
			{
				if ( *cit == transform_entity )
				{
					break;
				}
			}

			assert( cit != parentTransform._children.cend() );
			parentTransform._children.erase( cit );

			// Remove parent from current
			transform._parent = ECS::GameEntity();

			// Reset local transform to default values
			transform._localPosition = Vec2f( 0.f, 0.f );
			transform._localRotationAngle = 0.f;
			transform._localScale = Vec2f( 1.f, 1.f );

			transform._isDirty = false;
		}
	}

	void TransformHierarchyHelperFunctions::SetParent( TransformComponent& transform, ECS::GameEntity& transform_entity,
	                                                   ECS::GameEntity& parent_entity ) const
	{
		assert( transform_entity.IsValid() );
		assert( parent_entity.IsValid() );
		assert( parent_entity.HasComponent< TransformComponent >() );

		// If it is dirty, resolve it before removing the parent in order to have the updated values
		if ( transform._isDirty )
		{
			ResolveDirty( transform );
		}

		// Remove the parent
		if ( transform._parent.IsValid() )
		{
			RemoveParent( transform, transform_entity );
		}

		// Set current as child parent
		TransformComponent& parentTransform = parent_entity.GetComponent< TransformComponent >();
		parentTransform._children.push_back( transform_entity );

		// Set parent as current parent
		transform._parent = parent_entity;

		// If parent is dirty, resolve it
		if ( parentTransform._isDirty )
		{
			ResolveDirty( parentTransform );
		}

		// Calculate local transform based on current global transform and parent's global transform
		transform._localPosition = GetGlobalPosition( transform ) - GetGlobalPosition( parentTransform );
		transform._localRotationAngle = GetGlobalRotation( transform ) - GetGlobalRotation( parentTransform );
		transform._localScale = GetGlobalScale( transform ) - GetGlobalScale( parentTransform );
	}

	bool TransformHierarchyHelperFunctions::HasParent( const TransformComponent& transform ) const
	{
		return transform._parent.IsValid();
	}

	bool TransformHierarchyHelperFunctions::HasChildren( const TransformComponent& transform ) const
	{
		return !transform._children.empty();
	}

	std::vector< ECS::GameEntity > TransformHierarchyHelperFunctions::GetChildren(
	    const TransformComponent& transform ) const
	{
		return transform._children;
	}

	void TransformHierarchyHelperFunctions::SetChildrenDirty( const TransformComponent& transform ) const
	{
		auto childrenIt = transform._children.begin();
		for ( ; childrenIt != transform._children.end(); ++childrenIt )
		{
			const TransformComponent& childTransform = childrenIt->GetComponent< TransformComponent >();
			childTransform._isDirty = true;
			if ( childTransform._children.size() > 0 )
			{
				SetChildrenDirty( childTransform );
			}
		}
	}

	void TransformHierarchyHelperFunctions::ResolveDirty( const TransformComponent& transform ) const
	{
		if ( !transform._isDirty || !HasParent( transform ) )
		{
			return;
		}

		// Resolve parent's dirty state first
		TransformComponent parentTransform = transform._parent.GetComponent< TransformComponent >();
		if ( IsDirty( parentTransform ) )
		{
			ResolveDirty( parentTransform );
		}

		// Recalculate global transform based on local transform and parent's global transform
		transform._position = GetGlobalPosition( parentTransform ) + GetLocalPosition( transform );
		transform._rotationAngle = GetGlobalRotation( parentTransform ) + GetLocalRotationAngle( transform );
		transform._scale = GetGlobalScale( parentTransform ) * GetLocalScale( transform );
		transform._isDirty = false;
	}

	bool TransformHierarchyHelperFunctions::IsDirty( const TransformComponent& transform ) const
	{
		return transform._isDirty;
	}
} // namespace Engine