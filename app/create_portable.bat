@echo off
setlocal

echo ========================================
echo  Creation version portable WallpaperIA
echo ========================================
echo.

REM Chemin Qt (ajustez selon votre installation)
set QT_DIR=C:\Installation\Qt\6.9.2\mingw_64

REM Creer dossier portable
if not exist "portable" mkdir portable
if not exist "portable\WallpaperIA_Portable" mkdir portable\WallpaperIA_Portable

REM Copier l'executable et ressources
copy "build\WallpaperIA.exe" "portable\WallpaperIA_Portable\"
copy "build\icon.png" "portable\WallpaperIA_Portable\"
copy "build\red_cross.svg" "portable\WallpaperIA_Portable\"

cd portable\WallpaperIA_Portable

REM Deployer les dependances Qt avec toutes les options
"%QT_DIR%\bin\windeployqt.exe" --compiler-runtime --release --force --no-translations --no-system-d3d-compiler --no-opengl-sw WallpaperIA.exe

REM Copier manuellement les DLLs critiques si necessaire
if exist "%QT_DIR%\bin\libgcc_s_seh-1.dll" copy "%QT_DIR%\bin\libgcc_s_seh-1.dll" .
if exist "%QT_DIR%\bin\libstdc++-6.dll" copy "%QT_DIR%\bin\libstdc++-6.dll" .
if exist "%QT_DIR%\bin\libwinpthread-1.dll" copy "%QT_DIR%\bin\libwinpthread-1.dll" .

REM Verifier que le dossier platforms existe
if not exist "platforms" (
    echo ERREUR: Le dossier platforms n'a pas ete cree par windeployqt
    echo Copie manuelle...
    mkdir platforms
    copy "%QT_DIR%\plugins\platforms\qwindows.dll" "platforms\"
)

echo.
echo ========================================
echo  VERSION PORTABLE CREEE !
echo ========================================
echo.
echo L'application portable se trouve dans: portable\WallpaperIA_Portable\
echo Vous pouvez maintenant copier ce dossier n'importe ou.
echo.

cd ..\..
pause