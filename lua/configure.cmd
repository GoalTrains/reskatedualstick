@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
"C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" -S "C:/Games/Skate/mine/reskate-luau-controls" -B "C:/Games/Skate/mine/reskate-luau-controls/build/vs2026-x64" -G "Visual Studio 18 2026" -A x64 -DDINGOSDK_BUILD_PLUGIN_TESTS=ON
