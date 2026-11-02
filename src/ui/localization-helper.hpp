/**
 * @file localization-helper.hpp
 * @brief Lightweight internationalization (i18n) helper parsing INI locale files
 *        and providing fallback string lookups.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace AudioRouter {

class I18nHelper {
public:
    static I18nHelper& instance() {
        static I18nHelper s_instance;
        return s_instance;
    }

    void loadLanguage(const std::string& localeFilePath) {
        std::ifstream file(localeFilePath);
        if (!file.is_open()) return;

        strings.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            // Remove aspas se houver
            if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2);
            }
            strings[key] = val;
        }
    }

    std::string text(const std::string& key, const std::string& defaultFallback = "") const {
        auto it = strings.find(key);
        if (it != strings.end()) {
            return it->second;
        }
        return defaultFallback.empty() ? key : defaultFallback;
    }

private:
    I18nHelper() = default;
    std::unordered_map<std::string, std::string> strings;
};

inline std::string loc(const std::string& key, const std::string& defaultVal = "") {
    return I18nHelper::instance().text(key, defaultVal);
}

} // namespace AudioRouter

