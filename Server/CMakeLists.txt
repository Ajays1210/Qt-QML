cmake_minimum_required(VERSION 3.16)
project(Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(Server
    main.cpp
    protocol.h
    server.h
    server.cpp
    tcplistener.h
    tcplistener.cpp
    udpbroadcaster.h
    udpbroadcaster.cpp
)

target_link_libraries(Server PRIVATE Qt6::Core Qt6::Network)
