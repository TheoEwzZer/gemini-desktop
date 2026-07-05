#pragma once

#include <windows.h>

namespace gemini {

// Identifiants partages entre les composants.
inline constexpr wchar_t kWindowClassName[] = L"GeminiDesktopWindowClass";
inline constexpr wchar_t kWindowTitle[]     = L"Gemini";
// AppUserModelID explicite et stable : garantit que la fenetre lancee se
// regroupe sous le meme bouton que le raccourci epingle dans la barre des
// taches (sinon Windows en derive un instable et affiche une icone en double).
inline constexpr wchar_t kAppUserModelId[]  = L"TheoEwzZer.GeminiDesktop";
inline constexpr wchar_t kMutexName[]       = L"Global\\GeminiDesktop_SingleInstance";
inline constexpr wchar_t kShowMessageName[] = L"GeminiDesktop_Show";
inline constexpr wchar_t kRegistryKey[]     = L"Software\\GeminiDesktop";
inline constexpr wchar_t kHomeUrl[]         = L"https://gemini.google.com";
inline constexpr wchar_t kWebView2DownloadUrl[] =
    L"https://developer.microsoft.com/microsoft-edge/webview2/";

// Message de la zone de notification (tray).
inline constexpr UINT WM_TRAY_CALLBACK = WM_APP + 1;

// Raccourci global (afficher/masquer depuis n'importe ou) : Ctrl+Alt+G.
inline constexpr int  HOTKEY_TOGGLE_ID  = 1;
inline constexpr UINT kHotkeyModifiers  = MOD_CONTROL | MOD_ALT | MOD_NOREPEAT;
inline constexpr UINT kHotkeyVk         = 'G';

// Identifiants du menu contextuel du tray.
inline constexpr UINT ID_TRAY_SHOW    = 1001;
inline constexpr UINT ID_TRAY_QUIT    = 1002;
inline constexpr UINT ID_TRAY_STARTUP = 1003;

// Identifiant de l'icone applicative (doit correspondre au .rc).
inline constexpr int IDI_APPICON = 101;

} // namespace gemini
