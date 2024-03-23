# Network-Library
Network library for realtime multiplayer games written in C++

Features:
1. Connection pipeline
2. Transmission channels support
   - Unreliable unordered transmission support
   - Unreliable ordered transmission support
   - Reliable ordered transmission support
3. Reliability:
   - Message level ACKs
   - Dynamic retransmission timeout based on connection's RTT
4. Time sync
