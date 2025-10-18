#include "gtest/gtest.h"
#include "numeric_types.h"

#include "Vec2f.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"
#include "ecs/archetype.h"

#include "transform/transform_hierarchy_helper_functions.h"
#include "transform/transform_component.h"

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
		Engine::TransformComponent& transformComponent = entity.GetComponent< Engine::TransformComponent >();

		// Set rotation look at
		const Engine::TransformComponentProxy transformComponentProxy;
		transformComponentProxy.SetRotationLookAt( transformComponent, direction );

		EXPECT_NEAR( transformComponentProxy.GetGlobalRotation( transformComponent ), angle, EPSILON );
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
		Engine::TransformComponent& transformComponent = entity.GetComponent< Engine::TransformComponent >();

		// Set global rotation
		const Engine::TransformComponentProxy transformComponentProxy;
		transformComponentProxy.SetGlobalRotationAngle( transformComponent, angle );

		// Calculate forward vector
		const Vec2f resultedForwardVector = transformComponentProxy.GetForwardVector( transformComponent );

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
		Engine::TransformComponent& transformComponent = entity.GetComponent< Engine::TransformComponent >();

		// Set angle based on look at position
		const Engine::TransformComponentProxy transformComponentProxy;
		transformComponentProxy.LookAt( transformComponent, lookAtPosition );

		EXPECT_NEAR( transformComponentProxy.GetGlobalRotation( transformComponent ), angle, EPSILON );
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
		Engine::ECS::ChildPrefab childPrefabInstance;
		childPrefabInstance.name = "TransformOnlyChild";
		childPrefabInstance.localPosition = Vec2f( 0.f, 0.f );
		childPrefabInstance.localRotation = 0.f;
		childPrefabInstance.localScale = Vec2f( 1.f, 1.f );
		parentPrefab.childrenPrefabs.push_back( childPrefabInstance );
		world.RegisterPrefab( std::move( parentPrefab ) );
	}

	// An entity with a parent has its parent removed. We check that the parent-child relationship is correctly removed.
	TEST( TransformParentTests, RemoveParent )
	{
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		world.CreateGameEntity( "TransformOnlyParent", Vec2f( 0.f, 0.f ) );

		// Find the child entity
		const Engine::TransformComponentProxy transformComponentProxy;
		std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< Engine::TransformComponent >();
		auto childIt = entities.begin();
		for ( ; childIt != entities.end(); ++childIt )
		{
			const Engine::TransformComponent& childTransformComponent =
			    childIt->GetComponent< Engine::TransformComponent >();
			if ( transformComponentProxy.HasParent( childTransformComponent ) )
			{
				break;
			}
		}

		Engine::TransformComponent& childTransformComponent = childIt->GetComponent< Engine::TransformComponent >();

		// Find the parent entity
		auto parentIt = entities.begin();
		for ( ; parentIt != entities.end(); ++parentIt )
		{
			const Engine::TransformComponent& parentTransformComponent =
			    parentIt->GetComponent< Engine::TransformComponent >();
			if ( transformComponentProxy.HasChildren( parentTransformComponent ) )
			{
				break;
			}
		}

		const Engine::TransformComponent& parentTransformComponent =
		    parentIt->GetComponent< Engine::TransformComponent >();

		// Remove parent
		transformComponentProxy.RemoveParent( childTransformComponent, *childIt );

		EXPECT_FALSE( transformComponentProxy.HasParent( childTransformComponent ) );
		EXPECT_FALSE( transformComponentProxy.HasChildren( parentTransformComponent ) );
	}

	// An entity with no parent is set as child of another entity. We check that the parent-child relationship is
	// correctly set.
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
		const Engine::TransformComponent& parentTransformComponent =
		    parentEntity.GetComponent< Engine::TransformComponent >();
		Engine::ECS::GameEntity childEntity = entities[ 1 ];
		Engine::TransformComponent& childTransformComponent = childEntity.GetComponent< Engine::TransformComponent >();

		const Engine::TransformComponentProxy transformComponentProxy;

		// Set parent
		transformComponentProxy.SetParent( childTransformComponent, childEntity, parentEntity );

		EXPECT_TRUE( transformComponentProxy.HasParent( childTransformComponent ) );
		EXPECT_TRUE( transformComponentProxy.HasChildren( parentTransformComponent ) );
	}

	// An entity with a parent is set as child of another entity. We check that the parent-child relationship is
	// correctly set.
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

		const Engine::TransformComponentProxy transformComponentProxy;

		auto childIt = entities.begin();
		for ( ; childIt != entities.end(); ++childIt )
		{
			const Engine::TransformComponent& childTransformComponent =
			    childIt->GetComponent< Engine::TransformComponent >();
			if ( transformComponentProxy.HasParent( childTransformComponent ) )
			{
				break;
			}
		}

		Engine::TransformComponent& childTransformComponent = childIt->GetComponent< Engine::TransformComponent >();

		auto firstParentIt = entities.begin();
		for ( ; firstParentIt != entities.end(); ++firstParentIt )
		{
			const Engine::TransformComponent& firstParentTransformComponent =
			    firstParentIt->GetComponent< Engine::TransformComponent >();
			if ( transformComponentProxy.HasChildren( firstParentTransformComponent ) )
			{
				break;
			}
		}

		const Engine::TransformComponent& firstParentTransformComponent =
		    firstParentIt->GetComponent< Engine::TransformComponent >();

		auto otherParentIt = entities.begin();
		for ( ; otherParentIt != entities.end(); ++otherParentIt )
		{
			const Engine::TransformComponent& otherParentTransformComponent =
			    otherParentIt->GetComponent< Engine::TransformComponent >();
			if ( !transformComponentProxy.HasParent( otherParentTransformComponent ) &&
			     !transformComponentProxy.HasChildren( otherParentTransformComponent ) )
			{
				break;
			}
		}
		Engine::ECS::GameEntity otherParentEntity = *otherParentIt;
		const Engine::TransformComponent& otherParentTransformComponent =
		    otherParentEntity.GetComponent< Engine::TransformComponent >();

		// Set parent
		transformComponentProxy.SetParent( childTransformComponent, *childIt, otherParentEntity );

		EXPECT_TRUE( transformComponentProxy.HasParent( childTransformComponent ) );
		EXPECT_FALSE( transformComponentProxy.HasChildren( firstParentTransformComponent ) );
		EXPECT_TRUE( transformComponentProxy.HasChildren( otherParentTransformComponent ) );
	}

	// When moving a parent entity we check it also moves its child entity accordingly.
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

		const Engine::TransformComponentProxy transformComponentProxy;
		Engine::TransformComponent& parentTransformComponent = parent.GetComponent< Engine::TransformComponent >();
		Engine::TransformComponent& childTransformComponent = child.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetParent( childTransformComponent, child, parent );

		// Move parent
		transformComponentProxy.SetGlobalPosition( parentTransformComponent, Vec2f( movement, movement ) );

		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).X(),
		             childResultedPosition.X(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).Y(),
		             childResultedPosition.Y(), EPSILON );
	}

	// An entity with a parent is set as child of another entity. We check that the local transform is set accordingly
	TEST( TransformParentTests, SettingParentCheckLocalTransformFromChild )
	{
		const Vec2f parentInitialPosition( 0.f, 0.f );
		const Vec2f childInitialPosition( 5.f, 5.f );
		const Vec2f resultedLocalPosition = childInitialPosition - parentInitialPosition;

		const float32 parentInitialRotationAngle = 0.f;
		const float32 childInitialRotationAngle = 90.f;
		const float32 resultedLocalRotationAngle = childInitialRotationAngle - parentInitialRotationAngle;

		const Vec2f parentInitialScale( 1.f, 1.f );
		const Vec2f childInitialScale( 5.f, 5.f );
		const Vec2f resultedLocalScale = childInitialScale - parentInitialScale;
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		Engine::ECS::GameEntity parent = world.CreateGameEntity( "TransformOnlyChild", parentInitialPosition );
		Engine::ECS::GameEntity child = world.CreateGameEntity( "TransformOnlyChild", childInitialPosition );

		const Engine::TransformComponentProxy transformComponentProxy;
		Engine::TransformComponent& parentTransformComponent = parent.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetGlobalRotationAngle( parentTransformComponent, parentInitialRotationAngle );
		transformComponentProxy.SetGlobalScale( parentTransformComponent, parentInitialScale );

		Engine::TransformComponent& childTransformComponent = child.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetGlobalRotationAngle( childTransformComponent, childInitialRotationAngle );
		transformComponentProxy.SetGlobalScale( childTransformComponent, childInitialScale );

		// Set parent
		transformComponentProxy.SetParent( childTransformComponent, child, parent );

		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).X(), resultedLocalPosition.X(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).Y(), resultedLocalPosition.Y(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalRotationAngle( childTransformComponent ),
		             resultedLocalRotationAngle, EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalScale( childTransformComponent ).X(), resultedLocalScale.X(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalScale( childTransformComponent ).Y(), resultedLocalScale.Y(),
		             EPSILON );
	}

	// An entity with a parent has its parent removed. We check that the child has his local transform equal to his
	// global transform.
	TEST( TransformParentTests, RemovingParentCheckLocalTransformFromChildIsEqualToGlobalTransform )
	{
		const Vec2f parentPosition( 0.f, 0.f );
		const Vec2f childPosition( 10.f, 10.f );
		const float32 childRotationAngle = 90.f;
		const Vec2f childScale( 2.f, 2.f );
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entities
		Engine::ECS::GameEntity parent = world.CreateGameEntity( "TransformOnlyParent", parentPosition );

		const Engine::TransformComponentProxy transformComponentProxy;

		Engine::TransformComponent& parentTransformComponent = parent.GetComponent< Engine::TransformComponent >();
		auto children = transformComponentProxy.GetChildren( parentTransformComponent );
		assert( children.size() == 1 );

		Engine::ECS::GameEntity child = children[ 0 ];
		Engine::TransformComponent& childTransformComponent = child.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetLocalPosition( childTransformComponent, childPosition );
		transformComponentProxy.SetLocalRotationAngle( childTransformComponent, childRotationAngle );
		transformComponentProxy.SetLocalScale( childTransformComponent, childScale );

		transformComponentProxy.RemoveParent( childTransformComponent, child );

		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).X(),
		             transformComponentProxy.GetGlobalPosition( childTransformComponent ).X(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).Y(),
		             transformComponentProxy.GetGlobalPosition( childTransformComponent ).Y(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalRotationAngle( childTransformComponent ),
		             transformComponentProxy.GetGlobalRotation( childTransformComponent ), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalScale( childTransformComponent ).X(),
		             transformComponentProxy.GetGlobalScale( childTransformComponent ).X(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalScale( childTransformComponent ).Y(),
		             transformComponentProxy.GetGlobalScale( childTransformComponent ).Y(), EPSILON );
	}

	// A parent entity gets destroyed. We check that the child entity is also destroyed.
	TEST( TransformParentTests, DestroyParentCheckChildIsAlsoDestroyed )
	{
		const Vec2f parentPosition( 0.f, 0.f );
		const Vec2f childPosition( 10.f, 10.f );
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entity
		Engine::ECS::GameEntity parent = world.CreateGameEntity( "TransformOnlyParent", parentPosition );

		// Destroy entity
		world.DestroyGameEntity( parent );
		world.EndOfFrame(); // To process the destroy request

		EXPECT_TRUE( world.GetEntitiesOfType< Engine::TransformComponent >().empty() );
	}

	// A entity with a parent gets its local position updated. We check that the entity's local and global position is
	// updated.
	TEST( TransformParentTests, ChildWithParentSetLocalPositionCheckItAlsoUpdatesGlobalPosition )
	{
		const Vec2f parentInitialPosition( 1.f, 1.f );
		const Vec2f childNewLocalPosition( 10.f, 10.f );
		const Vec2f childFinalGlobalPosition = parentInitialPosition + childNewLocalPosition;
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entity
		Engine::ECS::GameEntity parent = world.CreateGameEntity( "TransformOnlyChild", parentInitialPosition );
		Engine::ECS::GameEntity child = world.CreateGameEntity( "TransformOnlyChild", parentInitialPosition );

		const Engine::TransformComponentProxy transformComponentProxy;

		Engine::TransformComponent& childTransformComponent = child.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetParent( childTransformComponent, child, parent );

		// Update local position
		transformComponentProxy.SetLocalPosition( childTransformComponent, childNewLocalPosition );

		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).X(), childNewLocalPosition.X(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).Y(), childNewLocalPosition.Y(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).X(),
		             childFinalGlobalPosition.X(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).Y(),
		             childFinalGlobalPosition.Y(), EPSILON );
	}

	// A entity without a parent gets its local position updated. We check that the entity's local and global position
	// is updated.
	TEST( TransformParentTests, ChildWithoutParentSetLocalPositionCheckItAlsoUpdatesGlobalPosition )
	{
		const Vec2f childNewLocalPosition( 10.f, 10.f );
		Engine::ECS::World world;

		// Load world with nested prefab
		LoadNestedTransformOnlyPrefabIntoWorld( world );

		// Create entity
		Engine::ECS::GameEntity child = world.CreateGameEntity( "TransformOnlyChild", Vec2f( 0.f, 0.f ) );

		const Engine::TransformComponentProxy transformComponentProxy;
		Engine::TransformComponent& childTransformComponent = child.GetComponent< Engine::TransformComponent >();

		// Update local position
		transformComponentProxy.SetLocalPosition( childTransformComponent, childNewLocalPosition );

		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).X(), childNewLocalPosition.X(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetLocalPosition( childTransformComponent ).Y(), childNewLocalPosition.Y(),
		             EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).X(),
		             childNewLocalPosition.X(), EPSILON );
		EXPECT_NEAR( transformComponentProxy.GetGlobalPosition( childTransformComponent ).Y(),
		             childNewLocalPosition.Y(), EPSILON );
	}

	// TODO Add test that when changing the parent's global rotation the children local position changes accordingly too

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