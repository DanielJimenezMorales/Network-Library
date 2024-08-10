#include "InputState.h"
#include "Buffer.h"

int InputState::GetSize() const
{
    return (2 * sizeof(float)) + (2 * (sizeof(int32_t)));
}

void InputState::Serialize(NetLib::Buffer& buffer) const
{
    buffer.WriteFloat(movement.X());
    buffer.WriteFloat(movement.Y());

    uint32_t adaptedMouseDeltaX = mouseDeltaX + 1000;
    buffer.WriteInteger(adaptedMouseDeltaX);
    uint32_t adaptedMouseDeltaY = mouseDeltaY + 1000;
    buffer.WriteInteger(adaptedMouseDeltaY);
}

void InputState::Deserialize(NetLib::Buffer& buffer)
{
    movement.X(buffer.ReadFloat());
    movement.Y(buffer.ReadFloat());
    mouseDeltaX = static_cast<int32_t>(buffer.ReadInteger()) - 1000;
    mouseDeltaY = static_cast<int32_t>(buffer.ReadInteger()) - 1000;
}
