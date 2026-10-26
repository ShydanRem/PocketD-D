@echo off
cd /d "%~dp0"
where node >nul 2>nul
if errorlevel 1 (
  echo Installa Node.js 24 da https://nodejs.org/ e poi riapri questo file.
  pause
  exit /b 1
)
node --use-env-proxy -v >nul 2>nul
if errorlevel 1 (
  echo Aggiorna Node.js alla versione 24 attuale da https://nodejs.org/.
  pause
  exit /b 1
)
node --use-env-proxy scripts/start-pc.js
pause
