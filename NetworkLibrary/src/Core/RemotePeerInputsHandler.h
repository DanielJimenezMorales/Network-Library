#pragma once
#include "NumericTypes.h"
#include <unordered_map>
#include <queue>

namespace NetLib
{
	class IInputState;

	class RemotePeerInputsBuffer
	{
	public:
		void AddInputState(IInputState* input);
		IInputState* GetNextInputState();
		uint32 GetNumberOfInputsBuffered() const;
	private:
		std::queue<IInputState*> _inputsBuffered;
	};

	class RemotePeerInputsHandler
	{
	public:
		void AddInputState(IInputState* input, uint32 remotePeerId);
		const IInputState* GetNextInputFromRemotePeer(uint32 remotePeerId);
		void RemoveRemotePeer(uint32 remotePeerId);
	private:
		std::unordered_map<uint32, RemotePeerInputsBuffer> _remotePeerIdToInputsBufferMap;
	};
}
