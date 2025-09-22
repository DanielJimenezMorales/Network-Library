#include "gtest/gtest.h"
#include "numeric_types.h"

#include "Vec2f.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"
#include "ecs/archetype.h"

#include "read_only_transform_component_proxy.h"
#include "transform_component_proxy.h"

#include "components/transform_component.h"

namespace
{
	static void LoadTransformOnlyPrefabIntoWorld( Engine::ECS::World& world )
	{
		// Register component
		world.RegisterComponent< Engine::TransformComponent >( "Transform" );

		// Register archetype
		Engine::ECS::Archetype archetype = Engine::ECS::Archetype();
		archetype.name = "transform_only";
		archetype.components.push_back( "Transform" );
		world.RegisterArchetype( archetype );

		// Register prefab
		Engine::ECS::Prefab prefab = Engine::ECS::Prefab();
		prefab.name = "TransformOnly";
		prefab.archetype = "transform_only";
		world.RegisterPrefab( std::move( prefab ) );
	}

	class RotationAngleAndDirectionParams : public ::testing::TestWithParam< std::tuple< float32, Vec2f > >
	{
	};

	constexpr float32 EPSILON = 1e-6f;

	TEST_P( RotationAngleAndDirectionParams, SetRotationLookAt )
	{
		float32 angle = 0.f;
		Vec2f direction( 0.f, 0.f );
		Engine::ECS::World world;
		std::tie( angle, direction ) = GetParam();

		// Load world with prefab
		LoadTransformOnlyPrefabIntoWorld( world );

		// Create entity
		world.CreateGameEntity( "TransformOnly", Vec2f( 0.f, 0.f ) );

		// Get transform
		Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< Engine::TransformComponent >();
		Engine::TransformComponentProxy transformComponentProxy( entity );

		// Set rotation look at
		transformComponentProxy.SetRotationLookAt( direction );

		EXPECT_NEAR( transformComponentProxy.GetGlobalRotation(), angle, EPSILON );
	}

	TEST_P( RotationAngleAndDirectionParams, GetForwardVector )
	{
		float32 angle = 0.f;
		Vec2f forwardVector( 0.f, 0.f );
		Engine::ECS::World world;
		std::tie( angle, forwardVector ) = GetParam();

		// Load world with prefab
		LoadTransformOnlyPrefabIntoWorld( world );

		// Create entity
		world.CreateGameEntity( "TransformOnly", Vec2f( 0.f, 0.f ) );

		// Get transform
		Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< Engine::TransformComponent >();
		Engine::TransformComponentProxy transformComponentProxy( entity );

		// Set global rotation
		transformComponentProxy.SetGlobalRotationAngle( angle );

		// Calculate forward vector
		const Vec2f resultedForwardVector = transformComponentProxy.GetForwardVector();

		EXPECT_NEAR( resultedForwardVector.X(), forwardVector.X(), EPSILON );
		EXPECT_NEAR( resultedForwardVector.Y(), forwardVector.Y(), EPSILON );
	}

	class LookAtPositionCurrentPositionAndRotationAngleParams
	    : public ::testing::TestWithParam< std::tuple< Vec2f, Vec2f, float32 > >
	{
	};

	TEST_P( LookAtPositionCurrentPositionAndRotationAngleParams, LookAt )
	{
		Vec2f position( 0.f, 0.f );
		Vec2f lookAtPosition( 0.f, 0.f );
		float32 angle = 0.f;
		Engine::ECS::World world;
		std::tie( position, lookAtPosition, angle ) = GetParam();

		// Load world with prefab
		LoadTransformOnlyPrefabIntoWorld( world );

		// Create entity
		world.CreateGameEntity( "TransformOnly", position );

		// Get transform
		Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< Engine::TransformComponent >();
		Engine::TransformComponentProxy transformComponentProxy( entity );

		// Set angle based on look at position
		transformComponentProxy.LookAt( lookAtPosition );

		EXPECT_NEAR( transformComponentProxy.GetGlobalRotation(), angle, EPSILON );
	}

