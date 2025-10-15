@echo off
echo ========================================
echo  Encrypted File Uploader Server
echo ========================================
echo.
echo Configuration:
echo   - Port: 8080
echo   - Upload max: 500MB
echo   - Memory: 1GB
echo   - Timeout: 5 minutes
echo   - Mode: FTP uniquement
echo.
echo Interface web:
echo   http://localhost:8080/uploader.html
echo.
echo ========================================
echo.

cd /d "%~dp0"

if not exist "php.ini" (
    echo ERREUR: php.ini non trouve!
    pause
    exit /b 1
)

echo Demarrage du serveur PHP avec configuration personnalisee...
php -S localhost:8080 -c php.ini

