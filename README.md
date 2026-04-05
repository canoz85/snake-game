# Snake

A classic Snake game built with **Qt6 + C++** and **CMake**.

## Gameplay

- Move the snake with the **arrow keys**
- Eat apples to grow and increase your score
- The board wraps at all four edges — no wall deaths
- The only way to die is **running into your own body**
- Only one turn is applied per game tick, so rapid double-key sequences won't cause false collisions

## Controls

| Key | Action |
|-----|--------|
| ↑ ↓ ← → | Move / navigate menu |
| Enter / → | Confirm menu selection |

## Project Structure

```
Snake/
├── main.cpp           # Entry point
├── GameLogic.h/cpp    # Pure game state & rules (no Qt graphics)
├── SnakeGame.h/cpp    # QGraphicsView, rendering, input handling
├── MenuOverlay.h/cpp  # In-scene start / game-over menu
└── SnakeGame.ui       # Qt Designer UI file
```

## Building

**Requirements**

- Qt 6 (Widgets module)
- CMake ≥ 3.16
- A C++17-capable compiler (MinGW 13 on Windows is known to work)

**Steps**

```bash
cmake -S Snake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/Snake
```

Or open the project in Qt Creator / VS Code with the CMake Tools extension and build the `Snake` target.

## Authors

ʙᴀᴛᴜ & ᴄᴀɴ
