@echo off
setlocal

echo ========================================
echo  Compilation DEBUG WallpaperAI avec Qt/MinGW
echo ========================================
echo.

REM Arreter l'application si elle est en cours d'execution
echo Arret de l'application en cours d'execution...
powershell "Stop-Process -Name 'WallpaperAI' -Force -ErrorAction SilentlyContinue"
echo.

REM Ajouter Qt, MinGW et CMake au PATH
echo Configuration de l'environnement Qt/MinGW...
set PATH=C:\Installation\Qt\Tools\CMake_64\bin;C:\Installation\Qt\6.9.2\mingw_64\bin;C:\Installation\Qt\Tools\mingw1310_64\bin;%PATH%

echo Environnement configure avec succes
echo.

REM Creer le repertoire de build s'il n'existe pas
if not exist "build_debug" (
    echo Creation du repertoire build_debug...
    mkdir build_debug
)

REM Se placer dans le repertoire build_debug
cd build_debug

echo Configuration du projet avec CMake (MODE DEBUG)...
REM Definir le chemin vers Qt6 (ajustez selon votre installation)
set Qt6_DIR=C:\Installation\Qt\6.9.2\mingw_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Installation\Qt\6.9.2\mingw_64

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DQt6_DIR=%Qt6_DIR% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% ..

if %errorlevel% neq 0 (
    echo ERREUR: Echec de la configuration CMake
    pause
    exit /b 1
)

echo.
echo Compilation du projet en mode DEBUG...
cmake --build . --config Debug

if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation
    pause
    exit /b 1
)

echo.
echo Deploiement des dependances Qt...
"C:\Installation\Qt\6.9.2\mingw_64\bin\windeployqt.exe" WallpaperAI.exe

if %errorlevel% neq 0 (
    echo AVERTISSEMENT: Echec du deploiement Qt, mais l'executable est cree
)

echo.
echo Copie des plugins Qt necessaires...
if exist "platforms" xcopy "platforms" "..\build_debug_release\platforms\" /s /e /y /i
if exist "imageformats" xcopy "imageformats" "..\build_debug_release\imageformats\" /s /e /y /i
if exist "iconengines" xcopy "iconengines" "..\build_debug_release\iconengines\" /s /e /y /i
if exist "styles" xcopy "styles" "..\build_debug_release\styles\" /s /e /y /i
if exist "tls" xcopy "tls" "..\build_debug_release\tls\" /s /e /y /i

echo Copie des images PNG...
REM Copier les fichiers PNG du repertoire assets vers build_debug
copy "..\assets\*.png" "."

echo.
echo Creation du repertoire build_debug_release...
REM Revenir au repertoire parent pour creer build_debug_release
cd ..

REM Supprimer et recreer le repertoire build_debug_release
if exist "build_debug_release" (
    echo Suppression de l'ancien repertoire build_debug_release...
    rmdir /s /q "build_debug_release"
)
mkdir build_debug_release

echo Copie de l'executable DEBUG...
copy "build_debug\WallpaperAI.exe" "build_debug_release\"

echo Copie des dependances Qt (DLL uniquement)...
copy "build_debug\*.dll" "build_debug_release\"

echo Copie des plugins Qt necessaires...
if exist "build_debug\platforms" xcopy "build_debug\platforms" "build_debug_release\platforms\" /s /e /y /i
if exist "build_debug\imageformats" xcopy "build_debug\imageformats" "build_debug_release\imageformats\" /s /e /y /i
if exist "build_debug\iconengines" xcopy "build_debug\iconengines" "build_debug_release\iconengines\" /s /e /y /i
if exist "build_debug\styles" xcopy "build_debug\styles" "build_debug_release\styles\" /s /e /y /i
if exist "build_debug\tls" xcopy "build_debug\tls" "build_debug_release\tls\" /s /e /y /i

echo Copie des images PNG...
copy "assets\*.png" "build_debug_release\"

echo.
echo ========================================
echo  COMPILATION DEBUG REUSSIE !
echo ========================================
echo.
echo L'application DEBUG complete se trouve dans: build_debug_release\
echo Executable DEBUG: build_debug_release\WallpaperAI.exe
echo Images PNG: build_debug_release\*.png
echo.
echo IMPORTANT: Pour debugger dans VSCode/Visual Studio:
echo 1. Lancez build_debug_release\WallpaperAI.exe
echo 2. Attachez le debugger au processus en cours
echo 3. Ou configurez le debugger pour pointer vers build_debug_release\WallpaperAI.exe
echo.

pause
