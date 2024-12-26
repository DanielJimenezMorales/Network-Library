#pragma once
#include "inputs/i_input_state.h"

#include "Vec2f.h"
#include <cstdint>

class InputState : public NetLib::IInputState
{
public:
	int32 GetSize() const override;
	void Serialize(NetLib::Buffer& buffer) const override;
	void Deserialize(NetLib::Buffer& buffer) override;

	Vec2f movement;
	Vec2f virtualMousePosition;
};
