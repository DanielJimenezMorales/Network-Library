#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class Buffer
	{
		public:
			Buffer( uint8* data, int32 size );
			Buffer( const Buffer& ) = delete;

			~Buffer() {}

			int32 GetSize() const { return _size; }
			uint8* GetData() const { return _data; }
			uint32 GetAccessIndex() const { return _index; }
			void Clear();

			void CopyUsedData( uint8* dst, uint32 dst_size ) const;

			void WriteLong( uint64 value );
			void WriteInteger( uint32 value );
			void WriteShort( uint16 value );
			void WriteByte( uint8 value );
			void WriteFloat( float32 value );

			uint64 ReadLong();
			uint32 ReadInteger();
			uint16 ReadShort();
			uint8 ReadByte();
			float32 ReadFloat();

			void ResetAccessIndex();

		private:
			uint8* _data;
			int32 _size;
			int32 _index;
	};
} // namespace NetLib
