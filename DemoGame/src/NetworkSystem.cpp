#include "NetworkSystem.h"
#include "Initializer.h"
#include "Client.h"
#include "Server.h"
#include "Logger.h"

void NetworkSystem::Initialize(NetLib::PeerType type)
{
	NetLib::Initializer::Initialize();

	switch (type)
	{
	case NetLib::PeerType::ClientMode:
		_networkPeer = new NetLib::Client(5);
		break;
	case NetLib::PeerType::ServerMode:
		_networkPeer = new NetLib::Server(2);
		break;
	}

	if(!_networkPeer->Start())
	{
		Common::LOG_ERROR("Peer startup failed");
	}
}

void NetworkSystem::PreTick() const
{
	_networkPeer->Tick(0.02f);
}

void NetworkSystem::PosTick() const
{
}

void NetworkSystem::Release()
{
	if(!_networkPeer->Stop())
	{
		Common::LOG_ERROR("Peer stop failed");
	}

	delete _networkPeer;
	_networkPeer = nullptr;

	NetLib::Initializer::Finalize();
}
