#include "Startup.hpp"

#include <windows.h>

#include <string>

namespace gemini {

namespace {

constexpr wchar_t kRunKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValue[] = L"GeminiDesktop";

// Commande enregistree : "<chemin de l'exe>" --startup
// Le flag --startup fait demarrer l'app directement reduite dans le tray.
std::wstring StartupCommand() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::wstring(L"\"") + path + L"\" --startup";
}

} // namespace

bool IsStartupEnabled() {
    wchar_t buffer[MAX_PATH + 32] = {};
    DWORD size = sizeof(buffer);
    return RegGetValueW(HKEY_CURRENT_USER, kRunKey, kRunValue, RRF_RT_REG_SZ,
                        nullptr, buffer, &size) == ERROR_SUCCESS;
}

void SetStartupEnabled(bool enabled) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &key) !=
        ERROR_SUCCESS) {
        return;
    }

    if (enabled) {
        const std::wstring cmd = StartupCommand();
        RegSetValueExW(key, kRunValue, 0, REG_SZ,
                       reinterpret_cast<const BYTE*>(cmd.c_str()),
                       static_cast<DWORD>((cmd.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(key, kRunValue);
    }
    RegCloseKey(key);
}

} // namespace gemini
