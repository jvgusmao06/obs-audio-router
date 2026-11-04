#include <windows.h>
#include "audio-bridge-engine.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Single instance mutex to prevent multiple bridge processes
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"Global\\OBSAudioRouterBridgeRunningMutex");
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) CloseHandle(hMutex);
        return 0; // Already running, exit cleanly
    }

    AudioBridgeEngine engine;
    if (engine.initialize()) {
        engine.runLoop();
    }

    CloseHandle(hMutex);
    return 0;
}
