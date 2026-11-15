@echo off
setlocal
set "GPP="
if exist "C:\msys64\mingw64\bin\g++.exe" set "GPP=C:\msys64\mingw64\bin\g++.exe"
if not defined GPP where g++ >nul 2>&1 && set "GPP=g++"
if not defined GPP (
  echo g++ not found. Add C:\msys64\mingw64\bin to PATH.
  exit /b 1
)

if not exist build mkdir build

echo Building build\prioro.exe ...
"%GPP%" -std=c++17 -O2 -I src -static src/prioro.cpp src/main.cpp -o build/prioro.exe
if errorlevel 1 exit /b 1
echo OK. Run: build\prioro.exe
exit /b 0
