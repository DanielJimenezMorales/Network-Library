#pragma once
#include "numeric_types.h"

#include <vector>
#include <cassert>

#include "fixed_size_function.hpp"
#include "logger.h"

// TODO Make the delegate thread-safe
// TODO Add Delegate handler instead of using a naive uint id
namespace Common
{
	// This is a no allocation multicast delegate. It works like a C# Action delegate so it always return void and it
	// receives from 0 to X parameters :)
	// TODO make this class thread-safe
	template < typename... Parameters >
	class Delegate
	{
		public:
			typedef uint16 SubscriptionId;
			static constexpr SubscriptionId INVALID_SUBSCRIPTION_ID = 0;
			static constexpr SubscriptionId MAX_SUBSCRIPTION_ID = MAX_UINT16;

			struct SubscriptionHandler
			{
				public:
					SubscriptionHandler()
					    : _id( INVALID_SUBSCRIPTION_ID )
					{
					}

					SubscriptionHandler( SubscriptionId id )
					    : _id( id )
					{
					}

					static SubscriptionHandler Invalid() { return SubscriptionHandler( INVALID_SUBSCRIPTION_ID ); }

					bool operator==( const SubscriptionHandler& other ) const { return _id == other._id; }
					bool operator!=( const SubscriptionHandler& other ) const { return !( *this == other ); }

					bool IsValid() const { return _id != INVALID_SUBSCRIPTION_ID; }
					void Reset() { _id = INVALID_SUBSCRIPTION_ID; }

				private:
					SubscriptionId _id;

					friend struct SubscriptionData;
			};

			Delegate() = default;
			Delegate( const Delegate& ) = delete;
			Delegate( Delegate&& other ) = default;

			template < typename Functor >
			SubscriptionHandler AddSubscriber( Functor&& function )
			{
				// TODO add mutex here
				const SubscriptionId id = _nextId;
				IncreaseId();
				const SubscriptionData& data = _subscriptions.emplace_back( id, std::forward< Functor >( function ) );
				return data.GetHandler();
			}

			bool DeleteSubscriber( const SubscriptionHandler& handler )
			{
				bool result = false;

				if ( handler.IsValid() )
				{
					// TODO add mutex here
					for ( auto it = _subscriptions.begin(); it != _subscriptions.end(); ++it )
					{
						if ( it->DoesHandlerCorresponds( handler ) )
						{
							_subscriptions.erase( it );
							result = true;
							break;
						}
					}
				}

				return result;
			}

			void DeleteAllSubscribers() { _subscriptions.clear(); }

			void Execute( Parameters... params )
			{
				// TODO make this thread-safe. Maybe a temp copy of the subscribers in order to safely loop them?
				for ( auto it = _subscriptions.begin(); it != _subscriptions.end(); ++it )
				{
					it->Execute( params... );
				}
			}

		private:
			struct SubscriptionData
			{
					SubscriptionData()
					    : _callback( nullptr )
					    , _id( INVALID_SUBSCRIPTION_ID )
					{
					}

					template < typename Functor >
					SubscriptionData( SubscriptionId id, Functor&& function )
					    : _callback( std::forward< Functor >( function ) )
					    , _id( id )
					{
					}

					SubscriptionData( const SubscriptionData& other ) = default;

					SubscriptionData( SubscriptionData&& other )
					    : _callback( std::move( other._callback ) )
					    , _id( std::exchange( other._id, INVALID_SUBSCRIPTION_ID ) )
					{
					}

					SubscriptionData& operator=( const SubscriptionData& other ) = default;

					SubscriptionData& operator=( SubscriptionData&& other )
					{
						if ( this == &other )
						{
							return *this;
						}

						_callback = std::move( other._callback );
						_id = std::exchange( other._id, INVALID_SUBSCRIPTION_ID );
						return *this;
					}

					SubscriptionHandler GetHandler() const
					{
						assert( IsValid() );
						return SubscriptionHandler( _id );
					}

					bool DoesHandlerCorresponds( const SubscriptionHandler& handler ) const
					{
						assert( IsValid() );
						return _id == handler._id;
					}

					void Execute( Parameters... params )
					{
						assert( IsValid() );
						_callback( std::forward< Parameters >( params )... );
					}

				private:
					bool IsValid() const { return _id != INVALID_SUBSCRIPTION_ID && _callback != nullptr; }

					std::function< void( Parameters... ) > _callback;
					SubscriptionId _id;
			};

			void IncreaseId()
			{
				assert( _nextId < MAX_SUBSCRIPTION_ID );
				// TODO Add mutex here in order to support thread-safe
				++_nextId;
			}

			std::vector< SubscriptionData > _subscriptions;
			static uint32 _nextId;
	};

	template < typename... Parameters >
	uint32 Delegate< Parameters... >::_nextId = 1;
} // namespace Common
