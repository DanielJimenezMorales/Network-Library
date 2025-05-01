#include "gtest/gtest.h"

#include "raycaster.h"

#include "ecs/world.h"

#include "components/transform_component.h"
#include "components/collider_2d_component.h"

#include "CircleBounds2D.h"

namespace
{
	class RaycastAgainstCircleDoesCollideParams : public ::testing::TestWithParam< std::tuple< Vec2f, Vec2f, Vec2f > >
	{
	};

	TEST_P( RaycastAgainstCircleDoesCollideParams, RaycastAgainstCircleDoesCollide )
	{
		// Init Parameters
		Vec2f ray_origin;
		Vec2f ray_direction;
		Vec2f circle_origin;
		std::tie( ray_origin, ray_direction, circle_origin ) = GetParam();

		using namespace Raycaster;

		// Arrange
		const Ray ray{ ray_origin, ray_direction, 100 };
		RaycastResult result;

		ECS::World world;
		world.RegisterComponent< TransformComponent >( "Transform" );
		world.RegisterComponent< Collider2DComponent >( "Collider2D" );

		ECS::Archetype archetype;
		archetype.name = "entity_with_collider";
		archetype.components.push_back( "Transform" );
		archetype.components.push_back( "Collider2D" );
		world.RegisterArchetype( archetype );

		ECS::Prefab prefab;
		prefab.archetype = archetype.name;
		prefab.name = "EntityWithCollider";
		world.RegisterPrefab( std::move( prefab ) );

		ECS::GameEntity entity_with_collider = world.CreateGameEntity( "EntityWithCollider", circle_origin );
		Collider2DComponent& collider = entity_with_collider.GetComponent< Collider2DComponent >();
		collider.SetIsTrigger( true );
		CircleBounds2D* bounds = new CircleBounds2D( 1.f );
		collider.SetBounds( bounds );

		std::vector< ECS::GameEntity > entities_with_colliders;
		entities_with_colliders.push_back( entity_with_collider );

		// Act
		result = ExecuteRaycast( ray, entities_with_colliders, ECS::GameEntity() );

		// Assert
		EXPECT_TRUE( result.entity.IsValid() );
	}

	class RaycastAgainstCircleDoesNotCollideParams
	    : public ::testing::TestWithParam< std::tuple< Vec2f, Vec2f, Vec2f > >
	{
	};

	TEST_P( RaycastAgainstCircleDoesNotCollideParams, RaycastAgainstCircleDoesNotCollide )
	{
		// Init Parameters
		Vec2f ray_origin;
		Vec2f ray_direction;
		Vec2f circle_origin;
		std::tie( ray_origin, ray_direction, circle_origin ) = GetParam();

		using namespace Raycaster;

		// Arrange
		const Ray ray{ ray_origin, ray_direction, 100 };
		RaycastResult result;

		ECS::World world;
		world.RegisterComponent< TransformComponent >( "Transform" );
		world.RegisterComponent< Collider2DComponent >( "Collider2D" );

		ECS::Archetype archetype;
		archetype.name = "entity_with_collider";
		archetype.components.push_back( "Transform" );
		archetype.components.push_back( "Collider2D" );
		world.RegisterArchetype( archetype );

		ECS::Prefab prefab;
		prefab.archetype = archetype.name;
		prefab.name = "EntityWithCollider";
		world.RegisterPrefab( std::move( prefab ) );

		ECS::GameEntity entity_with_collider = world.CreateGameEntity( "EntityWithCollider", circle_origin );
		Collider2DComponent& collider = entity_with_collider.GetComponent< Collider2DComponent >();
		collider.SetIsTrigger( true );
		CircleBounds2D* bounds = new CircleBounds2D( 1.f );
		collider.SetBounds( bounds );

		std::vector< ECS::GameEntity > entities_with_colliders;
		entities_with_colliders.push_back( entity_with_collider );

		// Act
		result = ExecuteRaycast( ray, entities_with_colliders, ECS::GameEntity() );

		// Assert
		EXPECT_FALSE( result.entity.IsValid() );
	}

	/// <summary>
	/// This test suite checks that all rays do collide with a 1 unit radius circle. We are testing 8 ray basic
	/// directions (Up, down, left, right) and also the four diagonal possibilities.
	/// </summary>
	INSTANTIATE_TEST_SUITE_P( RaycastAgainstCircleDoesCollide, RaycastAgainstCircleDoesCollideParams,
	                          ::testing::Values( std::make_tuple( Vec2f( 0, 0 ), Vec2f( 0, 1 ), Vec2f( 0, 10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 0, -1 ), Vec2f( 0, -10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, 0 ), Vec2f( 10, 0 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, 0 ), Vec2f( -10, 0 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, 1 ), Vec2f( 10, 10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, 1 ), Vec2f( -10, 10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, -1 ), Vec2f( 10, -10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, -1 ),
	                                                              Vec2f( -10, -10 ) ) ) );

	/// <summary>
	/// This test suite checks that all rays do NOT collide with a 1 unit radius circle. We are testing 8 ray basic
	/// directions (Up, down, left, right) and also the four diagonal possibilities. We are aiming the ray in the
	/// opposite direction so we make sure it doesn't collide with any object behind the ray.
	/// </summary>
	INSTANTIATE_TEST_SUITE_P( RaycastAgainstCircleDoesNotCollide, RaycastAgainstCircleDoesNotCollideParams,
	                          ::testing::Values( std::make_tuple( Vec2f( 0, 0 ), Vec2f( 0, 1 ), Vec2f( 0, -10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 0, -1 ), Vec2f( 0, 10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, 0 ), Vec2f( -10, 0 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, 0 ), Vec2f( 10, 0 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, 1 ), Vec2f( -10, -10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, 1 ), Vec2f( 10, -10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( 1, -1 ), Vec2f( -10, 10 ) ),
	                                             std::make_tuple( Vec2f( 0, 0 ), Vec2f( -1, -1 ), Vec2f( 10, 10 ) ) ) );
} // namespace