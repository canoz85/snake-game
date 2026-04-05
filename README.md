# Snake

A classic Snake game built with **Qt6 + C++** and **CMake**.

## Gameplay

- Move the snake with the **arrow keys**
- Eat apples to grow and increase your score
- The board wraps at all four edges — no wall deaths
- The only way to die is **running into your own body**
- Only one turn is applied per game tick, so rapid double-key sequences won't cause false collisions
- Beat the all-time high score → type your name in the in-scene name-input prompt to save your entry
- View the **top-5 leaderboard** from the start menu (Scoreboard) or automatically after every game

## Controls

| Key | Action |
|-----|--------|
| ↑ ↓ ← → | Move / navigate menu |
| Enter / → | Confirm menu selection |

## Project Structure

```
Snake/
├── main.cpp               # Entry point
├── GameLogic.h/cpp        # Pure game state & rules (no Qt graphics)
├── SnakeGame.h/cpp        # QGraphicsView, rendering, input handling
├── MenuOverlay.h/cpp      # In-scene start / game-over menu
├── NameInputOverlay.h/cpp # In-scene name-input prompt (new high score)
├── ScoreboardOverlay.h/cpp# Top-5 leaderboard display
├── ScoreDB.h/cpp          # SQLite persistence layer (Qt6::Sql / QSQLITE)
└── SnakeGame.ui           # Qt Designer UI file
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
