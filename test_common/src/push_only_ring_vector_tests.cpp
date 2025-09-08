#include "gtest/gtest.h"

#include "numeric_types.h"
#include "push_only_ring_vector.hpp"

namespace
{
	TEST( PushOnlyRingVectorTests, CopyConstructor )
	{
		const uint32 MAX_SIZE = 32;

		PushOnlyRingVector< uint32, MAX_SIZE > sourceVector;
		for ( uint32 i = 0; i < MAX_SIZE; ++i )
		{
			sourceVector.Push( i );
		}

		PushOnlyRingVector< uint32, 32 > targetVector( sourceVector );

		EXPECT_TRUE( sourceVector == targetVector );
	}
} // namespace