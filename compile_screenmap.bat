@echo off
echo === Compilation de screenmap.cpp avec Visual Studio Tools ===

REM Activation de l'environnement Visual Studio 2022
echo Activation de l'environnement Visual Studio...
call "C:\Installation\Visual Studio 2022\Common7\Tools\vsdevcmd"

if %ERRORLEVEL% neq 0 (
    echo ERREUR: Impossible d'activer l'environnement Visual Studio
    pause
    exit /b 1
)

echo.
echo Compilation de screenmap.cpp...
cd /d "%~dp0app"

REM Compilation avec cl.exe (compilateur Visual Studio)
cl /EHsc /Fe:screenmap.exe screenmap.cpp user32.lib advapi32.lib gdi32.lib

if %ERRORLEVEL% equ 0 (
    echo.
    echo SUCCESS: Compilation réussie - screenmap.exe créé
    echo.
    echo Exécution du programme...
    screenmap.exe
) else (
    echo.
    echo ERREUR: Échec de la compilation
)

echo.
pause