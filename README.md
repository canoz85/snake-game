# Snake

A classic Snake game built with **Qt6 + C++** and **CMake**.

## Gameplay

- Move the snake with the **arrow keys**
- Eat apples to grow and increase your score
- The board wraps at all four edges — no wall deaths
- The only way to die is **running into your own body**
- Only one turn is applied per game tick, so rapid double-key sequences won't cause false collisions
- Includes two AI menu modes:
    - **Start AI**: visual AI play with normal tick speed and rendering
    - **Train AI**: headless fast-training mode (no board UI rendering)
- Beat the all-time high score → type your name in the in-scene name-input prompt to save your entry
- View the **top-5 leaderboard** from the start menu (Scoreboard) or automatically after every game

## Controls

| Key | Action |
|-----|--------|
| ↑ ↓ ← → | Move / navigate menu |
| Enter / → | Confirm menu selection |
| P / Space | Pause / resume (non-training gameplay) |

AI modes are started from the menu by selecting **Start AI** or **Train AI**.

## AI Integration (Socket + RL Loop)

The C++ game communicates with a Python server on `127.0.0.1:12345` using short-lived TCP requests.

Per AI tick, the game uses this observation state (11 values):

```text
[danger_straight, danger_right, danger_left,
 dir_left, dir_right, dir_up, dir_down,
 food_left, food_right, food_up, food_down]
```

Actions returned by Python (relative to current heading):

- `0 = straight`
- `1 = turn right`
- `2 = turn left`

Any other action value is rejected by C++ and logged as invalid.

Per-tick protocol:

1. **Act request** (before move)

```json
{"protocol_version":1,"mode":"act","state":[...]}
```

Server responds with a plain digit string such as `"2"`.

2. **Apply action in C++** and evaluate transition:

- apple eaten: `reward = 10.0`
- snake died or starvation: `reward = -10.0`, `done = true`
- moved closer to apple: `reward = +0.1`
- moved farther/same distance: `reward = -0.2`

3. **Train request** (after move)

```json
{
    "protocol_version": 1,
    "mode": "train",
    "state": [...],
    "action": 2,
    "reward": -0.1,
    "next_state": [...],
    "done": false,
    "size": 7,
    "starved": false
}
```

Server responds with `"ok"`.

### Train AI Mode

`Train AI` is optimized for speed:

- game runs multiple logic steps per timer tick
- board visuals are hidden (head/body/apple/score bar)
- debug socket logs remain enabled
- episodes auto-reset after death to continue training

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
│   ├── SnakeGame_flow.cpp           # Tick loop + normal/AI/train flow
│   └── SnakeGame.ui                 # Qt Designer UI file
├── core/
│   └── GameLogic.h/cpp              # Game rules + AI socket/RL bridge
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
