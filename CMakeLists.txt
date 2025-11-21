cmake_minimum_required(VERSION 3.16)
project(PageReplacementSimulatorAnimated LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

qt_add_executable(pager_gui
    main.cpp
    mainwindow.h
    mainwindow.cpp
    PageItem.h
    PageItem.cpp
    simulators.h
    simulators.cpp
)

target_link_libraries(pager_gui PRIVATE Qt6::Widgets)
