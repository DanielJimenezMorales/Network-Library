#include "NetworkSystem.h"
#include "Initializer.h"
#include "Client.h"
#include "Server.h"
#include "Logger.h"
#include "CurrentTickComponent.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include <vector>

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
		LOG_ERROR("Peer startup failed");
	}
}

void NetworkSystem::PreTick(EntityContainer& entityContainer, float elapsedTime) const
{
	_networkPeer->PreTick();
}

void NetworkSystem::PosTick(EntityContainer& entityContainer, float elapsedTime) const
{
	_networkPeer->Tick(elapsedTime);

	std::vector<GameEntity> gameEntity = entityContainer.GetEntitiesOfType<CurrentTickComponent>();
	CurrentTickComponent& currentTickComponent = gameEntity[0].GetComponent<CurrentTickComponent>();

	//TEMP
	if (_networkPeer->GetPeerType() == NetLib::PeerType::ClientMode)
	{
		return;
	}
	if (currentTickComponent.currentTick == 10)
	{
		static_cast<NetLib::Server*>(_networkPeer)->CreateNetworkEntity(10, 256.f, 256.f);
	}
	++currentTickComponent.currentTick;
}

void NetworkSystem::Release()
{
	if(!_networkPeer->Stop())
	{
		LOG_ERROR("Peer stop failed");
	}

	delete _networkPeer;
	_networkPeer = nullptr;

	NetLib::Initializer::Finalize();
}
