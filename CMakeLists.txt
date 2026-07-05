cmake_minimum_required(VERSION 3.21)

project(GeminiDesktop VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependances fournies par vcpkg (voir vcpkg.json).
#   webview2 -> unofficial::webview2::webview2 (loader statique)
#   wil      -> WIL::WIL (header-only)
find_package(unofficial-webview2 CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)

add_executable(GeminiDesktop WIN32
    src/main.cpp
    src/App.cpp
    src/SingleInstance.cpp
    src/Tray.cpp
    src/Paths.cpp
    res/app.rc
)

# API Windows en Unicode (les macros IDI_*/IDC_* prennent alors leur forme W).
target_compile_definitions(GeminiDesktop PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(GeminiDesktop PRIVATE
    unofficial::webview2::webview2
    WIL::WIL
    Shell32
    Ole32
    Shlwapi
    User32
    Gdi32
    Advapi32
    Version
)

# Application fenetree (pas de console) meme si un generateur passe outre.
set_target_properties(GeminiDesktop PROPERTIES
    WIN32_EXECUTABLE ON
)

if(MSVC)
    target_compile_options(GeminiDesktop PRIVATE
        /W4
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>
    )
    target_link_options(GeminiDesktop PRIVATE
        $<$<CONFIG:Release>:/LTCG>
        /SUBSYSTEM:WINDOWS
    )
    # Manifeste applicatif (DPI PerMonitorV2) integre via le .rc, on desactive
    # donc la generation automatique par l'editeur de liens.
    target_link_options(GeminiDesktop PRIVATE /MANIFEST:NO)
endif()
