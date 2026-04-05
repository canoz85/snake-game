#include "SnakeGame.h"

#include <QApplication>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SnakeGame::SnakeGame(QWidget *parent)
    : QGraphicsView(parent)
{
    setupScene();
    const qreal W = GameLogic::Columns * CellSize;
    const qreal BoardH = GameLogic::Rows * CellSize;
    m_menu = std::make_unique<MenuOverlay>(scene(), W, BoardH);
    m_menu->showStartMenu();
    connect(&m_timer, &QTimer::timeout, this, &SnakeGame::onTick);
    m_timer.setInterval(TimerDelay);
    updateScoreDisplay();
    setMenuVisible(true);
}

// ---------------------------------------------------------------------------
// Scene setup (one-time, called from constructor)
// ---------------------------------------------------------------------------

void SnakeGame::setupScene()
{
    const int W = GameLogic::Columns * CellSize;
    const int BoardH = GameLogic::Rows * CellSize;
    const int H = BoardH + ScoreBarHeight;

    QGraphicsScene *sc = new QGraphicsScene(this);
    sc->setSceneRect(0, 0, W, H);
    setScene(sc);
    setFixedSize(W, H);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QColor(30, 30, 30));

    m_scoreBarItem = sc->addRect(
        0, BoardH, W, ScoreBarHeight,
        QPen(Qt::NoPen),
        QBrush(QColor(18, 18, 18))
    );
    m_scoreBarItem->setZValue(3);

    m_scoreTextItem = sc->addText(QString(), QFont("Consolas", 11, QFont::Bold));
    m_scoreTextItem->setDefaultTextColor(QColor(170, 220, 170));
    m_scoreTextItem->setZValue(4);
}

void SnakeGame::setMenuVisible(bool visible)
{
    m_inMenu = visible;

    if (m_menu)
        m_menu->setVisible(visible);

    if (m_headItem)
        m_headItem->setVisible(!visible);
    if (m_headEyeItem)
        m_headEyeItem->setVisible(!visible);
    for (auto *item : std::as_const(m_snakeItems))
        item->setVisible(!visible);
    if (m_appleItem)
        m_appleItem->setVisible(!visible);
}

// ---------------------------------------------------------------------------
// Game control
// ---------------------------------------------------------------------------

