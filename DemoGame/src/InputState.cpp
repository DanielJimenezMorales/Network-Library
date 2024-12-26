#include "InputState.h"

#include "core/buffer.h"

int32 InputState::GetSize() const
{
    return (4 * sizeof(float));
}

void InputState::Serialize(NetLib::Buffer& buffer) const
{
    buffer.WriteFloat(movement.X());
    buffer.WriteFloat(movement.Y());

    buffer.WriteFloat(virtualMousePosition.X());
    buffer.WriteFloat(virtualMousePosition.Y());
}

void InputState::Deserialize(NetLib::Buffer& buffer)
{
    movement.X(buffer.ReadFloat());
    movement.Y(buffer.ReadFloat());

    virtualMousePosition.X(buffer.ReadFloat());
    virtualMousePosition.Y(buffer.ReadFloat());
}
