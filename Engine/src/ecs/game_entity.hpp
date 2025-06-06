#pragma once
#include <cassert>

#include "safe_pointer.hpp"

#include "entt.hpp"

#include "ecs/entity_container.h"

namespace Engine
{
	namespace ECS
	{
		// TODO improve GameEntity
		class GameEntity
		{
			public:
				GameEntity()
				    : _ecsEntityId()
				    , _entityContainer( nullptr ) {};
				GameEntity( EntityId enttId, EntityContainer* entityContainer )
				    : _ecsEntityId( enttId )
				    , _entityContainer( entityContainer ) {};
				GameEntity( const GameEntity& other ) = default;
				~GameEntity() {};

				GameEntity& operator=( const GameEntity& other )
				{
					if ( this == &other )
					{
						return *this;
					}

					_ecsEntityId = other._ecsEntityId;
					_entityContainer = other._entityContainer;
					return *this;
				}

				bool operator==( const GameEntity& other ) const
				{
					return _ecsEntityId == other._ecsEntityId && _entityContainer == other._entityContainer;
				}

				bool operator!=( const GameEntity& other ) const { return !( *this == other ); }

				EntityId GetId() const { return _ecsEntityId; }

				EntityContainer* GetEntityContainer() { return _entityContainer.GetRawPointer(); }
				const EntityContainer* GetEntityContainer() const { return _entityContainer.GetRawPointer(); }

				bool IsValid() const { return _entityContainer != nullptr; };

				template < typename T, typename... Params >
				T& AddComponent( Params&&... params );

				template < typename T >
				bool HasComponent() const;

				template < typename T >
				T& GetComponent();

				template < typename T >
				const T& GetComponent() const;

				template < typename T >
				void RemoveComponent();

			private:
				EntityId _ecsEntityId;

				SafePointer< EntityContainer > _entityContainer;

				friend class EntityContainer;
		};

		template < typename T, typename... Params >
		inline T& GameEntity::AddComponent( Params&&... params )
		{
			assert( IsValid() );
			return _entityContainer->AddComponentToEntity< T >( *this, std::forward< Params >( params )... );
		};

		template < typename T >
		inline bool GameEntity::HasComponent() const
		{
			assert( IsValid() );
			return _entityContainer->HasEntityComponent< T >( *this );
		};

		template < typename T >
		inline T& GameEntity::GetComponent()
		{
			assert( IsValid() );
			return _entityContainer->GetComponentFromEntity< T >( *this );
		}
		template < typename T >
		inline const T& GameEntity::GetComponent() const
		{
			assert( IsValid() );
			return _entityContainer->GetComponentFromEntity< T >( *this );
		}

		template < typename T >
		inline void GameEntity::RemoveComponent()
		{
			assert( IsValid() );
			_entityContainer->RemoveComponentFromEntity( *this );
		};
	} // namespace ECS
} // namespace Engine
