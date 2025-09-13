cmake_minimum_required(VERSION 3.16)
project(WallpaperIA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt6_standard_project_setup()

add_executable(WallpaperIA WIN32
    main.cpp
    resources.qrc
    app.rc
)

target_link_libraries(WallpaperIA Qt6::Core Qt6::Widgets Qt6::Network)