/**
 * @file process-utils.hpp
 * @brief Utility functions for querying process identifiers, executable names, and file paths on Windows.
 */

#pragma once

#include <string>
#include <cstdint>

namespace AudioRouter {

class ProcessUtils {
public:
    // Obtém o nome base do executável a partir do PID (ex: "Discord.exe")
    static std::string getProcessName(uint32_t processId);

    // Obtém o caminho completo do executável
    static std::string getProcessPath(uint32_t processId);

    // Verifica se um processo ainda está em execução
    static bool isProcessRunning(uint32_t processId);

    // Converte para minúsculas
    static std::string toLower(std::string str);
};

} // namespace AudioRouter

