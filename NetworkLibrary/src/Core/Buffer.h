#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class Buffer
	{
		public:
			Buffer( uint8* data, uint32 size );
			Buffer( const Buffer& ) = delete;

			~Buffer() {}

			uint32 GetSize() const { return _size; }
			uint32 GetRemainingSize() const { return _size - _index; }
			uint8* GetData() const { return _data; }
			uint32 GetAccessIndex() const { return _index; }
			void Clear();

			void CopyUsedData( uint8* dst, uint32 dst_size ) const;

			void WriteLong( uint64 value );
			void WriteInteger( uint32 value );
			void WriteShort( uint16 value );
			void WriteByte( uint8 value );
			void WriteFloat( float32 value );
			void WriteData( const uint8* data, uint32 size );

			uint64 ReadLong();
			bool ReadLong( uint64& value );
			uint32 ReadInteger();
			bool ReadInteger( uint32& value );
			uint16 ReadShort();
			bool ReadShort( uint16& value );
			uint8 ReadByte();
			bool ReadByte( uint8& value );
			float32 ReadFloat();
			bool ReadData( uint8* data, uint32 size );

			void ResetAccessIndex();

		private:
			uint8* _data;
			uint32 _size;
			uint32 _index;
	};
} // namespace NetLib
