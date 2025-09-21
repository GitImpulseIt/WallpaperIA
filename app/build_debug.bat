@echo off

echo ========================================
echo  Compilation DEBUG WallpaperIA avec Qt/MinGW
echo ========================================

:: Arreter l'application si elle est en cours d'execution
echo.
echo Arret de l'application en cours d'execution...
taskkill /f /im WallpaperIA.exe >nul 2>&1

:: Configuration de l'environnement Qt/MinGW
echo.
echo Configuration de l'environnement Qt/MinGW...
set PATH=C:\Installation\Qt\Tools\CMake_64\bin;C:\Installation\Qt\6.9.2\mingw_64\bin;C:\Installation\Qt\Tools\mingw1310_64\bin;%PATH%

echo Environnement configure avec succes

:: Creer et nettoyer le repertoire de build
if exist build rmdir /S /Q build
mkdir build
cd build

:: Configuration du projet avec CMake en mode DEBUG
echo.
echo Configuration du projet avec CMake (DEBUG)...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -DDEBUG" ..

if %ERRORLEVEL% neq 0 (
    echo ERREUR: Echec de la configuration CMake
    exit /b 1
)

:: Compilation du projet
echo.
echo Compilation du projet en mode DEBUG...
mingw32-make -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo ERREUR: Echec de la compilation
    exit /b 1
)

echo.
echo ========================================
echo  COMPILATION DEBUG REUSSIE !
echo ========================================
echo.
echo Executable DEBUG: build\WallpaperIA.exe
echo Pret pour le debogage avec GDB/VSCode
echo.