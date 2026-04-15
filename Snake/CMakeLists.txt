cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

# Tell CMake to run the Qt helpers automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 widgets and SQL (for SQLite scoreboard)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core Network)

# Build the executable
qt_add_executable(Snake
    app/main.cpp
    core/GameLogic.h
    core/GameLogic.cpp
    ui/menu/MenuOverlay.h
    ui/menu/MenuOverlay.cpp
    app/SnakeGame.h
    app/SnakeGame.cpp
    app/SnakeGame_config.h
    app/SnakeGame_config.cpp
    app/SnakeGame_scene.cpp
    app/SnakeGame_input.cpp
    app/SnakeGame_flow.cpp
    app/SnakeGame.ui
    data/ScoreDB.h
    data/ScoreDB.cpp
    ui/scoreboard/NameInputOverlay.h
    ui/scoreboard/NameInputOverlay.cpp
    ui/scoreboard/ScoreboardOverlay.h
    ui/scoreboard/ScoreboardOverlay.cpp
)

# Link Qt widgets
target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Core Qt6::Sql Qt6::Network)

# Allow includes like "ui/menu/MenuOverlay.h" and "data/ScoreDB.h".
target_include_directories(Snake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Optional – set some binary properties
set_target_properties(Snake PROPERTIES
    MACOSX_BUNDLE   TRUE
)

# Show a console window in Debug on Windows so qDebug() output is visible.
if (WIN32)
    set_target_properties(Snake PROPERTIES
        WIN32_EXECUTABLE $<IF:$<CONFIG:Debug>,FALSE,TRUE>
    )
endif()
