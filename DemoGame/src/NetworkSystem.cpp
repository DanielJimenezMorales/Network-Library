#include "NetworkSystem.h"
#include "Initializer.h"
#include "Client.h"
#include "Server.h"
#include "Logger.h"

void NetworkSystem::Initialize(SDL_Renderer* renderer, Scene* scene, NetLib::PeerType type, IInputController* inputController)
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

	_networkEntityFactory.SetRenderer(renderer);
	_networkEntityFactory.SetScene(scene);
	_networkEntityFactory.SetPeerType(type);
	_networkEntityFactory.SetKeyboard(inputController);
	_networkPeer->RegisterNetworkEntityFactory(&_networkEntityFactory);

	if(!_networkPeer->Start())
	{
		Common::LOG_ERROR("Peer startup failed");
	}
}

void NetworkSystem::PreTick()
{
	_networkPeer->PreTick();
}

void NetworkSystem::Tick(float elapsedTime)
{
	_networkPeer->Tick(elapsedTime);

	//TEMP
	if (_networkPeer->GetPeerType() == NetLib::PeerType::ClientMode)
	{
		return;
	}
	if (_currentTick == 10)
	{
		static_cast<NetLib::Server*>(_networkPeer)->CreateNetworkEntity(10, 256.f, 256.f);
	}
	++_currentTick;
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