void SnakeGame::newGame()
{
    // Remove and delete existing head items.
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

    // Remove and delete existing body graphics items.
    for (auto *item : std::as_const(m_snakeItems)) {
        scene()->removeItem(item);
        delete item;
    }
    m_snakeItems.clear();

    // Remove and delete the apple item.
    if (m_appleItem) {
        scene()->removeItem(m_appleItem);
        delete m_appleItem;
        m_appleItem = nullptr;
    }

    // Reset pure game logic.
    m_logic.reset();

    // --- Head: arrow-shaped polygon ---
    m_headItem = scene()->addPolygon(
        headPolygon(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(QColor(0, 230, 0))
    );
    m_headItem->setZValue(1);

    // --- Eye: small white ellipse with a dark pupil rendered as a single dot ---
    m_headEyeItem = scene()->addEllipse(
        eyeRect(m_logic.currentDir()),
        QPen(Qt::NoPen),
        QBrush(QColor(20, 20, 20))
    );
    m_headEyeItem->setZValue(2);

    // --- Apple ---
    m_appleItem = scene()->addRect(
        0, 0, CellSize, CellSize,
        QPen(Qt::NoPen),
        QBrush(QColor(255, 50, 50))
    );

    // Pre-create body items (InitLength - 1 segments).
    ensureSnakeItems(m_logic.snakeBody().size() - 1);

    // Position everything.
    syncGraphics();

    if (!m_timer.isActive())
        m_timer.start();
}

// ---------------------------------------------------------------------------
// Timer tick — step logic then refresh graphics
// ---------------------------------------------------------------------------

void SnakeGame::onTick()
{
    if (m_inMenu)
        return;

    m_logic.step();
    syncGraphics();

    if (m_logic.isGameOver()) {
        m_timer.stop();
        if (m_menu)
            m_menu->showGameOverMenu(m_logic.score());
        setMenuVisible(true);
    }
}

// ---------------------------------------------------------------------------
// Rendering — map GameLogic state to QGraphicsItems
// ---------------------------------------------------------------------------

void SnakeGame::syncGraphics()
{
    const QVector<QPointF> &body = m_logic.snakeBody();
    const QPointF dir = m_logic.currentDir();

    // --- Head: update triangle shape for new direction + position ---
    if (m_headItem) {
        m_headItem->setPolygon(headPolygon(dir));
        m_headItem->setPos(body.first().x() * CellSize,
                           body.first().y() * CellSize);
    }
    if (m_headEyeItem) {
        m_headEyeItem->setRect(eyeRect(dir));
        m_headEyeItem->setPos(body.first().x() * CellSize,
                              body.first().y() * CellSize);
    }

    // --- Body segments (index 1+) ---
    ensureSnakeItems(body.size() - 1);
    for (int i = 1; i < body.size(); ++i) {
        m_snakeItems[i - 1]->setRect(
            body[i].x() * CellSize,
            body[i].y() * CellSize,
            CellSize, CellSize
        );
    }

    // --- Apple ---
    if (m_appleItem) {
        const QPointF ap = m_logic.applePos();
        m_appleItem->setRect(ap.x() * CellSize, ap.y() * CellSize, CellSize, CellSize);
    }

    updateScoreDisplay();
}

void SnakeGame::ensureSnakeItems(int n)
{
    while (m_snakeItems.size() < n) {
        m_snakeItems.append(
            scene()->addRect(
                0, 0, CellSize, CellSize,
                QPen(Qt::NoPen),
                QBrush(QColor(0, 200, 0))
            )
        );
    }
}

void SnakeGame::updateScoreDisplay()
{
    if (!m_scoreTextItem)
        return;

    const QString scoreLabel = QString("Score: %1").arg(m_logic.score());
    const int boardHeight = GameLogic::Rows * CellSize;

    m_scoreTextItem->setPlainText(scoreLabel);
    const QRectF textBounds = m_scoreTextItem->boundingRect();
    m_scoreTextItem->setPos(12, boardHeight + (ScoreBarHeight - textBounds.height()) / 2.0);
}

// ---------------------------------------------------------------------------
// Head shape helpers — all geometry in local cell coords (origin = top-left)
// ---------------------------------------------------------------------------

QPolygonF SnakeGame::headPolygon(QPointF dir) const
{
    const qreal S = CellSize;
    const qreal m = 2.0; // margin from cell edge

    QPolygonF poly;
    if (dir == QPointF(1, 0)) {         // right →
        poly << QPointF(m,     m)
             << QPointF(S - m, S / 2.0)
             << QPointF(m,     S - m);
    } else if (dir == QPointF(-1, 0)) { // left ←
        poly << QPointF(S - m, m)
             << QPointF(m,     S / 2.0)
             << QPointF(S - m, S - m);
    } else if (dir == QPointF(0, 1)) {  // down ↓
        poly << QPointF(m,         m)
             << QPointF(S / 2.0,   S - m)
             << QPointF(S - m,     m);
    } else {                            // up ↑
        poly << QPointF(m,         S - m)
             << QPointF(S / 2.0,   m)
             << QPointF(S - m,     S - m);
    }
    return poly;
}

QRectF SnakeGame::eyeRect(QPointF dir) const
{
    const qreal S  = CellSize;
    const qreal es = 3.5; // eye diameter in px

    // Position the eye near the tip of the triangle, offset to one side.
    QPointF ep;
    if      (dir == QPointF( 1,  0)) ep = QPointF(S * 0.58, S * 0.20);
    else if (dir == QPointF(-1,  0)) ep = QPointF(S * 0.20, S * 0.20);
    else if (dir == QPointF( 0,  1)) ep = QPointF(S * 0.20, S * 0.58);
    else                              ep = QPointF(S * 0.20, S * 0.20);

    return QRectF(ep.x(), ep.y(), es, es);
}

// ---------------------------------------------------------------------------
// Input — translate key events to GameLogic direction changes
// ---------------------------------------------------------------------------

void SnakeGame::keyPressEvent(QKeyEvent *event)
{
    if (m_inMenu) {
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
            } else if (action == MenuOverlay::Action::Exit) {
                qApp->quit();
            }
            break;
        }
        default:
            QGraphicsView::keyPressEvent(event);
        }
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
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

