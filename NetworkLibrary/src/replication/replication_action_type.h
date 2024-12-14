#pragma once
#include "NumericTypes.h"

namespace NetLib
{
	enum class ReplicationActionType : uint8
	{
		CREATE = 0,
		UPDATE = 1,
		DESTROY = 2
	};
}
