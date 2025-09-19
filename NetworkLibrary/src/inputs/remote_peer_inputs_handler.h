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
			    , _isEnabled( true )
			{
			}
			void AddInputState( IInputState* input );
			IInputState* GetNextInputState();
			uint32 GetNumberOfInputsBuffered() const;
			void Enable();
			void Disable();
			bool GetAvailability() const;
			void Clear();

		private:
			std::queue< IInputState* > _inputsBuffered;
			bool _isEnabled;
	};

	class RemotePeerInputsHandler
	{
		public:
			bool CreateInputsBuffer( uint32 remote_peer_id );
			void AddInputState( IInputState* input, uint32 remote_peer_id );
			const IInputState* GetNextInputFromRemotePeer( uint32 remotePeerId );
			void RemoveRemotePeer( uint32 remotePeerId );

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
