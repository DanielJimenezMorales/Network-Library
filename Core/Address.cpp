#include "Address.h"

bool Address::operator==(const Address& other) const
{
	if (_addressInfo.sin_family == other._addressInfo.sin_family &&
		_addressInfo.sin_port == other._addressInfo.sin_port &&
		_addressInfo.sin_addr.S_un.S_addr == other._addressInfo.sin_addr.S_un.S_addr)
	{
		return true;
	}

	return false;
}
