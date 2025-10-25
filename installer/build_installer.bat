@echo off
setlocal

echo ========================================
echo  Build WallpaperAI Multilang Installer
echo ========================================
echo.

REM Chemin vers NSIS
set "NSIS_PATH=C:\Installation\NSIS\makensis.exe"

REM Verifier si NSIS est disponible dans le PATH
if exist "%NSIS_PATH%" (
    set "MAKENSIS=%NSIS_PATH%"
    goto nsis_found
)

where makensis >nul 2>&1
if %errorlevel% equ 0 (
    set "MAKENSIS=makensis"
    goto nsis_found
)

echo ERREUR: NSIS n'est pas installe ou introuvable
echo.
echo Installez NSIS depuis https://nsis.sourceforge.io/Download
echo Ou ajoutez NSIS au PATH systeme
pause
exit /b 1

:nsis_found
echo NSIS trouve: %MAKENSIS%
echo.

REM Afficher la version de NSIS
echo Version de NSIS:
"%MAKENSIS%" /VERSION
echo.

REM Verifier que les executables multilingues existent
echo Verification des executables sources...
set "MISSING=0"

for %%L in (FR EN ES PT IT DE RU) do (
    if not exist "..\app\release\WallpaperAI_%%L.exe" (
        echo ERREUR: WallpaperAI_%%L.exe introuvable
        set "MISSING=1"
    ) else (
        echo [OK] WallpaperAI_%%L.exe
    )
)

echo.

if "%MISSING%"=="1" (
    echo.
    echo ERREUR: Certains executables sont manquants
    echo Veuillez compiler tous les executables avec: cd ..\app ^&^& build.bat ALL
    echo.
    pause
    exit /b 1
)

echo Tous les executables sont presents
echo.

REM Compiler le script NSIS
echo Compilation de l'installateur multilingue...
echo.
"%MAKENSIS%" /V3 WallpaperAI.nsi

if %errorlevel% neq 0 (
    echo.
    echo ERREUR: La compilation NSIS a echoue
    pause
    exit /b 1
)

echo.
echo ========================================
echo  BUILD REUSSI !
echo ========================================
echo.

REM Afficher les informations sur l'installateur
if exist "WallpaperAI-Setup-Multilang-1.0.3.exe" (
    echo Installateur cree: WallpaperAI-Setup-Multilang-1.0.3.exe
    for %%F in ("WallpaperAI-Setup-Multilang-1.0.3.exe") do echo Taille: %%~zF octets
) else (
    echo AVERTISSEMENT: Le fichier installateur n'a pas ete trouve
)

echo.
pause
