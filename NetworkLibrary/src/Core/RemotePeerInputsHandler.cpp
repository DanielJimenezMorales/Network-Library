#include "RemotePeerInputsHandler.h"
#include "IInputState.h"
#include <cassert>

namespace NetLib
{
	void RemotePeerInputsBuffer::AddInputState(IInputState* input)
	{
		assert(input != nullptr);
		_inputsBuffered.push(input);
	}

	IInputState* RemotePeerInputsBuffer::GetNextInputState()
	{
		assert(GetNumberOfInputsBuffered() > 0);

		IInputState* inputToReturn = _inputsBuffered.front();
		_inputsBuffered.pop();
		return inputToReturn;
	}

	uint32_t RemotePeerInputsBuffer::GetNumberOfInputsBuffered() const
	{
		return _inputsBuffered.size();
	}

	void RemotePeerInputsHandler::AddInputState(IInputState* input, uint32_t remotePeerId)
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find(remotePeerId);
		if (remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end())
		{
			remotePeerFoundIt->second.AddInputState(input);
		}
		else
		{
			_remotePeerIdToInputsBufferMap[remotePeerId] = RemotePeerInputsBuffer();
			_remotePeerIdToInputsBufferMap[remotePeerId].AddInputState(input);
		}
	}

	const IInputState* RemotePeerInputsHandler::GetNextInputFromRemotePeer(uint32_t remotePeerId)
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find(remotePeerId);
		if (remotePeerFoundIt == _remotePeerIdToInputsBufferMap.end())
		{
			return nullptr;
		}

		if (remotePeerFoundIt->second.GetNumberOfInputsBuffered() == 0)
		{
			return nullptr;
		}

		return remotePeerFoundIt->second.GetNextInputState();
	}

	void RemotePeerInputsHandler::RemoveRemotePeer(uint32_t remotePeerId)
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find(remotePeerId);
		if (remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end())
		{
			_remotePeerIdToInputsBufferMap.erase(remotePeerId);
		}
	}
}
