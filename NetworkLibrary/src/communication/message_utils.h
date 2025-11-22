#pragma once
#include <memory>

namespace NetLib
{
	class Message;
	class MessageFactory;
	class Buffer;

	class MessageUtils
	{
		public:
			static std::unique_ptr< Message > ReadMessage( MessageFactory& message_factory, Buffer& buffer );
	};
}
