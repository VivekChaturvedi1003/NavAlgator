# NavAlgator 🎮🤖

An interactive, real-time 2D grid navigation game built in **C++** using **Raylib**. The game features dynamic maze generation, player controls with history tracking, and intelligent enemy tracking powered by graph search algorithms.

---

## 📌 Project Overview
**NavAlgator** serves as a practical demonstration of fundamental Data Structures and Algorithms (DSA) applied to game development. Players navigate a grid-based maze to reach the destination while avoiding an active enemy AI that continuously recalculates the shortest path using **Breadth-First Search (BFS)**.

---

## 🏗️ System Architecture & File Structure

```text
NavAlgator/
│
├── src/
│   ├── Main.cpp          # Entry point and main game execution loop
│   ├── Grid.h / .cpp     # Grid matrix representation, rendering, & wall placement
│   ├── Player.h / .cpp   # Player coordinates, lives system, and undo movement stack
│   ├── Enemy.h / .cpp    # Enemy state & real-time BFS pathfinding logic
│   └── Engine.h / .cpp   # Game loop control, collision checks, & state management
│
├── .gitignore            # Ignores build artifacts (.exe, .o, .obj)
├── LICENSE               # Open-source license details
└── README.md             # Project documentation
