#include "plugin-macros.hpp"
#include "ui/audio-router-dock.hpp"
#include "ui/localization-helper.hpp"
#include <obs-frontend-api.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

static AudioRouter::AudioRouterDock *g_audioRouterDock = nullptr;

static void setup_localization(void) {
    const char *locale = obs_get_locale();
    std::string locStr = locale ? locale : "en-US";

    char *pluginDataPath = obs_module_file("");
    std::string basePath = pluginDataPath ? pluginDataPath : "data/";
    bfree(pluginDataPath);

    std::string localeFile = basePath + "locale/" + locStr + ".ini";
    AudioRouter::I18nHelper::instance().loadLanguage(localeFile);
}

static void on_frontend_event(enum obs_frontend_event event, void *private_data) {
    if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        setup_localization();
        blog(LOG_INFO, "OBS loaded. Initializing Audio Router Dock...");
        
        auto *mainWindow = static_cast<QWidget*>(obs_frontend_get_main_window());
        if (!g_audioRouterDock && mainWindow) {
            g_audioRouterDock = new AudioRouter::AudioRouterDock(mainWindow);
            obs_frontend_add_dock_by_id("obs_audio_router_dock", obs_module_text("DockTitle"), g_audioRouterDock);
            blog(LOG_INFO, "Audio Router Dock successfully registered.");
        }
    }
}

bool obs_module_load(void) {
    blog(LOG_INFO, "Loading plugin %s v%s...", PLUGIN_NAME, PLUGIN_VERSION);
    obs_frontend_add_event_callback(on_frontend_event, nullptr);
    return true;
}

void obs_module_unload(void) {
    blog(LOG_INFO, "Unloading plugin %s...", PLUGIN_NAME);
    obs_frontend_remove_event_callback(on_frontend_event, nullptr);
    g_audioRouterDock = nullptr;
}

MODULE_EXPORT const char *obs_module_author(void) {
    return "Antigravity Team & Community";
}

MODULE_EXPORT const char *obs_module_name(void) {
    return "OBS Audio Router";
}

MODULE_EXPORT const char *obs_module_description(void) {
    return "Intelligent per-application audio routing";
}
