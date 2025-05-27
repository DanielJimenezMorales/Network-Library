#include "InputStateFactory.h"
#include "InputState.h"
#include <cassert>

NetLib::IInputState* InputStateFactory::Create()
{
    return new InputState();
}

void InputStateFactory::Destroy(NetLib::IInputState* inputToDestroy)
{
    assert(inputToDestroy != nullptr);
    delete inputToDestroy;
    inputToDestroy = nullptr;
}
