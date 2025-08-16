#pragma once
#include "shared/InputState.h"

#include <unordered_map>

struct RemotePeerInputsStorage
{
	InputState lastInputState;
};

struct ServerRemotePeerInputsGlobalComponent
{
		std::unordered_map< uint32, RemotePeerInputsStorage > remotePeerInputs;
};
