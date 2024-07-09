#pragma once

namespace NetLib
{
	class Buffer;

	class IInputState
	{
	public:
		virtual int GetSize() const = 0;
		virtual void Serialize(Buffer& buffer) const = 0;
		virtual void Deserialize(Buffer& buffer) = 0;
	};
}
