#include "gtest/gtest.h"
#include "numeric_types.h"

#include "Vec2f.h"
#include "components/transform_component.h"

namespace
{
	class RotationAngleAndDirectionParams : public ::testing::TestWithParam< std::tuple< float32, Vec2f > >
	{
	};

	constexpr float32 EPSILON = 1e-6f;

	TEST_P( RotationAngleAndDirectionParams, SetRotationLookAt )
	{
		float32 angle = 0.f;
		Vec2f direction( 0.f, 0.f );
		std::tie( angle, direction ) = GetParam();

		Engine::TransformComponent transformComponent;
		transformComponent.SetRotationLookAt( direction );

		EXPECT_NEAR( transformComponent.GetRotationAngle(), angle, EPSILON );
	}

	TEST_P( RotationAngleAndDirectionParams, GetForwardVector )
	{
		float32 angle = 0.f;
		Vec2f forwardVector( 0.f, 0.f );
		std::tie( angle, forwardVector ) = GetParam();

		Engine::TransformComponent transformComponent( Vec2f( 0.f, 0.f ), angle);
		const Vec2f resultedForwardVector = transformComponent.GetForwardVector();

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
		std::tie( position, lookAtPosition, angle ) = GetParam();

		Engine::TransformComponent transformComponent( position, 0.f );
		transformComponent.LookAt( lookAtPosition );

		EXPECT_NEAR( transformComponent.GetRotationAngle(), angle, EPSILON );
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