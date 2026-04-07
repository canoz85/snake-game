# Snake

A classic Snake game built with **Qt6 + C++** and **CMake**.

## Gameplay

- Move the snake with the **arrow keys**
- Eat apples to grow and increase your score
- The board wraps at all four edges — no wall deaths
- The only way to die is **running into your own body**
- Only one turn is applied per game tick, so rapid double-key sequences won't cause false collisions
- Includes an **AI mode** from the menu (**Start AI**) that controls the snake automatically
- AI pathfinding uses **time-aware A\*** on the wrapped grid, so it plans with future tail movement in mind
- Beat the all-time high score → type your name in the in-scene name-input prompt to save your entry
- View the **top-5 leaderboard** from the start menu (Scoreboard) or automatically after every game

## Controls

| Key | Action |
|-----|--------|
| ↑ ↓ ← → | Move / navigate menu |
| Enter / → | Confirm menu selection |

AI mode is started from the menu by selecting **Start AI**.

## AI Algorithm (A*)

The AI uses a **time-expanded A\*** search where each node is `(x, y, t)`:

- `x, y` are board coordinates (with edge wrapping)
- `t` is the future tick index

This lets the planner model that snake body cells can become free in later ticks as the tail moves.

High-level decision flow:

```text
Input: head position, apple position, current direction, snake body

1) Build occupancy release times from current body layout.
2) Run A* on states (cell, time):
    - neighbors: Up/Down/Left/Right with wrap-around
    - disallow immediate 180-degree reverse at t=0
    - disallow moves into cells not yet released by body timing
    - disallow collisions with recently planned head trail
    - heuristic: wrapped Manhattan distance to apple
3) If a path to apple is found, take the first step on that path.
4) If no path is found, choose the safest local move that minimizes
    wrapped distance to apple.
5) If no safe local move exists, keep moving in current direction.
```

## Project Structure

```
Snake/
├── app/
│   ├── main.cpp                     # Entry point
│   ├── SnakeGame.h                  # View class declaration
│   ├── SnakeGame.cpp                # Constructor/bootstrap wiring
│   ├── SnakeGame_scene.cpp          # Scene setup + rendering helpers
│   ├── SnakeGame_input.cpp          # Input/menu routing
│   ├── SnakeGame_flow.cpp           # Tick loop + game-over flow
│   └── SnakeGame.ui                 # Qt Designer UI file
├── core/
│   └── GameLogic.h/cpp              # Pure game state & rules (no Qt graphics)
├── data/
│   └── ScoreDB.h/cpp                # SQLite persistence layer (Qt6::Sql / QSQLITE)
└── ui/
    ├── menu/
    │   └── MenuOverlay.h/cpp        # In-scene start / game-over menu
    └── scoreboard/
        ├── NameInputOverlay.h/cpp   # In-scene name-input prompt (new high score)
        └── ScoreboardOverlay.h/cpp  # Top-5 leaderboard display
```

## Building

**Requirements**

- Qt 6 (Widgets + Sql modules)
- CMake ≥ 3.16
- A C++17-capable compiler (MinGW 13 on Windows is known to work)

> The `Sql` module ships with the standard Qt6 installer and bundles the QSQLITE driver — no separate SQLite installation is needed. Scores are stored in `snake_scores.db` next to the executable.

**Steps**

```bash
cmake -S Snake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/Snake
```

Or open the project in Qt Creator / VS Code with the CMake Tools extension and build the `Snake` target.

## Authors

ʙᴀᴛᴜ & ᴄᴀɴ
