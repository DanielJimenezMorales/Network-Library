#include "remote_peer_inputs_handler.h"

#include <cassert>

#include "logger.h"

#include "inputs/i_input_state.h"

namespace NetLib
{
	// TODO: Make the IInputState to have fixed header fields (tick and serverTime) instead of letting the concrete
	// input class to define them. In that way we can check for tampered messages or order later to reduce jitter
	// effects. Then, in this method, we can check if input's tick and server time are valid.
	void RemotePeerInputsBuffer::AddInputState( IInputState* input )
	{
		assert( input != nullptr );
		_inputsBuffered.push( input );
	}

	const IInputState* RemotePeerInputsBuffer::PopNextInputState()
	{
		assert( GetNumberOfInputsBuffered() > 0 );

		IInputState* inputToReturn = _inputsBuffered.front();
		assert( inputToReturn != nullptr );

		_inputsBuffered.pop();

		if ( _lastInputPopped != nullptr )
		{
			delete _lastInputPopped;
		}

		_lastInputPopped = inputToReturn;
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

	const IInputState* RemotePeerInputsHandler::PopNextInputFromRemotePeer( uint32 remote_peer_id )
	{
		RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		assert( inputsBuffer != nullptr );

		if ( inputsBuffer->GetNumberOfInputsBuffered() == 0 )
		{
			return nullptr;
		}

		return inputsBuffer->PopNextInputState();
	}

	const IInputState* RemotePeerInputsHandler::GetLastInputPoppedFromRemotePeer( uint32 remote_peer_id ) const
	{
		const RemotePeerInputsBuffer* inputsBuffer = TryGetInputsBufferFromRemotePeerId( remote_peer_id );
		assert( inputsBuffer != nullptr );
		return inputsBuffer->GetLastInputPopped();
	}

	void RemotePeerInputsHandler::RemoveInputsBuffer( uint32 remote_peer_id )
	{
		auto remotePeerFoundIt = _remotePeerIdToInputsBufferMap.find( remote_peer_id );
		if ( remotePeerFoundIt != _remotePeerIdToInputsBufferMap.end() )
		{
			_remotePeerIdToInputsBufferMap.erase( remote_peer_id );
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