	static void LoadNestedTransformOnlyPrefabIntoWorld( Engine::ECS::World& world )
	{
		// Register component
		world.RegisterComponent< Engine::TransformComponent >( "Transform" );

		// Register archetype
		Engine::ECS::Archetype archetype = Engine::ECS::Archetype();
		archetype.name = "transform_only";
		archetype.components.push_back( "Transform" );
		world.RegisterArchetype( archetype );

		// Register child prefab
		Engine::ECS::Prefab childPrefab = Engine::ECS::Prefab();
		childPrefab.name = "TransformOnlyChild";
		childPrefab.archetype = "transform_only";
		world.RegisterPrefab( std::move( childPrefab ) );

		// Register parent prefab
		Engine::ECS::Prefab parentPrefab = Engine::ECS::Prefab();
		parentPrefab.name = "TransformOnlyParent";
		parentPrefab.archetype = "transform_only";
		parentPrefab.childrenPrefabNames.push_back( "TransformOnlyChild" );
		world.RegisterPrefab( std::move( parentPrefab ) );
	}

	TEST( TransformParentTests, RemoveParent )
	{
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		world.CreateGameEntity( "TransformOnlyParent", Vec2f( 0.f, 0.f ) );
		// Find the child entity
		std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< Engine::TransformComponent >();
		auto childIt = entities.begin();
		for ( ; childIt != entities.end(); ++childIt )
		{
			Engine::TransformComponentProxy transformProxy( *childIt );
			if ( transformProxy.HasParent() )
			{
				break;
			}
		}
		Engine::TransformComponentProxy childTransformProxy( *childIt );

		// Find the parent entity
		auto parentIt = entities.begin();
		for ( ; parentIt != entities.end(); ++parentIt )
		{
			Engine::TransformComponentProxy transformProxy( *parentIt );
			if ( transformProxy.HasChildren() )
			{
				break;
			}
		}
		Engine::TransformComponentProxy parentTransformProxy( *parentIt );

		// Remove parent
		childTransformProxy.RemoveParent();

		EXPECT_FALSE( childTransformProxy.HasParent() );
		EXPECT_FALSE( parentTransformProxy.HasChildren() );
	}

	TEST( TransformParentTests, HavingNoParentSetParent )
	{
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		world.CreateGameEntity( "TransformOnlyChild", Vec2f( 0.f, 0.f ) );
		world.CreateGameEntity( "TransformOnlyChild", Vec2f( 0.f, 0.f ) );

		// Get parent and child entities
		std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< Engine::TransformComponent >();
		assert( entities.size() == 2 );

		Engine::ECS::GameEntity parentEntity = entities[ 0 ];
		Engine::ECS::GameEntity childEntity = entities[ 1 ];
		Engine::TransformComponentProxy parentTransformProxy( parentEntity );
		Engine::TransformComponentProxy childTransformProxy( childEntity );

		// Set parent
		childTransformProxy.SetParent( parentEntity );

		EXPECT_TRUE( childTransformProxy.HasParent() );
		EXPECT_TRUE( parentTransformProxy.HasChildren() );
	}

