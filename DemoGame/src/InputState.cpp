#include "InputState.h"
#include "Buffer.h"

int InputState::GetSize() const
{
    return 2 * sizeof(float);
}

void InputState::Serialize(NetLib::Buffer& buffer) const
{
    buffer.WriteFloat(movement.X());
    buffer.WriteFloat(movement.Y());
}

void InputState::Deserialize(NetLib::Buffer& buffer)
{
    movement.X(buffer.ReadFloat());
    movement.Y(buffer.ReadFloat());
}
