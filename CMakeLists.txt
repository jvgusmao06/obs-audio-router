# ==============================================================================
# OBS Audio Router Plugin - CMake Build Configuration
# Builds the plugin binary (obs-audio-router.dll) and test targets with Qt6.
# ==============================================================================

cmake_minimum_required(VERSION 3.20)
project(obs-audio-router VERSION 0.2.3 LANGUAGES CXX)

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
    src/ui/chatmix-slider.hpp
    src/ui/chatmix-slider.cpp
    src/ui/channel-strip-widget.hpp
    src/ui/channel-strip-widget.cpp
    src/ui/app-badge-widget.hpp
    src/ui/app-badge-widget.cpp
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

# Vincula as bibliotecas obs e obs-frontend-api geradas
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit/obs.lib")
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit/obs.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/64bit/obs-frontend-api.lib"
    )
endif()

enable_testing()

add_executable(test_i18n_and_rules
    tests/test_i18n_and_rules.cpp
    src/core/routing-rules.cpp
)
target_include_directories(test_i18n_and_rules PRIVATE src)

add_executable(test_chatmix_and_volumes
    tests/test_chatmix_and_volumes.cpp
    src/core/channel-audio-engine.cpp
    src/core/routing-rules.cpp
    src/core/audio-session-watcher.cpp
    src/core/process-utils.cpp
)
target_include_directories(test_chatmix_and_volumes PRIVATE src)
if(WIN32)
    target_link_libraries(test_chatmix_and_volumes PRIVATE ole32 psapi)
endif()

add_test(NAME test_i18n_and_rules COMMAND test_i18n_and_rules)
add_test(NAME test_chatmix_and_volumes COMMAND test_chatmix_and_volumes)
