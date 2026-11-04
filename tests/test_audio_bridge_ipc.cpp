#include <iostream>
#include <cassert>
#include "audio-bridge-ipc.hpp"

int main() {
    std::cout << "[TEST] Running AudioBridgeIPC tests..." << std::endl;

    AudioBridgeIPC controller;
    AudioBridgeIPC reader;

    // Ensure reset state initially
    controller.setActive(false);
    assert(!controller.isOBSActive());
    assert(!reader.isOBSActive());
    std::cout << "  ✓ Initial state is inactive" << std::endl;

    // Test activation
    bool activated = controller.setActive(true);
    assert(activated);
    assert(controller.isOBSActive());
    assert(reader.isOBSActive());
    std::cout << "  ✓ Activation signaled to reader successfully" << std::endl;

    // Test deactivation
    bool deactivated = controller.setActive(false);
    assert(deactivated);
    assert(!controller.isOBSActive());
    assert(!reader.isOBSActive());
    std::cout << "  ✓ Deactivation signaled to reader successfully" << std::endl;

    // Test idempotency
    controller.setActive(true);
    controller.setActive(true);
    assert(reader.isOBSActive());
    controller.setActive(false);
    assert(!reader.isOBSActive());
    std::cout << "  ✓ Activation idempotency verified" << std::endl;

    // Handle check
    assert(controller.getEventHandle() != NULL);
    std::cout << "  ✓ Event handle is valid" << std::endl;

    std::cout << "[TEST] All AudioBridgeIPC tests passed!" << std::endl;
    return 0;
}
