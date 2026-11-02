/**
 * @file process-utils.cpp
 * @brief Implementation of process resolution helpers using Windows PSAPI.
 */

#include "process-utils.hpp"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#include <algorithm>
#include <filesystem>

namespace AudioRouter {

std::string ProcessUtils::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return str;
}

std::string ProcessUtils::getProcessPath(uint32_t processId) {
    if (processId == 0) {
        return "System";
    }

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(processId));
    if (!hProcess) {
        return "";
    }

    wchar_t pathBuffer[MAX_PATH];
    DWORD size = MAX_PATH;
    std::string result;

    if (QueryFullProcessImageNameW(hProcess, 0, pathBuffer, &size)) {
        // Conversão simples wchar -> utf8
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, pathBuffer, -1, nullptr, 0, nullptr, nullptr);
        if (utf8Size > 0) {
            result.resize(utf8Size - 1);
            WideCharToMultiByte(CP_UTF8, 0, pathBuffer, -1, &result[0], utf8Size, nullptr, nullptr);
        }
    }

    CloseHandle(hProcess);
    return result;
#else
    return "";
#endif
}

std::string ProcessUtils::getProcessName(uint32_t processId) {
    if (processId == 0) {
        return "System";
    }

    std::string fullPath = getProcessPath(processId);
    if (fullPath.empty()) {
        return "";
    }

    try {
        std::filesystem::path p(fullPath);
        return p.filename().string();
    } catch (...) {
        auto pos = fullPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            return fullPath.substr(pos + 1);
        }
        return fullPath;
    }
}

} // namespace AudioRouter

