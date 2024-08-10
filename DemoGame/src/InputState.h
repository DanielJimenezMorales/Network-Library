#pragma once
#include "IInputState.h"
#include "Vec2f.h"
#include <cstdint>

class InputState : public NetLib::IInputState
{
public:
	int GetSize() const override;
	void Serialize(NetLib::Buffer& buffer) const override;
	void Deserialize(NetLib::Buffer& buffer) override;

	Vec2f movement;
	int32_t mouseDeltaX;
	int32_t mouseDeltaY;
};
