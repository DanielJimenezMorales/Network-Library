#pragma once
#include <memory>

class Message;
class Buffer;

class MessageUtils
{
public:
	static std::unique_ptr<Message> ReadMessage(Buffer& buffer);
};