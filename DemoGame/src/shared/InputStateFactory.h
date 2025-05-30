#pragma once
#include "inputs/i_input_state_factory.h"

class InputStateFactory : public NetLib::IInputStateFactory
{
public:
	InputStateFactory() : IInputStateFactory() {}
	~InputStateFactory() {}

	NetLib::IInputState* Create() override;
	void Destroy(NetLib::IInputState* inputToDestroy) override;
};