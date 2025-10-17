#pragma once
#include "inputs/i_input_state.h"

#include "numeric_types.h"

#include "vec2f.h"

class InputState : public NetLib::IInputState
{
	public:
		InputState();

		int32 GetSize() const override;
		void Serialize( NetLib::Buffer& buffer ) const override;
		void Deserialize( NetLib::Buffer& buffer ) override;

		// Header fields
		uint32 tick;
		float32 serverTime;

		// Body fields
		Vec2f movement;
		bool isAiming;
		bool isShooting;
		Vec2f virtualMousePosition;
};
