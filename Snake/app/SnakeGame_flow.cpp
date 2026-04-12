
#include "SnakeGame.h"
#include "SnakeGame_config.h"

#include <QBrush>
#include <QPen>
#include <QColor>

void SnakeGame::newGame(bool aiMode, bool trainingMode)
{
    if (m_headItem) {
        scene()->removeItem(m_headItem);
        delete m_headItem;
        m_headItem = nullptr;
    }
    for (auto *item : std::as_const(m_headEyeItems)) {
        scene()->removeItem(item);
        delete item;
    }
    m_headEyeItems.clear();
    
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
    m_logic.setAIMode(aiMode);
    m_trainingMode = (aiMode && trainingMode);
    m_timer.setInterval(m_trainingMode ? TrainTimerDelay : TimerDelay);
    
    m_headItem = scene()->addPolygon(
        headPolygon(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(SnakeGameConfig::SnakeHeadColor)
    );
    m_headItem->setZValue(1);
    
    const QVector<QRectF> eyes = eyeRects(m_logic.currentDir());
    for (const QRectF &eye : eyes) {
        auto *eyeItem = scene()->addEllipse(
            eye,
            QPen(Qt::NoPen),
            QBrush(SnakeGameConfig::SnakeEyeColor)
        );
        eyeItem->setZValue(2);
        m_headEyeItems.append(eyeItem);
    }
    
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

    if (m_logic.isAIMode() && m_trainingMode) {
        for (int i = 0; i < TrainStepsPerTick; ++i)
            m_logic.stepAI();
        return;
    }

    m_logic.isAIMode() ? m_logic.stepAI() : m_logic.step();
    syncGraphics();

    if (m_logic.isGameOver()) {
        m_timer.stop();
        handleGameOver();
    }
}

void SnakeGame::handleGameOver()
{
    if (m_trainingMode)
        return;

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
