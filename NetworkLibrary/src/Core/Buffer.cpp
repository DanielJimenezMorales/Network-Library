#include "buffer.h"

#include "Logger.h"
#include "asserts.h"

#include <cmath>
#include <cstring>

namespace NetLib
{
	Buffer::Buffer( uint8* data, uint32 size )
	    : _data( data )
	    , _size( size )
	{
		_index = 0;
	}

	void Buffer::Clear()
	{
		_index = 0;
	}

	void Buffer::CopyUsedData( uint8* dst, uint32 dst_size ) const
	{
		ASSERT( dst_size >= _index, "Buffer.%s The destination buffer is smaller than the used data to copy.",
		        THIS_FUNCTION_NAME );

		std::memcpy( dst, _data, _index );
	}

	void Buffer::WriteLong( uint64 value )
	{
		ASSERT( _index + 8 <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		*( ( uint64* ) ( _data + _index ) ) = value;

		_index += 8;
	}

	void Buffer::WriteInteger( uint32 value )
	{
		ASSERT( _index + 4 <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		*( ( uint32* ) ( _data + _index ) ) = value;

		_index += 4;
	}

	void Buffer::WriteShort( uint16 value )
	{
		ASSERT( _index + 2 <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		*( ( uint16* ) ( _data + _index ) ) = value;

		_index += 2;
	}

	void Buffer::WriteByte( uint8 value )
	{
		ASSERT( _index + 1 <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		*( ( uint8* ) ( _data + _index ) ) = value;

		++_index;
	}

	void Buffer::WriteFloat( float32 value )
	{
		ASSERT( _index + 4 <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		// This memcpy needs to be done using memcpy to keep the bits configuration too
		std::memcpy( ( _data + _index ), &value, sizeof( uint32 ) );
		_index += 4;
	}

	void Buffer::WriteData( const uint8* data, uint32 size )
	{
		ASSERT( data != nullptr, "Buffer.%s Can't write nullptr data.", THIS_FUNCTION_NAME );
		ASSERT( _index + size <= _size, "Buffer.%s Write operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		std::memcpy( _data + _index, data, size );
		_index += size;
	}

	uint64 Buffer::ReadLong()
	{
		ASSERT( _index + 8 <= _size, "Buffer.%s Read operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		uint64 value;
		value = *( ( uint64* ) ( _data + _index ) );

		_index += 8;
		return value;
	}

	bool Buffer::ReadLong( uint64& value )
	{
		if ( _index + 8 <= _size )
		{
			value = *( ( uint64* ) ( _data + _index ) );
			_index += 8;
			return true;
		}
		else
		{
			return false;
		}
	}

	uint32 Buffer::ReadInteger()
	{
		ASSERT( _index + 4 <= _size, "Buffer.%s Read operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		uint32 value;
		value = *( ( uint32* ) ( _data + _index ) );

		_index += 4;
		return value;
	}

	bool Buffer::ReadInteger( uint32& value )
	{
		if ( _index + 4 <= _size )
		{
			value = *( ( uint32* ) ( _data + _index ) );

			_index += 4;
			return true;
		}
		else
		{
			return false;
		}
	}

	uint16 Buffer::ReadShort()
	{
		ASSERT( _index + 2 <= _size, "Buffer.%s Read operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		uint16 value;

		value = *( ( uint16* ) ( _data + _index ) );

		_index += 2;

		return value;
	}

	bool Buffer::ReadShort( uint16& value )
	{
		if ( _index + 2 <= _size )
		{
			value = *( ( uint16* ) ( _data + _index ) );
			_index += 2;
			return true;
		}
		else
		{
			return false;
		}
	}

	uint8 Buffer::ReadByte()
	{
		ASSERT( _index + 1 <= _size, "Buffer.%s Read operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		uint8 value;

		value = *( ( uint8* ) ( _data + _index ) );

		++_index;

		return value;
	}

	bool Buffer::ReadByte( uint8& value )
	{
		if ( _index + 1 <= _size )
		{
			value = *( ( uint8* ) ( _data + _index ) );
			++_index;
			return true;
		}
		else
		{
			return false;
		}
	}

	float32 Buffer::ReadFloat()
	{
		ASSERT( _index + 4 <= _size, "Buffer.%s Read operation exceeds buffer bounds. Size: %u, Remaining: %u",
		        THIS_FUNCTION_NAME, _size, _size - _index );
		float32 value;
		// This memcpy needs to be done using memcpy to recover the bits configuration too
		std::memcpy( &value, ( _data + _index ), sizeof( float32 ) );

		_index += 4;
		return value;
	}

	bool Buffer::ReadData( uint8* data, uint32 size )
	{
		ASSERT( data != nullptr, "Buffer.%s Can't read into nullptr data.", THIS_FUNCTION_NAME );
		if ( _index + size <= _size )
		{
			std::memcpy( data, _data + _index, size );
			_index += size;
			return true;
		}
		else
		{
			return false;
		}
	}

	void Buffer::ResetAccessIndex()
	{
		_index = 0;
	}
} // namespace NetLib
