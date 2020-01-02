cmake_minimum_required(VERSION 3.15)
project(KIV_UPS_sp_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(KIV_UPS_sp_server src/main.cpp src/Server.cpp src/Server.hpp src/Logger.cpp src/Logger.hpp src/Helper.hpp)
#add_executable(KIV_UPS_sp_server src/main.cpp src/Server.cpp src/Server.hpp src/Logger.cpp src/Logger.hpp src/Client.cpp src/Client.hpp)
#add_executable(KIV_UPS_sp_server tmp/tmp.cpp tmp/tmp.hpp tmp/tmp1.cpp tmp/tmp2.cpp)