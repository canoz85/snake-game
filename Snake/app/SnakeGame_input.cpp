#include "SnakeGame.h"

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>

void SnakeGame::showInGameEscMenu()
{
    const bool wasPaused = (m_uiState == UiState::Paused);
    if (!wasPaused)
        m_timer.stop();

    QMessageBox box(this);
    box.setWindowTitle("Game Menu");
    box.setText("Choose an action:");
    QPushButton *restartBtn = box.addButton("Restart", QMessageBox::AcceptRole);
    QPushButton *menuBtn = box.addButton("Back to Main Menu", QMessageBox::ActionRole);
    QPushButton *exitBtn = box.addButton("Exit", QMessageBox::DestructiveRole);
    box.addButton("Cancel", QMessageBox::RejectRole);
    box.exec();

    QAbstractButton *clicked = box.clickedButton();
    if (clicked == restartBtn) {
        newGame(m_logic.isAIMode(), m_trainingMode, m_logic.aiPolicy());
        setMenuVisible(false);
        return;
    }

    if (clicked == menuBtn) {
        m_timer.stop();
        m_menu->showStartMenu();
        setMenuVisible(true);
        return;
    }

    if (clicked == exitBtn) {
        qApp->quit();
        return;
    }

    if (wasPaused) {
        setUiState(UiState::Paused);
    } else {
        if (!m_timer.isActive())
            m_timer.start();
        setUiState(UiState::Playing);
    }
}

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
    case UiState::Paused:
        // Allow pause/resume toggle and escape to menu
        if (event->key() == Qt::Key_P || event->key() == Qt::Key_Space) {
            togglePause();
        } else if (event->key() == Qt::Key_Escape) {
            showInGameEscMenu();
        } else {
            QGraphicsView::keyPressEvent(event);
        }
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
        m_menu->showStartMenu();
        setMenuVisible(true);
    }
}

void SnakeGame::handleMenuInput(QKeyEvent *event)
{
    if (!m_menu) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    auto processAction = [&](MenuOverlay::Action action) {
        if (action == MenuOverlay::Action::None)
            return false;

        if (action == MenuOverlay::Action::StartNormal) {
            newGame(false, false, GameLogic::AiPolicy::Neural);
            setMenuVisible(false);
        } else if (action == MenuOverlay::Action::OpenAIMenu) {
            m_menu->showAIModeMenu();
            setMenuVisible(true);
        } else if (action == MenuOverlay::Action::StartNeuralUI) {
            newGame(true, false, GameLogic::AiPolicy::Neural);
            setMenuVisible(false);
        } else if (action == MenuOverlay::Action::StartNeuralTrain) {
            newGame(true, true, GameLogic::AiPolicy::Neural);
            setMenuVisible(false);
        } else if (action == MenuOverlay::Action::StartRuleBased) {
            newGame(true, false, GameLogic::AiPolicy::RuleBased);
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

        return true;
    };

    MenuOverlay::Action shortcutAction = MenuOverlay::Action::None;

    switch (event->key()) {
    case Qt::Key_Up:
        m_menu->moveSelectionUp();
        break;
    case Qt::Key_Down:
        m_menu->moveSelectionDown();
        break;
    case Qt::Key_Right:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        processAction(m_menu->activateSelection());
        break;
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        shortcutAction = m_menu->activateNumber(event->key() - Qt::Key_0);
        processAction(shortcutAction);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void SnakeGame::handleGameplayInput(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        showInGameEscMenu();
        return;
    }

    if (m_trainingMode) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    if (m_logic.isGameOver()) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Up:    m_logic.setDirection(QPointF( 0, -1)); break;
    case Qt::Key_Down:  m_logic.setDirection(QPointF( 0,  1)); break;
    case Qt::Key_Left:  m_logic.setDirection(QPointF(-1,  0)); break;
    case Qt::Key_Right: m_logic.setDirection(QPointF( 1,  0)); break;
    case Qt::Key_P:
    case Qt::Key_Space: togglePause(); break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}
