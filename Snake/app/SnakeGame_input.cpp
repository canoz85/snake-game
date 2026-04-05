#include "SnakeGame.h"

#include <QApplication>

void SnakeGame::keyPressEvent(QKeyEvent *event)
{
    switch (m_uiState) {
    case UiState::Menu:
        handleMenuInput(event);
        break;
    case UiState::NameInput:
        handleNameInput(event);
        break;
    case UiState::Scoreboard:
        handleScoreboardInput(event);
        break;
    case UiState::Playing:
        handleGameplayInput(event);
        break;
    }
}

void SnakeGame::handleNameInput(QKeyEvent *event)
{
    m_nameInput->handleKey(event);
    if (m_nameInput->isConfirmed())
        onNameInputDone();
}

void SnakeGame::handleScoreboardInput(QKeyEvent *event)
{
    m_scoreboard->handleKey(event);
    if (m_scoreboard->poll() != ScoreboardOverlay::Action::Continue)
        return;

    if (m_scoreboardFromStart) {
        m_scoreboardFromStart = false;
        m_menu->showStartMenu();
        setMenuVisible(true);
    } else {
        m_menu->showGameOverMenu(m_logic.score());
        setMenuVisible(true);
    }
}

void SnakeGame::handleMenuInput(QKeyEvent *event)
{
    if (!m_menu) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Up:
        m_menu->moveSelectionUp();
        break;
    case Qt::Key_Down:
        m_menu->moveSelectionDown();
        break;
    case Qt::Key_Right:
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        const MenuOverlay::Action action = m_menu->activateSelection();
        if (action == MenuOverlay::Action::Start || action == MenuOverlay::Action::Restart) {
            newGame();
            setMenuVisible(false);
        } else if (action == MenuOverlay::Action::BackToMenu) {
            m_menu->showStartMenu();
            setMenuVisible(true);
        } else if (action == MenuOverlay::Action::Scoreboard) {
            setUiState(UiState::Scoreboard);
            m_scoreboardFromStart = true;
            m_scoreboard->show(m_db.getTopScores());
        } else if (action == MenuOverlay::Action::Exit) {
            qApp->quit();
        }
        break;
    }
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void SnakeGame::handleGameplayInput(QKeyEvent *event)
{
    if (m_logic.isGameOver()) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Up:    m_logic.setDirection(QPointF( 0, -1)); break;
    case Qt::Key_Down:  m_logic.setDirection(QPointF( 0,  1)); break;
    case Qt::Key_Left:  m_logic.setDirection(QPointF(-1,  0)); break;
    case Qt::Key_Right: m_logic.setDirection(QPointF( 1,  0)); break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}
