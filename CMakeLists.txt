cmake_minimum_required(VERSION 3.21)
project(OpenDepot CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W3 /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /DWIN32_LEAN_AND_MEAN)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set(OPENDEPOT_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OpenDepotCore"
    CACHE PATH "Path to the OpenDepotCore checkout")
if(NOT EXISTS "${OPENDEPOT_CORE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "OpenDepotCore not found at ${OPENDEPOT_CORE_DIR}. Set OPENDEPOT_CORE_DIR.")
endif()
add_subdirectory("${OPENDEPOT_CORE_DIR}" "${CMAKE_BINARY_DIR}/opendepotcore")

qt_add_executable(OpenDepot WIN32
    ui/main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/resources.qrc
)
target_include_directories(OpenDepot PRIVATE ui)
target_link_libraries(OpenDepot PRIVATE Qt6::Widgets opendepot_core)
