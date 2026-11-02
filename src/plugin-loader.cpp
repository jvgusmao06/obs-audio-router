#include "obs-module-compat.hpp"
#include <windows.h>
#include <iostream>

static obs_module_t *g_obs_module = nullptr;

// Versão da API do OBS (OBS_VERSION encode: major << 24 | minor << 16 | patch)
// OBS 30.0.0+ = (30 << 24)
#define LIBOBS_API_VER ((30 << 24) | (0 << 16) | (0))

EXPORT_API uint32_t obs_module_ver(void) {
    return LIBOBS_API_VER;
}

EXPORT_API void obs_module_set_pointer(obs_module_t *module) {
    g_obs_module = module;
}

EXPORT_API bool obs_module_load(void) {
    OutputDebugStringA("[OBS Audio Router] obs_module_load chamado com sucesso!\n");
    return true;
}

EXPORT_API void obs_module_post_load(void) {
    OutputDebugStringA("[OBS Audio Router] obs_module_post_load chamado!\n");
}

EXPORT_API void obs_module_unload(void) {
    OutputDebugStringA("[OBS Audio Router] obs_module_unload chamado!\n");
    g_obs_module = nullptr;
}

EXPORT_API const char *obs_module_author(void) {
    return "Antigravity Team";
}

EXPORT_API const char *obs_module_name(void) {
    return "OBS Audio Router";
}

EXPORT_API const char *obs_module_description(void) {
    return "Application audio routing for OBS Studio";
}
