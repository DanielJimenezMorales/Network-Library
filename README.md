# Network-Library
Network library for realtime multiplayer games written in C++

This repository contains four different projects:
1. Network library
2. Demo game. [Docs](docs/demo_game/demo_game_index.md)
3. Common (Shared files between Demo game and Network Library projects)
4. Tests

## Network library Features:
Implementation Legend: 
🔲 Not Yet 🔰 Partially ✅ Done ❌ Won't do

### Main
- ✅ UDP protocol
- ✅ RUDP protocol
- 🔰 Connection pipeline (It's not customizable)
- ✅ Time Synchronization
- ✅ World Replication
- 🔰 Server-Side Inputs Buffer (Naive version)
- 🔲 RPCs
- 🔲 Delta Snapshots

### Transmission channels support
- ✅ Reliable Ordered
- ✅ Unreliable Unordered
- 🔰 Reliable Unordered
- 🔲/❌ Unreliable Ordered

### Reliability
- ✅ Message Level ACKs
- ✅ Dynamic Message Retransmission Timeout (based on connection's RTT)

### Network Metrics
- ✅ Latency
- ✅ Jitter
- ✅ Packet Loss
- ✅ Upload Bandwidth
- ✅ Download Bandwidth
- 🔲 Upload Bandwidth per Channel
- 🔲 Download Bandwidth per Channel
- ✅ Out Of Order Count
- ✅ Retransmissions Count
- ✅ Duplicates Count

## How to get it working:
1. Download the project locally (Fork, clone, copy & paste...)
2. Generate project files using premake5. In my case, I use Visual Studio 2022, so I will open CMD, navigate to the repository folder, and type "vendor/premake5/premake5 vs2022". However, you will need to replace vs2022 with your preferred option.
3. Choose which project you want to set as the Start Up (DemoGame or Tests), compile and generate the .exe file.
4. If you wish to run Tests, simply open its .exe file. However, if you want to run DemoGame, you'll need to copy SDL2 dll files from "vendor/sdl" into Demogame .exe folder.