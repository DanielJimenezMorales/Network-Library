#pragma once

class Message;
class Buffer;

class MessageUtils
{
public:
	static void ReadMessage(Buffer& buffer, Message** message);
};