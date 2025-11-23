#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class Buffer;

	class IInputState
	{
		public:
			virtual int32 GetSize() const = 0;
			virtual void Serialize( Buffer& buffer ) const = 0;

			/// <summary>
			/// Deserializes the input state fields from the provided buffer. This function also checks that the fields
			/// are valid.
			/// </summary>
			/// <returns>True if deserialization and validation went successfully, False otherwise.</returns>
			virtual bool Deserialize( Buffer& buffer ) = 0;
	};
}
