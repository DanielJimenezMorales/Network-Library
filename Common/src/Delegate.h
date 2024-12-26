#pragma once
#include "numeric_types.h"

#include <vector>

#include "fixed_size_function.hpp"
#include "logger.h"

namespace Common
{
	// This is a no allocation multicast delegate. It works like a C# Action delegate so it always return void and it
	// receives from 0 to X parameters :)
	// TODO make this class thread-safe
	template < typename... Parameters >
	class Delegate
	{
		public:
			Delegate()
			{
				const uint32 initialSize = 3;
				Init( initialSize );
			}

			template < typename Functor >
			uint32 AddSubscriber( Functor&& function )
			{
				// TODO add mutex here
				int32 index = -1;
				for ( uint32 i = 0; i < _subscribers.size(); ++i )
				{
					if ( !_subscribers[ i ] )
					{
						index = i;
						break;
					}
				}

				uint32 idToReturn = 0;
				if ( index != -1 )
				{
					_subscribers[ index ] = std::forward< Functor >( function );
					idToReturn = _ids[ index ];
				}
				else
				{
					_subscribers.emplace_back( std::forward< Functor >( function ) );
					_ids.emplace_back( nextId );
					idToReturn = nextId;
					IncreaseId();
				}

				return idToReturn;
			}

			void DeleteSubscriber( uint32 id )
			{
				// TODO add mutex here
				for ( uint32 i = 0; i < _ids.size(); ++i )
				{
					if ( _ids[ i ] == id )
					{
						_subscribers[ i ].reset();
						break;
					}
				}
			}

			void DeleteAllSubscribers()
			{
				// TODO add mutex here
				for ( uint32 i = 0; i < _subscribers.size(); ++i )
				{
					_subscribers[ i ].reset();
				}
			}

			void Execute( Parameters... params )
			{
				// TODO make this thread-safe. Maybe a temp copy of the subscribers in order to safely loop them?
				for ( uint32 i = 0; i < _subscribers.size(); ++i )
				{
					if ( _subscribers[ i ] )
					{
						_subscribers[ i ]( params... );
					}
				}
			}

		private:
			void Init( uint32 initialSize )
			{
				_subscribers.reserve( initialSize );
				_ids.reserve( initialSize );

				for ( uint32 i = 0; i < initialSize; ++i )
				{
					_subscribers.emplace_back();
					_ids.emplace_back( nextId );
					IncreaseId();
				}
			}

			void IncreaseId()
			{
				// TODO Add mutex here in order to support thread-safe
				++nextId;
			}

			std::vector< uint32 > _ids;
			std::vector< fixed_size_function< void( Parameters... ), 128, construct_type::copy_and_move > >
			    _subscribers;
			static uint32 nextId;
	};

	template < typename... Parameters >
	uint32 Delegate< Parameters... >::nextId = 0;
} // namespace Common
