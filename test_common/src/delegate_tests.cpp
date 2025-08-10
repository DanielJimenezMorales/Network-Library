#include "gtest/gtest.h"

#include "numeric_types.h"
#include "Delegate.hpp"

namespace
{
	struct DelegateCallbackTracker
	{
			DelegateCallbackTracker()
			    : voidCallbackCount( 0 )
			{
			}

			void CallbackVoidPlusOne() { ++voidCallbackCount; }
			void CallbackVoidPlusTwo() { voidCallbackCount += 2; }

			uint32 voidCallbackCount;
	};

	TEST( DelegateTests, RemoveSubscriber )
	{
		const uint32 NUMBER_OF_ITERATIONS = 100;

		using namespace Common;
		Delegate<> delegate;

		DelegateCallbackTracker callbackTracker;
		const Delegate<>::SubscriptionHandler& subscriptionHandler =
		    delegate.AddSubscriber( std::bind( &DelegateCallbackTracker::CallbackVoidPlusOne, &callbackTracker ) );

		const bool removedSuccessfully = delegate.DeleteSubscriber( subscriptionHandler );

		for ( uint32 i = 0; i < NUMBER_OF_ITERATIONS; ++i )
		{
			delegate.Execute();
		}

		EXPECT_TRUE( removedSuccessfully );
		EXPECT_EQ( callbackTracker.voidCallbackCount, 0 );
	}

	TEST( DelegateTests, RemoveInvalidSubscriber )
	{
		using namespace Common;
		Delegate<> delegate;

		const Delegate<>::SubscriptionHandler& subscriptionHandler( Delegate<>::SubscriptionHandler::Invalid() );

		const bool removedSuccessfully = delegate.DeleteSubscriber( subscriptionHandler );

		EXPECT_FALSE( removedSuccessfully );
	}

	TEST( DelegateTests, RemoveAllSubscribers )
	{
		const uint32 NUMBER_OF_ITERATIONS = 100;

		using namespace Common;
		Delegate<> delegate;

		DelegateCallbackTracker callbackTracker;
		delegate.AddSubscriber( std::bind( &DelegateCallbackTracker::CallbackVoidPlusOne, &callbackTracker ) );
		delegate.AddSubscriber( std::bind( &DelegateCallbackTracker::CallbackVoidPlusTwo, &callbackTracker ) );

		delegate.DeleteAllSubscribers();

		for ( uint32 i = 0; i < NUMBER_OF_ITERATIONS; ++i )
		{
			delegate.Execute();
		}

		EXPECT_EQ( callbackTracker.voidCallbackCount, 0 );
	}

	TEST( DelegateTests, Execute )
	{
		const uint32 NUMBER_OF_ITERATIONS = 100;

		using namespace Common;
		Delegate<> delegate;

		DelegateCallbackTracker callbackTracker;
		delegate.AddSubscriber( std::bind( &DelegateCallbackTracker::CallbackVoidPlusOne, &callbackTracker ) );
		delegate.AddSubscriber( std::bind( &DelegateCallbackTracker::CallbackVoidPlusTwo, &callbackTracker ) );

		for ( uint32 i = 0; i < NUMBER_OF_ITERATIONS; ++i )
		{
			delegate.Execute();
		}

		EXPECT_EQ( callbackTracker.voidCallbackCount, 300 );
	}
} // namespace