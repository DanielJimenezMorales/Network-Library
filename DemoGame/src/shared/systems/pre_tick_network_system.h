#pragma once
#include "ecs/i_simple_system.h"

#include <string>

class PreTickNetworkSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PreTickNetworkSystem( const std::string& ip, uint32 port );

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

	private:
		std::string _ip;
		uint32 _port;
};
