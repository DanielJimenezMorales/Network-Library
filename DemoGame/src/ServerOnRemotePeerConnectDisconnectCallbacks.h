#pragma once
class Scene;

class ServerOnRemotePeerConnectDisconnectCallbacks
{
public:
	ServerOnRemotePeerConnectDisconnectCallbacks(Scene* scene) : _scene(scene)
	{
	}

	void OnRemotePeerConnect() const;
	//void OnRemotePeerDisconnect() const;

private:
	Scene* _scene;
};