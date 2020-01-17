cmake_minimum_required(VERSION 3.15)
project(KIV_UPS_sp_server)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(KIV_UPS_sp_server
        src/system/Logger.cpp src/system/Logger.hpp
        src/system/logger_helper.hpp

        src/system/signal.hpp
        src/system/defaults.hpp

        src/system/main.cpp
        src/system/argument_parser.cpp

        src/network/protocol.hpp
        src/network/server_handler.cpp
        src/network/packet_handler.hpp
        src/network/Server.cpp src/network/Server.hpp

        src/network/ClientManager.cpp src/network/ClientManager.hpp
        src/network/Client.cpp src/network/Client.hpp

        src/game/Lobby.cpp src/game/Lobby.hpp
        src/game/RoomHnefatafl.cpp src/game/RoomHnefatafl.hpp
        )

find_package(Threads REQUIRED)
target_link_libraries(KIV_UPS_sp_server Threads::Threads)
