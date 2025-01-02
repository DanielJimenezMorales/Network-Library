#pragma once
#include "entt.hpp"
#include "ComponentView.hpp"
#include <vector>

#include "numeric_types.h"
#include <cassert>

class GameEntity;

namespace ECS
{
	class EntityContainer
	{
		public:
			GameEntity CreateGameEntity();

			void DestroyGameEntity( const GameEntity& gameEntity );

			template < typename T >
			bool HasEntityComponent( const GameEntity& gameEntity ) const;

			template < typename T, typename... Params >
			T& AddComponentToEntity( const GameEntity& gameEntity, Params&&... params );

			template < typename T >
			T& GetComponentFromEntity( const GameEntity& gameEntity );

			template < typename T >
			const T& GetComponentFromEntity( const GameEntity& gameEntity ) const;

			template < typename T >
			void RemoveComponentFromEntity( const GameEntity& gameEntity );

			template < typename T >
			std::vector< GameEntity > GetEntitiesOfType();

			template < typename T >
			const std::vector< GameEntity > GetEntitiesOfType() const;

			template < typename T >
			GameEntity GetFirstEntityOfType();

			template < typename T >
			const GameEntity GetFirstEntityOfType() const;

			template < typename T1, typename T2 >
			std::vector< GameEntity > GetEntitiesOfBothTypes();

			template < typename T1, typename T2 >
			const std::vector< GameEntity > GetEntitiesOfBothTypes() const;

			template < typename T >
			T& GetFirstComponentOfType();

			template < typename T >
			const T& GetFirstComponentOfType() const;

			template < typename T >
			ComponentView< T > GetComponentsOfType();

			template < typename T >
			const ComponentView< T > GetComponentsOfType() const;

			GameEntity GetEntityFromId( uint32 id );

		private:
			entt::registry _entities;
	};

	template < typename T >
	inline bool EntityContainer::HasEntityComponent( const GameEntity& gameEntity ) const
	{
		return _entities.all_of< T >( gameEntity._ecsEntityId );
	}

	template < typename T, typename... Params >
	inline T& EntityContainer::AddComponentToEntity( const GameEntity& gameEntity, Params&&... params )
	{
		assert( !HasEntityComponent< T >( gameEntity ) );
		return _entities.emplace< T >( gameEntity._ecsEntityId, std::forward< Params >( params )... );
	}

	template < typename T >
	inline T& EntityContainer::GetComponentFromEntity( const GameEntity& gameEntity )
	{
		assert( HasEntityComponent< T >( gameEntity ) );
		return _entities.get< T >( gameEntity._ecsEntityId );
	}

	template < typename T >
	inline const T& EntityContainer::GetComponentFromEntity( const GameEntity& gameEntity ) const
	{
		assert( HasEntityComponent< T >( gameEntity ) );
		return _entities.get< T >( gameEntity._ecsEntityId );
	}

	template < typename T >
	inline void EntityContainer::RemoveComponentFromEntity( const GameEntity& gameEntity )
	{
		assert( HasEntityComponent< T >( gameEntity ) );
		_entities.remove< T >( _ecsEntityId );
	}

	template < typename T >
	inline std::vector< GameEntity > EntityContainer::GetEntitiesOfType()
	{
		std::vector< GameEntity > entitiesFound;
		auto& view = _entities.view< T >();
		// TODO See why this size_hint is not working
		// entitiesFound.reserve(view.size_hint());

		for ( auto& entity : view )
		{
			entitiesFound.emplace_back( entity, this );
		}

		return entitiesFound;
	}

	template < typename T >
	inline const std::vector< GameEntity > EntityContainer::GetEntitiesOfType() const
	{
		std::vector< GameEntity > entitiesFound;
		auto& view = _entities.view< T >();

		// TODO See why this size_hint is not working
		// entitiesFound.reserve(view.size_hint());

		for ( auto& entity : view )
		{
			entitiesFound.emplace_back( entity, this );
		}

		return entitiesFound;
	}

	template < typename T >
	inline GameEntity EntityContainer::GetFirstEntityOfType()
	{
		return const_cast< GameEntity& >( static_cast< const EntityContainer& >( *this ).GetFirstEntityOfType< T >() );
	}

	// TODO For some reason this method throws a compilation error of type: C2440 '<function-style-cast>': cannot
	// convert from 'initializer list' to 'GameEntity. Edit: This issue has been solved by adding const_cast. However,
	// not a really good quality solution. Dev should think in something better.
	template < typename T >
	inline const GameEntity EntityContainer::GetFirstEntityOfType() const
	{
		auto& view = _entities.view< T >();
		assert( !view.empty() );
		return GameEntity( *view.begin(), const_cast< EntityContainer* >( this ) );
	}

	template < typename T1, typename T2 >
	inline std::vector< GameEntity > EntityContainer::GetEntitiesOfBothTypes()
	{
		std::vector< GameEntity > entitiesFound;
		auto& view = _entities.view< T1, T2 >();
		entitiesFound.reserve( view.size_hint() );

		for ( auto& entity : view )
		{
			entitiesFound.emplace_back( entity, this );
		}

		return entitiesFound;
	}

	// TODO For some reason this method throws a compilation error of type: C2440 '<function-style-cast>': cannot
	// convert from 'initializer list' to 'GameEntity. Edit: This issue has been solved by adding const_cast. However,
	// not a really good quality solution. Dev should think in something better.
	template < typename T1, typename T2 >
	inline const std::vector< GameEntity > EntityContainer::GetEntitiesOfBothTypes() const
	{
		std::vector< GameEntity > entitiesFound;
		auto& view = _entities.view< T1, T2 >();
		entitiesFound.reserve( view.size_hint() );

		for ( auto& entity : view )
		{
			entitiesFound.emplace_back( entity, const_cast< EntityContainer* >( this ) );
		}

		return entitiesFound;
	}

	template < typename T >
	inline T& EntityContainer::GetFirstComponentOfType()
	{
		return const_cast< T& >( static_cast< const EntityContainer& >( *this ).GetFirstComponentOfType< T >() );
	}

	template < typename T >
	inline const T& EntityContainer::GetFirstComponentOfType() const
	{
		GameEntity firstEntityOfType = GetFirstEntityOfType< T >();
		return firstEntityOfType.GetComponent< T >();
	}

	template < typename T >
	inline ComponentView< T > EntityContainer::GetComponentsOfType()
	{
		return ComponentView< T >( &_entities );
	}

	template < typename T >
	inline const ComponentView< T > EntityContainer::GetComponentsOfType() const
	{
		return ComponentView< T >( &_entities );
	}
} // namespace ECS
