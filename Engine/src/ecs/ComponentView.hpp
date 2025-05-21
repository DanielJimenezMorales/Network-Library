#pragma once
#include <cassert>
#include "entt.hpp"

namespace Engine
{
	namespace ECS
	{
		template < typename T >
		class ComponentView
		{
			public:
				ComponentView( entt::registry* registry )
				    : registry( registry )
				    , view( registry->view< T >() )
				    , iterator( view.begin() )
				{
				}

				bool ArePendingComponents() const;
				T& GetNext();
				const T& GetNext() const;

			private:
				entt::registry* registry;
				decltype( registry->view< T >() ) view;
				decltype( view.begin() ) iterator;
		};

		template < typename T >
		inline bool ComponentView< T >::ArePendingComponents() const
		{
			return iterator != view.end();
		}

		template < typename T >
		inline T& ComponentView< T >::GetNext()
		{
			assert( ArePendingComponents() );

			T& nextComponent = view.get< T >( *iterator );
			++iterator;

			return nextComponent;
		}

		template < typename T >
		inline const T& ComponentView< T >::GetNext() const
		{
			assert( ArePendingComponents() );

			T& nextComponent = view.get< T >( *iterator );
			++iterator;

			return nextComponent;
		}
	} // namespace ECS
} // namespace Engine
