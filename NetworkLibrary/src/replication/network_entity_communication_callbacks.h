#pragma once
#include "delegate.hpp"

#include "core/buffer.h"

namespace NetLib
{
	struct NetworkEntityCommunicationCallbacks
	{
			NetworkEntityCommunicationCallbacks() = default;

			/// <summary>
			/// <para>SERVER ONLY</para>
			/// <para>This callback is executed everytime the library requests a full entity state serialization for
			/// sending it to the owner client.</para>
			/// </summary>
			Common::Delegate< Buffer& > OnSerializeEntityStateForOwner;

			/// <summary>
			/// <para>SERVER ONLY</para>
			/// <para>This callback is executed everytime the library requests a partial entity state serialization for
			/// sending it to a non owner client.</para>
			/// </summary>
			Common::Delegate< Buffer& > OnSerializeEntityStateForNonOwner;

			/// <summary>
			/// <para>CLIENT ONLY</para>
			/// <para>This callback is executed everytime the library has received a full entity state for unserialize
			/// and process it within this client (The owner of the entity).</para>
			/// </summary>
			Common::Delegate< Buffer& > OnUnserializeEntityStateForOwner;

			/// <summary>
			/// <para>CLIENT ONLY</para>
			/// <para>This callback is executed everytime the library has received a partial entity state for
			/// unserialize and process it within this client (A non owner of the entity).</para>
			/// </summary>
			Common::Delegate< Buffer& > OnUnserializeEntityStateForNonOwner;
	};
}
