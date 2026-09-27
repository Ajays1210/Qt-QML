cmake_minimum_required(VERSION 3.16)
project(ClientWidget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_standard_project_setup()

qt_add_executable(ClientWidget
    main.cpp
    protocol.h
    network.h
    network.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(ClientWidget PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
