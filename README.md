# Network-Library
Network library for realtime multiplayer games written in C++

This repository contains four different projects:
1. Network library
2. Demo game
3. Common (Shared files between Demo game and Network Library projects)
4. Tests

## Network library features:
1. Supported protocols:
   - UDP
   - RUDP
2. Connection pipeline
3. Transmission channels support
   - Unreliable unordered transmission support
   - Unreliable ordered transmission support
   - Reliable ordered transmission support
4. Reliability:
   - Message level ACKs
   - Dynamic retransmission timeout based on connection's RTT
5. Time sync

## Demo game features: (IN PROGRESS)
