#include "network_peer_global_component.h"

#include "inputs/i_input_state_factory.h"

NetworkPeerGlobalComponent::~NetworkPeerGlobalComponent()
{
	if ( isTrackingRemotePeerConnect )
	{
		peer->UnsubscribeToOnPeerConnected( remotePeerConnectSubscriptionId );
	}

	delete peer;
	peer = nullptr;

	if ( inputStateFactory != nullptr )
	{
		delete inputStateFactory;
		inputStateFactory = nullptr;
	}
}
