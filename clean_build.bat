@echo off
rem Script to clean CMake artifacts from source tree and set up a proper build directory

echo Cleaning CMake artifacts from source tree...

rem Remove CMake artifacts from root directory
if exist CMakeCache.txt del /Q CMakeCache.txt
if exist cmake_install.cmake del /Q cmake_install.cmake
if exist Makefile del /Q Makefile
if exist compile_commands.json del /Q compile_commands.json

rem Remove generated directories
if exist CMakeFiles rmdir /S /Q CMakeFiles

rem Check for existing build directory
if not exist build (
  mkdir build
  echo Created build directory
) else (
  echo Build directory already exists.
)

echo Setting up build directory...

rem Create a build script that properly uses the build directory
echo @echo off > build.bat
echo rem Script to build KAI using proper out-of-source build >> build.bat
echo. >> build.bat
echo if not exist build mkdir build >> build.bat
echo cd build >> build.bat
echo. >> build.bat
echo echo Configuring with CMake... >> build.bat
echo cmake .. %* >> build.bat
echo if errorlevel 1 (echo CMake configuration failed ^& exit /b 1) >> build.bat
echo. >> build.bat
echo echo Building project... >> build.bat
echo cmake --build . >> build.bat
echo if errorlevel 1 (echo Build failed ^& exit /b 1) >> build.bat
echo. >> build.bat
echo echo Build successful! >> build.bat
echo echo Executables can be found in the Bin/ directory >> build.bat
echo cd .. >> build.bat

echo Created build.bat script

rem Instructions
echo.
echo Done! CMake artifacts have been cleaned from the source tree.
echo To build the project:
echo   1. Run build.bat
echo   2. All build artifacts will be in the build/ directory
echo   3. Executables will be in the Bin/ directory
echo.
echo For more detailed build instructions, see BUILD.md