@echo off
REM Script de génération de l'installateur WallpaperAI avec NSIS
REM Créé avec Claude Code

echo ========================================
echo  Generation de l'installateur WallpaperAI
echo ========================================
echo.

REM Vérifier que NSIS est installé
set "NSIS_PATH=C:\Installation\NSIS\makensis.exe"

REM Essayer d'abord le chemin par défaut
if exist "%NSIS_PATH%" (
    set "MAKENSIS=%NSIS_PATH%"
    goto nsis_found
)

REM Sinon, chercher dans le PATH
where makensis >nul 2>&1
if %errorlevel% equ 0 (
    set "MAKENSIS=makensis"
    goto nsis_found
)

REM NSIS non trouvé
echo ERREUR: NSIS n'est pas installe ou introuvable
echo.
echo Telechargez NSIS depuis: https://nsis.sourceforge.io/Download
echo Installez NSIS dans C:\Installation\NSIS ou ajoutez-le au PATH
echo Exemple: C:\Program Files (x86)\NSIS
echo.
pause
exit /b 1

:nsis_found

REM Vérifier que le répertoire release existe
if not exist "..\app\release\WallpaperAI.exe" (
    echo ERREUR: Le repertoire release n'existe pas ou est vide
    echo.
    echo Veuillez compiler l'application d'abord avec:
    echo   cd ..\app
    echo   build.bat
    echo.
    pause
    exit /b 1
)

echo Verification de l'environnement...
echo - NSIS trouve: OK
echo - Application compilee: OK
echo.

REM Compiler l'installateur
echo Compilation de l'installateur NSIS...
echo.
"%MAKENSIS%" /V3 WallpaperAI.nsi

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo  INSTALLATEUR GENERE AVEC SUCCES !
    echo ========================================
    echo.
    echo L'installateur se trouve dans:

    REM Trouver le fichier généré (WallpaperAI-Setup-*.exe)
    for %%F in ("WallpaperAI-Setup-*.exe") do (
        echo %cd%\%%F
        echo.
        echo Taille: %%~zF octets (%%~zF bytes)
    )
    echo.
) else (
    echo.
    echo ERREUR: La compilation de l'installateur a echoue
    echo Consultez les messages d'erreur ci-dessus
    echo.
    pause
    exit /b 1
)

pause
