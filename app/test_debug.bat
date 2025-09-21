@echo off
echo ========================================
echo  Test de debogage GDB pour WallpaperIA
echo ========================================

echo.
echo 1. Configuration de l'environnement...
set PATH=C:\Installation\Qt\Tools\CMake_64\bin;C:\Installation\Qt\6.9.2\mingw_64\bin;C:\Installation\Qt\Tools\mingw1310_64\bin;%PATH%

echo.
echo 2. Verification de GDB...
gdb --version | head -1

echo.
echo 3. Test de l'executable debug...
if not exist build\WallpaperIA.exe (
    echo ERREUR: Executable debug non trouve. Executez d'abord build_debug.bat
    pause
    exit /b 1
)

echo.
echo 4. Information sur l'executable...
file build\WallpaperIA.exe 2>nul || echo Commande 'file' non disponible

echo.
echo 5. Test de demarrage avec GDB...
echo Commandes GDB suggerees:
echo   (gdb) break main
echo   (gdb) run
echo   (gdb) continue
echo   (gdb) quit
echo.

cd build
gdb WallpaperIA.exe

echo.
echo Test termine.
pause