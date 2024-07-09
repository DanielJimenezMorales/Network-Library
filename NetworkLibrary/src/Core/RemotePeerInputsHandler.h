#pragma once
#include <cstdint>
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
		uint32_t GetNumberOfInputsBuffered() const;
	private:
		std::queue<IInputState*> _inputsBuffered;
	};

	class RemotePeerInputsHandler
	{
	public:
		void AddInputState(IInputState* input, uint32_t remotePeerId);
		const IInputState* GetNextInputFromRemotePeer(uint32_t remotePeerId);
		void RemoveRemotePeer(uint32_t remotePeerId);
	private:
		std::unordered_map<uint32_t, RemotePeerInputsBuffer> _remotePeerIdToInputsBufferMap;
	};
}
