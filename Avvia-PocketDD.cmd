@echo off
cd /d "%~dp0"
if exist "%~dp0runtime\node.exe" (
  "%~dp0runtime\node.exe" --use-env-proxy scripts/start-pc.js
  if errorlevel 1 pause
  exit /b
)
where node >nul 2>nul
if errorlevel 1 (
  echo Pacchetto incompleto. Estrai tutto PocketDD-PC.zip, inclusa la cartella runtime.
  pause
  exit /b 1
)
node --use-env-proxy scripts/start-pc.js
if errorlevel 1 pause
