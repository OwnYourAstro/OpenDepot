cmake_minimum_required(VERSION 3.20)
project(OpenDepot CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W3 /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /DWIN32_LEAN_AND_MEAN)
endif()

set(OPENDEPOT_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OpenDepotCore"
    CACHE PATH "Path to the OpenDepotCore checkout")
if(NOT EXISTS "${OPENDEPOT_CORE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "OpenDepotCore not found at ${OPENDEPOT_CORE_DIR}. Set OPENDEPOT_CORE_DIR.")
endif()
add_subdirectory("${OPENDEPOT_CORE_DIR}" "${CMAKE_BINARY_DIR}/opendepotcore")

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")

if(WIN32)
    file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico" ICON_PATH)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/app.rc" @ONLY)
    set(APP_RC "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
endif()

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)
target_include_directories(imgui PUBLIC "${IMGUI_DIR}" "${IMGUI_DIR}/backends")
if(MSVC)
    target_compile_options(imgui PRIVATE /W0)
endif()

add_executable(OpenDepot WIN32
    src/main_gui.cpp
    ${APP_RC}
)
target_include_directories(OpenDepot PRIVATE src)
target_compile_definitions(OpenDepot PRIVATE UNICODE _UNICODE)
target_link_libraries(OpenDepot PRIVATE
    imgui
    opendepot_core
    d3d11 dxgi dwmapi ole32 advapi32 comdlg32 user32 gdi32 shell32
)

# The ImGui GUI drives radio and file operations through opendepot-cli, which is
# built by the core subdirectory. Keep it next to OpenDepot.exe until M5 links
# the core directly.
if(TARGET opendepot-cli)
    add_custom_command(TARGET OpenDepot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:opendepot-cli> $<TARGET_FILE_DIR:OpenDepot>
        VERBATIM)
endif()
