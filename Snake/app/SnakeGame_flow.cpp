
#include "SnakeGame.h"
#include "SnakeGame_config.h"

#include <QBrush>
#include <QPen>
#include <QColor>

void SnakeGame::newGame()
{
    if (m_headItem) {
        scene()->removeItem(m_headItem);
        delete m_headItem;
        m_headItem = nullptr;
    }
    if (m_headEyeItem) {
        scene()->removeItem(m_headEyeItem);
        delete m_headEyeItem;
        m_headEyeItem = nullptr;
    }

    for (auto *item : std::as_const(m_snakeItems)) {
        scene()->removeItem(item);
        delete item;
    }
    m_snakeItems.clear();

    if (m_appleItem) {
        scene()->removeItem(m_appleItem);
        delete m_appleItem;
        m_appleItem = nullptr;
    }

    m_logic.reset();

    m_headItem = scene()->addPolygon(
        headPolygon(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(SnakeGameConfig::SnakeHeadColor)
    );
    m_headItem->setZValue(1);

    m_headEyeItem = scene()->addEllipse(
        eyeRect(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(SnakeGameConfig::SnakeEyeColor)
    );
    m_headEyeItem->setZValue(2);

    m_appleItem = scene()->addRect(
        0, 0, CellSize, CellSize,
        QPen(Qt::NoPen),
        QBrush(SnakeGameConfig::AppleColor)
    );

    ensureSnakeItems(m_logic.snakeBody().size() - 1);
    syncGraphics();

    if (!m_timer.isActive())
        m_timer.start();
}

void SnakeGame::onTick()
{
    if (m_uiState != UiState::Playing)
        return;

    m_logic.stepAI();
    syncGraphics();

    if (m_logic.isGameOver()) {
        m_timer.stop();
        handleGameOver();
    }
}

void SnakeGame::handleGameOver()
{
    const int currentScore = m_logic.score();
    if (currentScore > m_db.getHighScore()) {
        m_nameInput->show(currentScore);
        setUiState(UiState::NameInput);
    } else {
        m_scoreboard->show(m_db.getTopScores());
        setUiState(UiState::Scoreboard);
        m_scoreboardFromStart = false;
    }
}

void SnakeGame::onNameInputDone()
{
    const QString name = m_nameInput->typedName().trimmed();
    if (!name.isEmpty())
        m_db.save(name, m_logic.score());

    updateScoreDisplay();

    m_scoreboard->show(m_db.getTopScores());
    setUiState(UiState::Scoreboard);
    m_scoreboardFromStart = false;
}

void SnakeGame::togglePause()
{
    if (m_uiState == UiState::Playing) {
        m_previousState = UiState::Playing;
        setUiState(UiState::Paused);
    } else if (m_uiState == UiState::Paused) {
        setUiState(UiState::Playing);
    }
}
