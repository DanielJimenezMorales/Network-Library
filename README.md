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
6. World replication
   - Network variables
   - Automatic network change tracking
   - ECS friendly


## Demo game features: (IN PROGRESS)

## How to get it working:
1. Download the project locally (Fork, clone, copy & paste...)
2. Generate project files using premake5. In my case, I use Visual Studio 2022, so I will open CMD, navigate to the repository folder, and type "vendor/premake5/premake5 vs2022". However, you will need to replace vs2022 with your preferred option.
3. Choose which project you want to set as the Start Up (DemoGame or Tests), compile and generate the .exe file.
4. If you wish to run Tests, simply open its .exe file. However, if you want to run DemoGame, you'll need to copy SDL2 dll files from "vendor/sdl" into Demogame .exe folder.