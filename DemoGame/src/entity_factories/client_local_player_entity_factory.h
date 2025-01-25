#pragma once
#include "entity_factories/i_entity_factory.h"

class TextureResourceHandler;

class ClientLocalPlayerEntityFactory : public IEntityFactory
{
	public:
		ClientLocalPlayerEntityFactory();

		void Configure( TextureResourceHandler* texture_resource_handler );

		void Create( GameEntity& entity, const BaseEntityConfiguration* configuration ) override;
		void Destroy( GameEntity& entity ) override;

	private:
		TextureResourceHandler* _textureResourceHandler;
};
