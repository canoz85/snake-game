#pragma once

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPolygonF>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
#include <memory>
#include "core/GameLogic.h"
#include "ui/menu/MenuOverlay.h"
#include "data/ScoreDB.h"
#include "ui/scoreboard/NameInputOverlay.h"
#include "ui/scoreboard/ScoreboardOverlay.h"

// SnakeGame is the view/input layer only.
// All game state and rules live in GameLogic.
class SnakeGame : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SnakeGame(QWidget *parent = nullptr);

    void newGame();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onTick(); // called by QTimer each game tick

private:
    enum class UiState {
        Menu,
        Playing,
        NameInput,
        Scoreboard,
    };

    // Rendering constants (pixel-level, view concerns only)
    static constexpr int CellSize   = 20;  // px per grid cell
    static constexpr int TimerDelay = 150; // ms
    static constexpr int ScoreBarHeight = 28;

    void setupScene();
    void setUiState(UiState state);
    void syncGraphics();           // map GameLogic state → QGraphicsItems
    void ensureSnakeItems(int n);  // grow body-item pool to n items
    void setMenuVisible(bool visible);
    void updateScoreDisplay();
    void handleGameOver();         // decide whether to show name input or scoreboard
    void onNameInputDone();        // called after player confirms/skips name entry
    void handleMenuInput(QKeyEvent *event);
    void handleNameInput(QKeyEvent *event);
    void handleScoreboardInput(QKeyEvent *event);
    void handleGameplayInput(QKeyEvent *event);

    // Head shape helpers — return geometry in local cell coords (origin = top-left)
    QPolygonF headPolygon(QPointF dir) const;
    QRectF    eyeRect(QPointF dir)     const;

    GameLogic  m_logic;
    QTimer     m_timer;
    UiState    m_uiState            = UiState::Menu;
    bool       m_scoreboardFromStart = false;
    ScoreDB    m_db;
    std::unique_ptr<MenuOverlay>        m_menu;
    std::unique_ptr<NameInputOverlay>   m_nameInput;
    std::unique_ptr<ScoreboardOverlay>  m_scoreboard;

    QGraphicsPolygonItem*       m_headItem    = nullptr; // triangle arrow
    QGraphicsEllipseItem*       m_headEyeItem = nullptr; // eye dot
    QVector<QGraphicsRectItem*> m_snakeItems;             // body segments only (index 1+)
    QGraphicsRectItem*          m_appleItem   = nullptr;
    QGraphicsRectItem*          m_scoreBarItem = nullptr;
    QGraphicsTextItem*          m_scoreTextItem = nullptr;
};
