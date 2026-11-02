cmake_minimum_required(VERSION 3.20)
project(obs-audio-router VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Headers da libobs e obs-frontend-api
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/obs-src/obs-studio-30.0.0/libobs")
    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/obs-src/obs-studio-30.0.0/libobs"
        "${CMAKE_CURRENT_SOURCE_DIR}/obs-src/obs-studio-30.0.0/UI/obs-frontend-api"
    )
    link_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib"
    )
endif()

add_library(${PROJECT_NAME} MODULE
    src/plugin-main.cpp
    src/plugin-macros.hpp
    src/core/process-utils.hpp
    src/core/process-utils.cpp
    src/core/audio-session-watcher.hpp
    src/core/audio-session-watcher.cpp
    src/core/routing-rules.hpp
    src/core/routing-rules.cpp
    src/core/channel-audio-engine.hpp
    src/core/channel-audio-engine.cpp
    src/ui/localization-helper.hpp
    src/ui/app-badge-widget.hpp
    src/ui/app-badge-widget.cpp
    src/ui/channel-drop-zone.hpp
    src/ui/channel-drop-zone.cpp
    src/ui/audio-router-dock.hpp
    src/ui/audio-router-dock.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ole32
    psapi
)

# Vincula as bibliotecas obs e obs-frontend-api se encontradas
find_library(LIBOBS_LIB NAMES obs obs.dll HINTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit" "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin")
find_library(LIBOBS_FRONTEND_LIB NAMES obs-frontend-api obs-frontend-api.dll HINTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit" "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin")

if(LIBOBS_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBOBS_LIB})
endif()
if(LIBOBS_FRONTEND_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBOBS_FRONTEND_LIB})
endif()
