@echo off
setlocal

echo ========================================
echo  Deploiement WallpaperIA
echo ========================================
echo.

REM Chemin Qt (ajustez selon votre installation)
set QT_DIR=C:\Installation\Qt\6.9.2\mingw_64

REM Creer dossier de deploiement
if not exist "deploy" mkdir deploy
if not exist "deploy\WallpaperIA" mkdir deploy\WallpaperIA

REM Copier l'executable
copy "build\WallpaperIA.exe" "deploy\WallpaperIA\"

REM Copier les ressources
copy "build\icon.png" "deploy\WallpaperIA\"
copy "build\red_cross.svg" "deploy\WallpaperIA\"

REM Deployer automatiquement les dependances Qt
cd deploy\WallpaperIA
"%QT_DIR%\bin\windeployqt.exe" --compiler-runtime --release WallpaperIA.exe

echo.
echo ========================================
echo  DEPLOIEMENT TERMINE !
echo ========================================
echo.
echo L'application deployee se trouve dans: deploy\WallpaperIA\
echo.

pause