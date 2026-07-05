@echo off
setlocal enableextensions

REM ============================================================
REM  Gemini Desktop - script de build (Windows x64, MSVC + vcpkg)
REM  Generateur : Ninja + compilateur cl (independant de la
REM  version de Visual Studio installee).
REM  Sortie de l'exe : build\GeminiDesktop.exe
REM ============================================================

REM --- Environnement MSVC (vcvars64) : charge cl.exe dans le PATH ---
set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
  for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VCVARS=%%i\VC\Auxiliary\Build\vcvars64.bat"
  )
)
if not exist "%VCVARS%" (
  echo [ERREUR] vcvars64.bat introuvable. Installez le workload "Desktop development with C++".
  exit /b 1
)
echo === Chargement de l'environnement MSVC x64 ===
call "%VCVARS%"
if errorlevel 1 ( echo [ERREUR] vcvars64 a echoue. & exit /b 1 )

REM Utilise le CMake + Ninja natifs fournis par Visual Studio (le cmake de
REM MSYS2/mingw, s'il est dans le PATH, casse la compilation des ressources RC).
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`) do set "VSINSTALL=%%i"
if defined VSINSTALL (
  set "PATH=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
)

REM --- vcpkg (defini APRES vcvars, qui ecrase sinon VCPKG_ROOT) ---
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

echo === CONFIGURE (CMake + vcpkg, Ninja + cl) ===
echo Premier run : vcpkg compile webview2 + wil, cela peut prendre plusieurs minutes...
cmake --preset x64-release
if errorlevel 1 ( echo [ERREUR] Configuration CMake echouee. & exit /b 1 )

echo.
echo === BUILD (Release) ===
cmake --build --preset x64-release
if errorlevel 1 ( echo [ERREUR] Compilation echouee. & exit /b 1 )

echo.
echo === TERMINE ===
set "EXE=%~dp0build\GeminiDesktop.exe"
if exist "%EXE%" (
  echo [OK] Binaire produit : "%EXE%"
) else (
  echo [ATTENTION] Binaire introuvable a l'emplacement attendu ^(voir logs ci-dessus^).
)
endlocal
