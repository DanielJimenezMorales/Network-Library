#include "remote_peer_inputs_handler.h"

#include <cassert>

#include "logger.h"

#include "inputs/i_input_state.h"

namespace NetLib
{
	void RemotePeerInputsBuffer::AddInputState( IInputState* input )
	{
		assert( input != nullptr );
		_inputsBuffered.push( input );
	}

	IInputState* RemotePeerInputsBuffer::GetNextInputState()
	{
		assert( GetNumberOfInputsBuffered() > 0 );

		IInputState* inputToReturn = _inputsBuffered.front();
		_inputsBuffered.pop();
		return inputToReturn;
	}

	uint32 RemotePeerInputsBuffer::GetNumberOfInputsBuffered() const
	{
		return static_cast< uint32 >( _inputsBuffered.size() );
	}

	void RemotePeerInputsBuffer::Enable()
	{
		_isEnabled = true;
	}

	void RemotePeerInputsBuffer::Disable()
	{
		_isEnabled = false;
	}

	bool RemotePeerInputsBuffer::GetAvailability() const
	{
		return _isEnabled;
	}

	void RemotePeerInputsBuffer::Clear()
	{
		// TODO queue.clear does not exist. Use your own queue implementation...
		_inputsBuffered = {};
	}

	bool RemotePeerInputsHandler::CreateInputsBuffer( uint32 remote_peer_id )
	{
		bool result = false;

		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		if ( inputsBuffer == nullptr )
		{
			_remotePeerIdToInputsBufferMap[ remote_peer_id ] = RemotePeerInputsBuffer();
			result = true;
		}

		return result;
	}

	void RemotePeerInputsHandler::AddInputState( IInputState* input, uint32 remote_peer_id )
	{
		assert( input != nullptr );

		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		assert( inputsBuffer != nullptr );
		inputsBuffer->AddInputState( input );
	}

	const IInputState* RemotePeerInputsHandler::GetNextInputFromRemotePeer( uint32 remote_peer_id )
	{
		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		assert( inputsBuffer != nullptr );

		if ( inputsBuffer->GetNumberOfInputsBuffered() == 0 )
		{
			return nullptr;
		}

		return inputsBuffer->GetNextInputState();
	}

	void RemotePeerInputsHandler::RemoveRemotePeer( uint32 remotePeerId )
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find( remotePeerId );
		if ( remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end() )
		{
			_remotePeerIdToInputsBufferMap.erase( remotePeerId );
		}
	}

	bool RemotePeerInputsHandler::EnableInputBuffer( uint32 remote_peer_id )
	{
		bool result = false;
		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		if ( inputsBuffer != nullptr )
		{
			result = true;
			inputsBuffer->Enable();
		}
		else
		{
			LOG_ERROR( "RemotePeerInputsHandler::%s Could not find inputs buffer for remote peer id %u",
			           THIS_FUNCTION_NAME, remote_peer_id );
		}

		return result;
	}

	bool RemotePeerInputsHandler::DisableInputBuffer( uint32 remote_peer_id )
	{
		bool result = false;
		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		if ( inputsBuffer != nullptr )
		{
			inputsBuffer->Clear();
			inputsBuffer->Disable();
			result = true;
		}
		else
		{
			LOG_ERROR( "RemotePeerInputsHandler::%s Could not find inputs buffer for remote peer id %u",
			           THIS_FUNCTION_NAME, remote_peer_id );
		}

		return result;
	}

	bool RemotePeerInputsHandler::GetInputsBufferAvailability( uint32 remote_peer_id ) const
	{
		bool result = false;
		const RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		if ( inputsBuffer != nullptr )
		{
			result = inputsBuffer->GetAvailability();
		}
		else
		{
			LOG_ERROR( "RemotePeerInputsHandler::%s Could not find inputs buffer for remote peer id %u",
			           THIS_FUNCTION_NAME, remote_peer_id );
		}

		return result;
	}

	RemotePeerInputsBuffer* RemotePeerInputsHandler::TryGetInputsBufferFromRemotePeerId( uint32 remote_peer_id )
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find( remote_peer_id );
		return ( remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end() ) ? &remotePeerFoundIt->second : nullptr;
	}

	const RemotePeerInputsBuffer* RemotePeerInputsHandler::TryGetInputsBufferFromRemotePeerId(
	    uint32 remote_peer_id ) const
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find( remote_peer_id );
		return ( remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end() ) ? &remotePeerFoundIt->second : nullptr;
	}
} // namespace NetLib
