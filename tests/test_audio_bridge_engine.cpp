#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "audio-bridge-engine.hpp"
#include "audio-bridge-ipc.hpp"

int main() {
    std::cout << "[TEST] Running AudioBridgeEngine tests..." << std::endl;

    AudioBridgeIPC ipc;
    // Set OBS to inactive initially
    ipc.setActive(false);

    AudioBridgeEngine engine;
    assert(!engine.isStreaming());
    std::cout << "  ✓ Initial engine state is not streaming" << std::endl;

    // Test transition when OBS becomes active
    engine.updateState(true);
    assert(!engine.isStreaming());
    assert(engine.isOBSActive());
    std::cout << "  ✓ Engine detects OBS active and stays/switches to paused" << std::endl;

    // Test transition when OBS becomes inactive
    engine.updateState(false);
    assert(!engine.isOBSActive());
    std::cout << "  ✓ Engine detects OBS inactive and is ready to stream" << std::endl;

    // Test start/stop lifecycle flags
    bool initialized = engine.initializeTestMode();
    assert(initialized);
    assert(engine.isReady());
    std::cout << "  ✓ Engine initializes in test mode" << std::endl;

    engine.stop();
    assert(!engine.isRunning());
    std::cout << "  ✓ Engine stop signal verified" << std::endl;

    std::cout << "[TEST] All AudioBridgeEngine tests passed!" << std::endl;
    return 0;
}
