# Execution flow
This library operates through four well-defined phases: **Start → PreTick → Tick → Stop**.
Each phase has a specific responsibility and must not perform tasks belonging to other phases.  
This ensures predictable behavior, easier debugging, and a clearer architecture.
![Execution flow diagram](./images/network_library_flow_diagram.png)

## Index:
- [Start Phase](#Representation)
- [PreTick Phase](#Parent-Child-Hierarchies)
- [Tick Phase](#Local-vs-Global-Transforms)
- [Stop Phase](#Transform-Component-Proxy)

## Start
Initialize the peer and all required internal systems in preparation for the network loop.  
This phase is executed **once**, before the first tick.

### 1. Set connection state to Connecting
Transition the peer's connection state from `Disconnected` to `Connecting`.
#### HOW
- Set local peer state to `Connecting`.
- Note:    
    - **Server**: promoted later to `Connected` inside this Start process.
    -  **Client**: remains in `Connecting` until receiving either _ConnectionAccepted_ or _ConnectionDenied_ from the server.
### 2. Start socket
Create and initialize the underlying network socket.
#### HOW
- Initialize Berkeley Sockets API.
- Create socket object.
- Configure blocking / non-blocking behavior.
### 3. Start peer-type specific logic
Initialize systems exclusive to the peer type (Client or Server).
#### HOW (Server)
- Bind the socket to listen on all incoming IPs.
#### HOW (Client)
- Bind the socket to only listen on server's IP.
- Generate communication salt number.
- Create the server's _early remote peer_.
### 4. Set current tick
Initialize simulation tick counter to `1`.

## PreTick
Receive network data, validate it, build packets, and place messages into transmission channels and process them.
This phase **does NOT**:
- Send any data.

### 1. Read received data
Read incoming datagrams and convert them into `NetworkPacket` objects. Then stores them within the transmission channels to be ready for being processed. This receive pipeline does not process messages yet.
#### HOW
- For each datagram received:
	1. Read raw data from socket.
	2. Validate datagram.
	3. Construct `NetworkPacket`.
	4. Route its messages to:
	    - an existing `RemotePeer`, or
	    - an `EarlyRemotePeer` (peer still in the connection/handshake process).
### 2. Process received data
Interpret and execute pending messages that were queued during data reception. This is where connection requests, ping-pongs, acknowledgments, and replication messages are consumed.
#### HOW
- For each **RemotePeer**:
	- For each pending message:
	    - Process message.
- For each **EarlyRemotePeer** (⚠️ currently missing):
	- For each pending message:
	    - Process message.

## Tick
Advance internal systems, update peer state, build outgoing data, send packets, and handle disconnections.
This phase **does NOT**:
- Read incoming socket data

### 1. Update remote peers
Update systems that belong to remote peers.
#### HOW
- For each Remote Peer:
	- **Peer lifecycle systems**
		- Update inactivity system.
		- Detect conditions to initiate disconnection.
	- **Peer communication systems**
		- Update transmission channels.
		- Update metrics / telemetry.
		- Update Ping-Pong.
### 2. Update peer-type specific logic
Execute logic specific to client or server.
#### HOW (Server)
- Update replication component.
#### HOW (Client)
- If connection state `Disconnected`, enqueue a connection request.
- Update time syncer component.
### 3. Finish disconnecting remote peers
Finalize disconnection of remote peers that entered the disconnecting state.
#### HOW
For each disconnection request:
- If `shouldNotify`, send a disconnection message.
- Perform disconnection cleanup.
### 4. Send pending data
Build and transmit outgoing packets for all remote peers and all their transmission channels.
#### HOW
- For each **RemotePeer**:
	- For each transmission channel:
	    - Build and send a network packet.
- For each **EarlyRemotePeer** (⚠️ currently missing):
	- For each transmission channel:
	    - Build and send a network packet.
#### Notes
- A **separate packet is built per transmission channel**.
- Messages from different channels are **never mixed** in the same packet.
### 5. Stop peer, if requested
If a stop has been requested during this tick, transition the system toward the Stop phase.

## Stop
Shut down the peer and release all network resources.

### 1. Stop peer-type specific logic
Shutdown logic specific to client or server.
#### HOW (Server)
- None
#### HOW (Client)
- None
### 2. Disconnect remote peers
Remove all remote peers and optionally notify them of the disconnection.
#### HOW
- For each remote peer:
	- If `shouldNotify`, send Disconnect message.
- Remove all remote peers.
### 3. Close socket
Release all socket API resources.
#### HOW
- Destroy socket object.
- Deinitialize socket API.
### 4. Set connection state to Disconnected
Mark the peer as fully disconnected by setting the connection state to `Disconnected`