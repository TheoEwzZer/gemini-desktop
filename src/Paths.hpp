#pragma once

#include <string>

namespace gemini {

// Retourne le dossier de donnees persistant de l'application :
// %LOCALAPPDATA%\GeminiDesktop\WebView2 (cree si absent).
// Utilise comme user data folder WebView2 -> conserve la session/login.
std::wstring GetUserDataFolder();

} // namespace gemini
