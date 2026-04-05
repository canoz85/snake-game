#include "SnakeGame.h"

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
        QBrush(QColor(0, 230, 0))
    );
    m_headItem->setZValue(1);

    m_headEyeItem = scene()->addEllipse(
        eyeRect(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(QColor(20, 20, 20))
    );
    m_headEyeItem->setZValue(2);

    m_appleItem = scene()->addRect(
        0, 0, CellSize, CellSize,
        QPen(Qt::NoPen),
        QBrush(QColor(255, 50, 50))
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

    m_logic.step();
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

    m_scoreboard->show(m_db.getTopScores());
    setUiState(UiState::Scoreboard);
    m_scoreboardFromStart = false;
}
