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
#include "GameLogic.h"
#include "MenuOverlay.h"

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
    // Rendering constants (pixel-level, view concerns only)
    static constexpr int CellSize   = 20;  // px per grid cell
    static constexpr int TimerDelay = 150; // ms
    static constexpr int ScoreBarHeight = 28;

    void setupScene();
    void syncGraphics();           // map GameLogic state → QGraphicsItems
    void ensureSnakeItems(int n);  // grow body-item pool to n items
    void setMenuVisible(bool visible);
    void updateScoreDisplay();

    // Head shape helpers — return geometry in local cell coords (origin = top-left)
    QPolygonF headPolygon(QPointF dir) const;
    QRectF    eyeRect(QPointF dir)     const;

    GameLogic  m_logic;
    QTimer     m_timer;
    bool       m_inMenu = true;
    std::unique_ptr<MenuOverlay> m_menu;

    QGraphicsPolygonItem*       m_headItem    = nullptr; // triangle arrow
    QGraphicsEllipseItem*       m_headEyeItem = nullptr; // eye dot
    QVector<QGraphicsRectItem*> m_snakeItems;             // body segments only (index 1+)
    QGraphicsRectItem*          m_appleItem   = nullptr;
    QGraphicsRectItem*          m_scoreBarItem = nullptr;
    QGraphicsTextItem*          m_scoreTextItem = nullptr;
};
