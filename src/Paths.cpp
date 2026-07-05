#include "Paths.hpp"

#include <windows.h>
#include <shlobj.h>
#include <wil/resource.h>

namespace gemini {

namespace {

// Cree recursivement chaque segment du chemin si necessaire.
void EnsureDirectory(const std::wstring& path) {
    // CreateDirectory echoue si un parent manque : on remonte segment par segment.
    for (size_t pos = path.find(L'\\'); pos != std::wstring::npos;
         pos = path.find(L'\\', pos + 1)) {
        const std::wstring sub = path.substr(0, pos);
        if (sub.size() > 2) { // ignore "C:" seul
            CreateDirectoryW(sub.c_str(), nullptr);
        }
    }
    CreateDirectoryW(path.c_str(), nullptr);
}

} // namespace

std::wstring GetUserDataFolder() {
    wil::unique_cotaskmem_string localAppData;
    std::wstring base;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr,
                                       &localAppData))) {
        base.assign(localAppData.get());
    } else {
        // Fallback tres improbable : dossier temporaire.
        wchar_t temp[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, temp);
        base.assign(temp);
        if (!base.empty() && base.back() == L'\\') {
            base.pop_back();
        }
    }

    std::wstring folder = base + L"\\GeminiDesktop\\WebView2";
    EnsureDirectory(folder);
    return folder;
}

} // namespace gemini
