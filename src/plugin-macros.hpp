/**
 * @file plugin-macros.hpp
 * @brief Global plugin metadata macros and logging helpers.
 */

#pragma once

#include <obs-module.h>

#define PLUGIN_NAME "obs-audio-router"
#define PLUGIN_VERSION "0.3.1"

#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
