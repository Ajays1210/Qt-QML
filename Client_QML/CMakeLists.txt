cmake_minimum_required(VERSION 3.16)
project(ClientQML LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ClientQML
    main.cpp
    protocol.h
    model.h
    model.cpp
    controller.h
    controller.cpp
    tcpsender.h
    tcpsender.cpp
    udpreceiver.h
    udpreceiver.cpp
)

qt_add_qml_module(ClientQML
    URI Client
    VERSION 1.0
    QML_FILES
    qml/Main.qml
)

target_link_libraries(ClientQML PRIVATE Qt6::Quick Qt6::Network)
