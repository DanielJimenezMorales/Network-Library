#pragma once
#include <queue>
#include <unordered_map>
#include <memory>

#include "communication/message.h"

namespace NetLib
{
	class MessageFactory
	{
		public:
			std::unique_ptr< Message > LendMessage( MessageType messageType );
			void ReleaseMessage( std::unique_ptr< Message > message );

			MessageFactory( uint32 size );
			MessageFactory( const MessageFactory& ) = delete;

			~MessageFactory();

			MessageFactory& operator=( const MessageFactory& ) = delete;

		private:
			void InitializePools();
			void InitializePool( std::queue< std::unique_ptr< Message > >& pool, MessageType messageType );
			std::queue< std::unique_ptr< Message > >* GetPoolFromType( MessageType messageType );
			std::unique_ptr< Message > CreateMessage( MessageType messageType );
			void ReleasePool( std::queue< std::unique_ptr< Message > >& pool );

			bool _isInitialized;
			uint32 _initialSize;

			std::unordered_map< MessageType, std::queue< std::unique_ptr< Message > > > _messagePools;
	};
} // namespace NetLib
