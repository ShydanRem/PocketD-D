@echo off
cd /d "%~dp0"
where node >nul 2>nul
if errorlevel 1 (
 echo Installa Node.js 24 da https://nodejs.org/ e riprova.
 pause
 exit /b 1
)
node --use-env-proxy scripts/start-bridge.js
pause
