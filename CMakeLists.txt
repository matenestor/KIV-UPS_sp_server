cmake_minimum_required(VERSION 3.15)
project(KIV_UPS_sp_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(KIV_UPS_sp_server
        src/Logger.cpp src/Logger.hpp
        src/main.cpp
        src/ServerHandler.cpp src/ServerHandler.hpp
        src/Server.cpp src/Server.hpp
        src/ClientManager.cpp src/ClientManager.hpp
        src/Client.cpp src/Client.hpp
        src/Lobby.cpp src/Lobby.hpp
        src/GameRoom.cpp src/GameRoom.hpp
        src/RoomHnefatafl.cpp src/RoomHnefatafl.hpp
        src/Helper.hpp
)

# main
#add_executable(KIV_UPS_sp_server src/main.cpp src/Logger.cpp src/Logger.hpp src/Helper.hpp)

# tmp
#add_executable(KIV_UPS_sp_server tmp/tmp.cpp tmp/tmp.hpp tmp/tmp1.cpp tmp/tmp2.cpp)