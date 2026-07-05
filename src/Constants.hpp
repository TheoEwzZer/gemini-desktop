#pragma once

#include <windows.h>

namespace gemini {

// Identifiants partages entre les composants.
inline constexpr wchar_t kWindowClassName[] = L"GeminiDesktopWindowClass";
inline constexpr wchar_t kWindowTitle[]     = L"Gemini";
inline constexpr wchar_t kMutexName[]       = L"Global\\GeminiDesktop_SingleInstance";
inline constexpr wchar_t kShowMessageName[] = L"GeminiDesktop_Show";
inline constexpr wchar_t kRegistryKey[]     = L"Software\\GeminiDesktop";
inline constexpr wchar_t kHomeUrl[]         = L"https://gemini.google.com";
inline constexpr wchar_t kWebView2DownloadUrl[] =
    L"https://developer.microsoft.com/microsoft-edge/webview2/";

// Message de la zone de notification (tray).
inline constexpr UINT WM_TRAY_CALLBACK = WM_APP + 1;

// Identifiants du menu contextuel du tray.
inline constexpr UINT ID_TRAY_SHOW = 1001;
inline constexpr UINT ID_TRAY_QUIT = 1002;

// Identifiant de l'icone applicative (doit correspondre au .rc).
inline constexpr int IDI_APPICON = 101;

} // namespace gemini
