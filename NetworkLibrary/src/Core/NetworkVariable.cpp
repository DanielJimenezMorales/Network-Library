#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

namespace NetLib
{
	template <typename T>
	void NetworkVariable<T>::RegisterVariable() const
	{
		_networkVariableChangesHandler->RegisterNetworkVariable(this);
	}

	template<typename T>
	void NetworkVariable<T>::UnregisterVariable() const
	{
		_networkVariableChangesHandler->UnregisterNetworkVariable(*this);
	}
}
