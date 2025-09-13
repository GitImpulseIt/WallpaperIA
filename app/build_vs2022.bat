@echo off
setlocal

echo ========================================
echo  Compilation WallpaperIA avec Qt/MinGW
echo ========================================
echo.

REM Charger l'environnement Visual Studio Build Tools
echo Chargement de l'environnement Visual Studio Build Tools...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"

if %errorlevel% neq 0 (
    echo ERREUR: Impossible de charger l'environnement Visual Studio Build Tools
    echo Verifiez que Visual Studio Build Tools est installe dans le bon repertoire
    pause
    exit /b 1
)

REM Ajouter Qt, MinGW et CMake au PATH
echo Configuration de l'environnement Qt/MinGW...
set PATH=C:\Installation\Qt\Tools\CMake_64\bin;C:\Installation\Qt\6.9.2\mingw_64\bin;C:\Installation\Qt\Tools\mingw1310_64\bin;%PATH%

echo Environnement configure avec succes
echo.

REM Creer le repertoire de build s'il n'existe pas
if not exist "build" (
    echo Creation du repertoire build...
    mkdir build
)

REM Se placer dans le repertoire build
cd build

echo Configuration du projet avec CMake...
REM Definir le chemin vers Qt6 (ajustez selon votre installation)
set Qt6_DIR=C:\Installation\Qt\6.9.2\mingw_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Installation\Qt\6.9.2\mingw_64

cmake -G "MinGW Makefiles" -DQt6_DIR=%Qt6_DIR% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% ..

if %errorlevel% neq 0 (
    echo ERREUR: Echec de la configuration CMake
    pause
    exit /b 1
)

echo.
echo Compilation du projet...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation
    pause
    exit /b 1
)

echo.
echo Deploiement des dependances Qt...
"C:\Installation\Qt\6.9.2\mingw_64\bin\windeployqt.exe" WallpaperIA.exe

if %errorlevel% neq 0 (
    echo AVERTISSEMENT: Echec du deploiement Qt, mais l'executable est cree
)

echo.
echo ========================================
echo  COMPILATION REUSSIE !
echo ========================================
echo.
echo L'executable se trouve dans: build\WallpaperIA.exe
echo.

REM Revenir au repertoire parent
cd ..

pause