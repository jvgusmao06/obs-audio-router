#pragma once

#include <cstdint>

#ifdef _WIN32
  #define EXPORT_API __declspec(dllexport)
#else
  #define EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void obs_module_t;

// Assinaturas exatas que o OBS Studio busca na DLL para carregar um plugin
EXPORT_API uint32_t obs_module_ver(void);
EXPORT_API void obs_module_set_pointer(obs_module_t *module);
EXPORT_API bool obs_module_load(void);
EXPORT_API void obs_module_unload(void);
EXPORT_API void obs_module_post_load(void);
EXPORT_API const char *obs_module_author(void);
EXPORT_API const char *obs_module_name(void);
EXPORT_API const char *obs_module_description(void);

#ifdef __cplusplus
}
#endif

