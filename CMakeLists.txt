cmake_minimum_required(VERSION 3.20)
project(obs-audio-router VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Dependências: Qt6 (Padrão para OBS Studio 29+)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Dependência: libobs & obs-frontend-api
# Pode ser fornecido apontando -Dobs-studio_DIR=<path-to-obs-cmake>
find_package(libobs QUIET)
find_package(obs-frontend-api QUIET)

if(NOT TARGET libobs)
    message(STATUS "libobs não encontrado via find_package. Tentando caminhos padrões no Windows...")
    set(OBS_PATH "C:/Program Files/obs-studio" CACHE PATH "Caminho do OBS Studio")
    include_directories(
        "${OBS_PATH}/include"
        "${OBS_PATH}/UI/obs-frontend-api"
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

if(TARGET libobs)
    target_link_libraries(${PROJECT_NAME} PRIVATE libobs obs-frontend-api)
endif()

# Teste unitário para o motor de regras
enable_testing()
add_executable(test_routing_rules
    tests/test_routing_rules.cpp
    src/core/routing-rules.cpp
)
target_include_directories(test_routing_rules PRIVATE src)
add_test(NAME TestRoutingRules COMMAND test_routing_rules)

