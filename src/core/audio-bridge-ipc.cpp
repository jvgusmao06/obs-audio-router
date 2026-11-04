#include "audio-bridge-ipc.hpp"
#include <sddl.h>

AudioBridgeIPC::AudioBridgeIPC() {
    ensureEventHandle();
}

AudioBridgeIPC::~AudioBridgeIPC() {
    if (m_eventHandle != NULL && m_eventHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_eventHandle);
        m_eventHandle = NULL;
    }
}

AudioBridgeIPC::AudioBridgeIPC(AudioBridgeIPC&& other) noexcept
    : m_eventHandle(other.m_eventHandle) {
    other.m_eventHandle = NULL;
}

AudioBridgeIPC& AudioBridgeIPC::operator=(AudioBridgeIPC&& other) noexcept {
    if (this != &other) {
        if (m_eventHandle != NULL && m_eventHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_eventHandle);
        }
        m_eventHandle = other.m_eventHandle;
        other.m_eventHandle = NULL;
    }
    return *this;
}

bool AudioBridgeIPC::ensureEventHandle() {
    if (m_eventHandle != NULL && m_eventHandle != INVALID_HANDLE_VALUE) {
        return true;
    }

    // Prepare permissive security descriptor (Everyone: All Access)
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = NULL;

    PSECURITY_DESCRIPTOR pSD = NULL;
    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
            L"D:(A;;GA;;;WD)", SDDL_REVISION_1, &pSD, NULL)) {
        sa.lpSecurityDescriptor = pSD;
    }

    // Attempt creation with Global namespace first (manual reset event)
    m_eventHandle = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_GLOBAL);

    if (m_eventHandle == NULL) {
        // Fallback to Local namespace if Global access is restricted
        m_eventHandle = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_LOCAL);
    }

    if (pSD != NULL) {
        LocalFree(pSD);
    }

    return (m_eventHandle != NULL && m_eventHandle != INVALID_HANDLE_VALUE);
}

bool AudioBridgeIPC::setActive(bool active) {
    if (!ensureEventHandle()) {
        return false;
    }

    if (active) {
        return SetEvent(m_eventHandle) != 0;
    } else {
        return ResetEvent(m_eventHandle) != 0;
    }
}

bool AudioBridgeIPC::isOBSActive() const {
    if (m_eventHandle == NULL || m_eventHandle == INVALID_HANDLE_VALUE) {
        // Try opening existing event
        HANDLE hEvent = OpenEventW(SYNCHRONIZE, FALSE, EVENT_NAME_GLOBAL);
        if (hEvent == NULL) {
            hEvent = OpenEventW(SYNCHRONIZE, FALSE, EVENT_NAME_LOCAL);
        }
        if (hEvent == NULL) {
            return false;
        }
        DWORD res = WaitForSingleObject(hEvent, 0);
        CloseHandle(hEvent);
        return (res == WAIT_OBJECT_0);
    }

    DWORD res = WaitForSingleObject(m_eventHandle, 0);
    return (res == WAIT_OBJECT_0);
}

HANDLE AudioBridgeIPC::getEventHandle() const {
    return m_eventHandle;
}
