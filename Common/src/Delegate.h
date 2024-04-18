#pragma once
#include <vector>

#include "fixed_size_function.hpp"
#include "Logger.h"

namespace Common
{
	//This is a no allocation multicast delegate. It works like a C# Action delegate so it always return void and it receives from 0 to X parameters :)
	//TODO make this class thread-safe
	template<typename... Parameters>
	class Delegate
	{
	public:
		Delegate()
		{
			const unsigned int initialSize = 3;
			Init(initialSize);
		}

		template<typename Functor>
		unsigned int AddSubscriber(Functor&& function)
		{
			//TODO add mutex here
			int index = -1;
			for (size_t i = 0; i < _subscribers.size(); ++i)
			{
				if (!_subscribers[i])
				{
					index = i;
					break;
				}
			}
			
			unsigned int idToReturn = 0;
			if (index != -1)
			{
				_subscribers[index] = std::forward<Functor>(function);
				idToReturn = _ids[index];
			}
			else
			{
				_subscribers.emplace_back(std::forward<Functor>(function));
				_ids.emplace_back(nextId);
				idToReturn = nextId;
				IncreaseId();
			}

			return idToReturn;
		}

		void DeleteSubscriber(unsigned int id)
		{
			//TODO add mutex here
			for (size_t i = 0; i < _ids.size(); ++i)
			{
				if (_ids[i] == id)
				{
					_subscribers[i].reset();
					break;
				}
			}
		}

		void DeleteAllSubscribers()
		{
			//TODO add mutex here
			for (size_t i = 0; i < _subscribers.size(); ++i)
			{
				_subscribers[i].reset();
			}
		}

		void Execute(Parameters... params)
		{
			//TODO make this thread-safe. Maybe a temp copy of the subscribers in order to safely loop them?
			for (unsigned int i = 0; i < _subscribers.size(); ++i)
			{
				if (_subscribers[i])
				{
					_subscribers[i](params...);
				}
			}
		}

	private:
		void Init(unsigned int initialSize)
		{
			_subscribers.reserve(initialSize);
			_ids.reserve(initialSize);

			for (unsigned int i = 0; i < initialSize; ++i)
			{
				_subscribers.emplace_back();
				_ids.emplace_back(nextId);
				IncreaseId();
			}
		}

		void IncreaseId()
		{
			//TODO Add mutex here in order to support thread-safe
			++nextId;
		}

		std::vector<unsigned int> _ids;
		std::vector<fixed_size_function<void(Parameters...), 128, construct_type::copy_and_move>> _subscribers;
		static unsigned int nextId;
	};

	template<typename... Parameters>
	unsigned int Delegate<Parameters...>::nextId = 0;
}
