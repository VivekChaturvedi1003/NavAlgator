@echo off
REM Builds NavAlgator using the compiler that came with the raylib installer.
set PATH=C:\raylib\w64devkit\bin;%PATH%
g++ -std=c++17 -I include -I C:\raylib\raylib\src src\main.cpp src\Grid.cpp src\Player.cpp src\Enemy.cpp src\Scoreboard.cpp src\GameEngine.cpp -o NavAlgator.exe -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
if %ERRORLEVEL% EQU 0 (echo Build OK - run NavAlgator.exe) else (echo Build FAILED - copy the error text)
