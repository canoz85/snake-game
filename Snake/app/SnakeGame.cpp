#include "SnakeGame.h"

#include <QCoreApplication>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SnakeGame::SnakeGame(QWidget *parent)
    : QGraphicsView(parent)
{
    setupScene();
    const qreal W      = GameLogic::Columns * CellSize;
    const qreal BoardH = GameLogic::Rows    * CellSize;

    m_db.open(QCoreApplication::applicationDirPath() + "/snake_scores.db");

    m_nameInput  = std::make_unique<NameInputOverlay>(scene(), W, BoardH);
    m_scoreboard = std::make_unique<ScoreboardOverlay>(scene(), W, BoardH);

    m_menu = std::make_unique<MenuOverlay>(scene(), W, BoardH);
    m_menu->showStartMenu();
    connect(&m_timer, &QTimer::timeout, this, &SnakeGame::onTick);
    m_timer.setInterval(TimerDelay);
    updateScoreDisplay();
    setMenuVisible(true);
}

