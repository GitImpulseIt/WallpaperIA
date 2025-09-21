cmake_minimum_required(VERSION 3.16)
project(WallpaperIA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug symbols and compile commands for VSCode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g")
    # Link statically with runtime libraries for better portability
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Svg)

qt6_standard_project_setup()

add_executable(WallpaperIA WIN32
    main.cpp
    src/utils/utils.cpp
    src/system/wallpaper_manager.cpp
    src/core/wallpaper_builder.cpp
    src/widgets/hover_filters.cpp
    src/widgets/screen_selector.cpp
    src/widgets/countdown_widget.cpp
    src/widgets/toggle_switch.cpp
    resources.qrc
    app.rc
)

target_link_libraries(WallpaperIA Qt6::Core Qt6::Widgets Qt6::Network Qt6::Svg)