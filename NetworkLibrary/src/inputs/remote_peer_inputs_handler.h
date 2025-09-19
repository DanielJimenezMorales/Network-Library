#pragma once
#include "numeric_types.h"

#include <unordered_map>
#include <queue>

namespace NetLib
{
	class IInputState;

	class RemotePeerInputsBuffer
	{
		public:
			RemotePeerInputsBuffer()
			    : _inputsBuffered()
			    , _lastInputPopped( nullptr )
			    , _isEnabled( true )
			{
			}
			void AddInputState( IInputState* input );
			const IInputState* PopNextInputState();
			const IInputState* GetLastInputPopped() const { return _lastInputPopped; }
			uint32 GetNumberOfInputsBuffered() const;
			void Enable();
			void Disable();
			bool GetAvailability() const;
			void Clear();

		private:
			std::queue< IInputState* > _inputsBuffered;
			IInputState* _lastInputPopped;
			bool _isEnabled;
	};

	/// <summary>
	/// This is a server-side class that will handle the inputs received from remote peers. It will ack as an input
	/// playout delay buffer to mitigate jitter effects by storing inputs until they are processed.
	/// </summary>
	class RemotePeerInputsHandler
	{
		public:
			bool CreateInputsBuffer( uint32 remote_peer_id );
			void AddInputState( IInputState* input, uint32 remote_peer_id );
			const IInputState* PopNextInputFromRemotePeer( uint32 remote_peer_id );
			const IInputState* GetLastInputPoppedFromRemotePeer( uint32 remote_peer_id ) const;
			void RemoveInputsBuffer( uint32 remote_peer_id );

			/// <summary>
			/// Enables the input buffer for a specific remote peer. That means that any incoming inputs from that
			/// remote peer will be stored.
			/// </summary>
			/// <param name="remote_peer_id">The remote peer id to disable its inputs buffer.</param>
			/// <returns>True if successfully enabled, False if remote peer was not found</returns>
			bool EnableInputBuffer( uint32 remote_peer_id );

			/// <summary>
			/// Disables the input buffer for a specific remote peer. That means that any incoming inputs from that
			/// remote peer will be automatically discarded.
			/// </summary>
			/// <param name="remote_peer_id">The remote peer id to disable its inputs buffer.</param>
			/// <returns>True if successfully disabled, False if remote peer was not found</returns>
			bool DisableInputBuffer( uint32 remote_peer_id );

			bool GetInputsBufferAvailability( uint32 remote_peer_id ) const;

		private:
			RemotePeerInputsBuffer* TryGetInputsBufferFromRemotePeerId( uint32 remote_peer_id );
			const RemotePeerInputsBuffer* TryGetInputsBufferFromRemotePeerId( uint32 remote_peer_id ) const;

			std::unordered_map< uint32, RemotePeerInputsBuffer > _remotePeerIdToInputsBufferMap;
	};
} // namespace NetLib
