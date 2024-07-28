#include "SceneInitializer.h"
#include "Scene.h"
#include "ScriptSystem.h"
#include "NetworkSystem.h"
#include "GameEntity.hpp"
#include "NetworkPeerComponent.h"
#include "Client.h"
#include "Server.h"
#include "Initializer.h"
#include "NetworkEntityFactory.h"
#include "KeyboardController.h"
#include "MouseController.h"
#include "InputActionIdsConfiguration.h"
#include "InputHandler.h"
#include "ITextureLoader.h"
#include "PlayerControllerSystem.h"
#include "ServerPlayerControllerSystem.h"
#include "RemotePlayerControllerSystem.h"
#include "InputComponent.h"
#include "InputStateFactory.h"
#include "CameraComponent.h"

void SceneInitializer::InitializeScene(Scene& scene, NetLib::PeerType networkPeerType, InputHandler& inputHandler) const
{
	//Inputs
	KeyboardController* keyboard = new KeyboardController();
	InputButton button(JUMP_BUTTON, SDLK_q);
	keyboard->AddButtonMap(button);
	InputAxis axis(HORIZONTAL_AXIS, SDLK_d, SDLK_a);
	keyboard->AddAxisMap(axis);
	InputAxis axis2(VERTICAL_AXIS, SDLK_s, SDLK_w);
	keyboard->AddAxisMap(axis2);
	inputHandler.AddController(keyboard);

	MouseController* mouse = new MouseController();
	InputButton mouseButton(MOUSE_LEFT_CLICK_BUTTON, SDL_BUTTON_LEFT);
	mouse->AddButtonMap(mouseButton);
	inputHandler.AddCursor(mouse);

	//Populate entities
	GameEntity mainCameraEntity = scene.CreateGameEntity();
	//TODO Do not hardcode width and height values
	mainCameraEntity.AddComponent<CameraComponent>(Vec2f(0.f, 0.f), 512, 512);

	GameEntity inputsEntity = scene.CreateGameEntity();
	inputsEntity.AddComponent<InputComponent>(keyboard, mouse);

	GameEntity networkPeerEntity = scene.CreateGameEntity();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.AddComponent<NetworkPeerComponent>();
	NetLib::Peer* networkPeer;
	if (networkPeerType == NetLib::PeerType::ServerMode)
	{
		networkPeer = new NetLib::Server(2);
	}
	else if (networkPeerType == NetLib::PeerType::ClientMode)
	{
		networkPeer = new NetLib::Client(5);
	}

	//TODO Make this initializer internal when calling to start
	NetLib::Initializer::Initialize();
	NetworkEntityFactory* networkEntityFactory = new NetworkEntityFactory();
	networkEntityFactory->SetScene(&scene);
	networkEntityFactory->SetPeerType(networkPeerType);
	networkPeer->RegisterNetworkEntityFactory(networkEntityFactory);
	networkPeerComponent.peer = networkPeer;

	if (networkPeer->GetPeerType() == NetLib::PeerType::ServerMode)
	{
		InputStateFactory* inputStateFactory = new InputStateFactory();
		networkPeerComponent.GetPeerAsServer()->RegisterInputStateFactory(inputStateFactory);
		networkPeerComponent.inputStateFactory = inputStateFactory;
		networkPeerComponent.TrackOnRemotePeerConnect();
	}

	//Populate systems
	//TODO Create a system storage in order to be able to free them at the end
	if (networkPeerType == NetLib::PeerType::ServerMode)
	{
		ServerPlayerControllerSystem* serverPlayerControllerSystem = new ServerPlayerControllerSystem();
		scene.AddTickSystem(serverPlayerControllerSystem);
	}
	else if (networkPeerType == NetLib::PeerType::ClientMode)
	{
		PlayerControllerSystem* playerControllerSystem = new PlayerControllerSystem();
		scene.AddTickSystem(playerControllerSystem);
	}

	RemotePlayerControllerSystem* remotePlayerControllerSystem = new RemotePlayerControllerSystem();
	scene.AddTickSystem(remotePlayerControllerSystem);

	ScriptSystem* scriptSystem = new ScriptSystem();
	scene.AddUpdateSystem(scriptSystem);
	scene.AddTickSystem(scriptSystem);

	NetworkSystem* networkSystem = new NetworkSystem();
	scene.AddPreTickSystem(networkSystem);
	scene.AddPosTickSystem(networkSystem);
}
