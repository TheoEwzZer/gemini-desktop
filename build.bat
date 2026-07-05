setlocal enableextensions

REM ============================================================
REM  Gemini Desktop - script de build (Windows x64, MSVC + vcpkg)
REM  Generateur : Visual Studio 2022 (localise MSVC tout seul,
REM  pas besoin de vcvars64 ni de Ninja).
REM  Sortie de l'exe : build\Release\GeminiDesktop.exe
REM ============================================================

REM --- vcpkg (defini APRES tout le reste pour ne pas etre ecrase) ---
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=C:\Users\theof\vcpkg"
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
  echo [ERREUR] vcpkg introuvable dans "%VCPKG_ROOT%".
  echo          Definissez VCPKG_ROOT ou editez ce script.
  exit /b 1
)

cd /d "%~dp0"

REM Le cache binaire vcpkg doit exister s'il est defini dans l'environnement.
if not "%VCPKG_DEFAULT_BINARY_CACHE%"=="" (
  if not exist "%VCPKG_DEFAULT_BINARY_CACHE%" (
    echo Creation du cache binaire vcpkg : "%VCPKG_DEFAULT_BINARY_CACHE%"
    mkdir "%VCPKG_DEFAULT_BINARY_CACHE%" 2>nul
  )
)

REM Purge d'un cache CMake incompatible (ex. genere avec un autre generateur).
if exist "%~dp0build\CMakeCache.txt" (
  echo Nettoyage du cache CMake precedent...
  rmdir /s /q "%~dp0build"
)

echo === CONFIGURE (CMake + vcpkg, generateur Visual Studio) ===
echo Premier run : vcpkg compile webview2 + wil, cela peut prendre plusieurs minutes...
cmake --preset x64-release
if errorlevel 1 ( echo [ERREUR] Configuration CMake echouee. & exit /b 1 )

echo.
echo === BUILD (Release) ===
cmake --build --preset x64-release
if errorlevel 1 ( echo [ERREUR] Compilation echouee. & exit /b 1 )

echo.
echo === TERMINE ===
set "EXE=%~dp0build\Release\GeminiDesktop.exe"
if exist "%EXE%" (
  echo [OK] Binaire produit : "%EXE%"
) else (
  echo [ATTENTION] Binaire introuvable a l'emplacement attendu ^(voir logs ci-dessus^).
)
endlocal
