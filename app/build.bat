@echo off
setlocal

REM Gestion du parametre de langue (par defaut: FR)
set LANG=%1
if "%LANG%"=="" set LANG=FR

REM Si le parametre est ALL, compiler pour toutes les langues
if /I "%LANG%"=="ALL" (
    echo ========================================
    echo  Compilation WallpaperAI - TOUTES LES LANGUES
    echo ========================================
    echo.

    REM Creer le repertoire release s'il n'existe pas
    if not exist "release" mkdir release

    REM Liste des langues supportees
    for %%L in (FR EN ES PT IT DE RU) do (
        echo.
        echo ========================================
        echo  Compilation pour la langue: %%L
        echo ========================================
        set BUILD_ALL_MODE=1
        call "%~f0" %%L
        if errorlevel 1 (
            echo ERREUR: Echec de la compilation pour %%L
            pause
            exit /b 1
        )

        REM Renommer l'executable et le deplacer
        echo Deplacement de WallpaperAI_%%L.exe...
        move /y "release\WallpaperAI.exe" "release\WallpaperAI_%%L.exe"
    )

    echo.
    echo ========================================
    echo  COMPILATION REUSSIE POUR TOUTES LES LANGUES !
    echo ========================================
    echo.
    echo Executables crees:
    dir /b "release\WallpaperAI_*.exe"
    echo.
    exit /b 0
)

echo ========================================
echo  Compilation WallpaperAI avec Qt/MinGW
echo  Langue: %LANG%
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

REM Passer la langue en tant que define a la compilation
cmake -G "MinGW Makefiles" -DQt6_DIR=%Qt6_DIR% -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% -DCMAKE_CXX_FLAGS="-DLANG_%LANG%" ..

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
"C:\Installation\Qt\6.9.2\mingw_64\bin\windeployqt.exe" WallpaperAI.exe

if %errorlevel% neq 0 (
    echo AVERTISSEMENT: Echec du deploiement Qt, mais l'executable est cree
)

echo.
echo Creation du repertoire release...
REM Revenir au repertoire parent pour creer release
cd ..

REM Supprimer et recreer le repertoire release (sauf en mode BUILD_ALL)
if "%BUILD_ALL_MODE%"=="1" (
    REM En mode ALL, ne pas supprimer release pour conserver les executables precedents
    if not exist "release" mkdir release
) else (
    if exist "release" (
        echo Suppression de l'ancien repertoire release...
        rmdir /s /q "release"
    )
    mkdir release
)

echo Copie de l'executable...
copy "build\WallpaperAI.exe" "release\"

echo Copie des dependances Qt (DLL uniquement)...
copy "build\*.dll" "release\"

echo Copie des plugins Qt necessaires...
if exist "build\platforms" xcopy "build\platforms" "release\platforms\" /s /e /y /i
if exist "build\imageformats" xcopy "build\imageformats" "release\imageformats\" /s /e /y /i
if exist "build\iconengines" xcopy "build\iconengines" "release\iconengines\" /s /e /y /i
if exist "build\styles" xcopy "build\styles" "release\styles\" /s /e /y /i
if exist "build\tls" xcopy "build\tls" "release\tls\" /s /e /y /i

echo Copie des images PNG...
REM Copier uniquement les fichiers PNG du repertoire assets vers la racine de release
copy "assets\*.png" "release\"

echo.
echo ========================================
echo  COMPILATION REUSSIE !
echo ========================================
echo.
echo L'application complete se trouve dans: release\
echo Executable principal: release\WallpaperAI.exe
echo Images PNG: release\*.png
echo.

REM Rester dans le repertoire courant

pause