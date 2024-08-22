#pragma once
#include "IInputStateFactory.h"

class InputStateFactory : public NetLib::IInputStateFactory
{
public:
	InputStateFactory() : IInputStateFactory() {}
	~InputStateFactory() {}

	NetLib::IInputState* Create() override;
	void Destroy(NetLib::IInputState* inputToDestroy) override;
};