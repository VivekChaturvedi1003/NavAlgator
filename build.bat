@echo off
REM Builds NavAlgator using the compiler that came with the raylib installer.
set PATH=C:\raylib\w64devkit\bin;%PATH%

REM SQLite is compiled once (about a minute) and reused on later builds.
if not exist libs\sqlite\sqlite3.c (
    echo ERROR: libs\sqlite\sqlite3.c not found. See libs\sqlite\README.txt
    exit /b 1
)
if not exist sqlite3.o (
    echo Compiling SQLite once - this takes about a minute...
    gcc -c -O2 libs\sqlite\sqlite3.c -o sqlite3.o
    if errorlevel 1 (echo SQLite compile FAILED & exit /b 1)
)

g++ -std=c++17 -I include -I libs\sqlite -I C:\raylib\raylib\src src\main.cpp src\Grid.cpp src\Player.cpp src\Enemy.cpp src\Scoreboard.cpp src\Database.cpp src\Bomb.cpp src\GameEngine.cpp sqlite3.o -o NavAlgator.exe -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 (echo Build FAILED - copy the error text & exit /b 1)
echo Build OK - run NavAlgator.exe
