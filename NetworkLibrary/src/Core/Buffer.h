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
			uint32 ReadInteger();
			uint16 ReadShort();
			uint8 ReadByte();
			float32 ReadFloat();
			void ReadData( uint8* data, uint32 size );

			void ResetAccessIndex();

		private:
			uint8* _data;
			uint32 _size;
			uint32 _index;
	};
} // namespace NetLib
