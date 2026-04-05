cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

# Tell CMake to run the Qt helpers automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 widgets and SQL (for SQLite scoreboard)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Build the executable
qt_add_executable(Snake
    main.cpp
    GameLogic.h
    GameLogic.cpp
    MenuOverlay.h
    MenuOverlay.cpp
    SnakeGame.h
    SnakeGame.cpp
    SnakeGame.ui
    ScoreDB.h
    ScoreDB.cpp
    NameInputOverlay.h
    NameInputOverlay.cpp
    ScoreboardOverlay.h
    ScoreboardOverlay.cpp
)

# Link Qt widgets
target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Sql)

# Optional – set some binary properties
set_target_properties(Snake PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE   TRUE
)