	TEST( TransformParentTests, HavingParentSetOtherParent )
	{
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		world.CreateGameEntity( "TransformOnlyParent", Vec2f( 0.f, 0.f ) );
		world.CreateGameEntity( "TransformOnlyChild", Vec2f( 0.f, 0.f ) );

		// Get parents and child entities
		std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< Engine::TransformComponent >();
		assert( entities.size() == 3 );

		auto childIt = entities.begin();
		for ( ; childIt != entities.end(); ++childIt )
		{
			Engine::TransformComponentProxy transformProxy( *childIt );
			if ( transformProxy.HasParent() )
			{
				break;
			}
		}
		Engine::TransformComponentProxy childTransformProxy( *childIt );

		auto firstParentIt = entities.begin();
		for ( ; firstParentIt != entities.end(); ++firstParentIt )
		{
			Engine::TransformComponentProxy transformProxy( *firstParentIt );
			if ( transformProxy.HasChildren() )
			{
				break;
			}
		}
		Engine::TransformComponentProxy firstParentTransformProxy( *firstParentIt );

		auto otherParentIt = entities.begin();
		for ( ; otherParentIt != entities.end(); ++otherParentIt )
		{
			Engine::TransformComponentProxy transformProxy( *otherParentIt );
			if ( !transformProxy.HasParent() && !transformProxy.HasChildren() )
			{
				break;
			}
		}
		Engine::ECS::GameEntity otherParentEntity = *otherParentIt;
		Engine::TransformComponentProxy otherParentTransformProxy( *otherParentIt );

		// Set parent
		childTransformProxy.SetParent( otherParentEntity );

		EXPECT_TRUE( childTransformProxy.HasParent() );
		EXPECT_FALSE( firstParentTransformProxy.HasChildren() );
		EXPECT_TRUE( otherParentTransformProxy.HasChildren() );
	}

	TEST( TransformParentTests, MovingParentCheckIfChildHasAlsoMoved )
	{
		const Vec2f parentInitialPosition( 0.f, 0.f );
		const Vec2f childInitialPosition( 5.f, 5.f );
		const float32 movement = 10.f;
		const Vec2f childResultedPosition = parentInitialPosition + childInitialPosition + Vec2f( movement, movement );
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		Engine::ECS::GameEntity parent = world.CreateGameEntity( "TransformOnlyChild", parentInitialPosition );
		Engine::ECS::GameEntity child = world.CreateGameEntity( "TransformOnlyChild", childInitialPosition );

		Engine::TransformComponentProxy parentTransformProxy( parent );
		Engine::TransformComponentProxy childTransformProxy( child );
		childTransformProxy.SetParent( parent );

		// Move parent
		parentTransformProxy.SetGlobalPosition( Vec2f( movement, movement ) );

		EXPECT_NEAR( childTransformProxy.GetGlobalPosition().X(), childResultedPosition.X(), EPSILON );
		EXPECT_NEAR( childTransformProxy.GetGlobalPosition().Y(), childResultedPosition.Y(), EPSILON );
	}

	INSTANTIATE_TEST_SUITE_P( SetRotationLookAt, RotationAngleAndDirectionParams,
	                          ::testing::Values( std::make_tuple( 0.f, Vec2f( 1.f, 0.f ) ),
	                                             std::make_tuple( 90.f, Vec2f( 0.f, 1.f ) ),
	                                             std::make_tuple( 180.f, Vec2f( -1.f, 0.f ) ),
	                                             std::make_tuple( 270.f, Vec2f( 0.f, -1.f ) ) ) );

	INSTANTIATE_TEST_SUITE_P( GetForwardVector, RotationAngleAndDirectionParams,
	                          ::testing::Values( std::make_tuple( 0.f, Vec2f( 1.f, 0.f ) ),
	                                             std::make_tuple( 90.f, Vec2f( 0.f, 1.f ) ),
	                                             std::make_tuple( 180.f, Vec2f( -1.f, 0.f ) ),
	                                             std::make_tuple( 270.f, Vec2f( 0.f, -1.f ) ) ) );

	INSTANTIATE_TEST_SUITE_P( LookAt, LookAtPositionCurrentPositionAndRotationAngleParams,
	                          ::testing::Values( std::make_tuple( Vec2f( 0.f, 0.f ), Vec2f( 10.f, 0.f ), 0.f ),
	                                             std::make_tuple( Vec2f( 0.f, 0.f ), Vec2f( 0.f, 10.f ), 90.f ),
	                                             std::make_tuple( Vec2f( 0.f, 0.f ), Vec2f( -10.f, 0.f ), 180.f ),
	                                             std::make_tuple( Vec2f( 0.f, 0.f ), Vec2f( 0.f, -10.f ), 270.f ) ) );
} // namespace