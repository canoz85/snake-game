#include "SnakeGame.h"

#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>

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

void SnakeGame::setUiState(UiState state)
{
    m_uiState = state;

    const bool menuVisible       = (state == UiState::Menu);
    const bool nameInputVisible  = (state == UiState::NameInput);
    const bool scoreboardVisible = (state == UiState::Scoreboard);
    const bool gameplayVisible   = !menuVisible;

    if (m_menu)
        m_menu->setVisible(menuVisible);
    if (m_nameInput)
        m_nameInput->setVisible(nameInputVisible);
    if (m_scoreboard)
        m_scoreboard->setVisible(scoreboardVisible);

    if (m_headItem)
        m_headItem->setVisible(gameplayVisible);
    if (m_headEyeItem)
        m_headEyeItem->setVisible(gameplayVisible);
    for (auto *item : std::as_const(m_snakeItems))
        item->setVisible(gameplayVisible);
    if (m_appleItem)
        m_appleItem->setVisible(gameplayVisible);
}

void SnakeGame::setMenuVisible(bool visible)
{
    setUiState(visible ? UiState::Menu : UiState::Playing);
    m_scoreboardFromStart = false;
}

void SnakeGame::syncGraphics()
{
    const QVector<QPointF> &body = m_logic.snakeBody();
    const QPointF dir = m_logic.currentDir();

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

    ensureSnakeItems(body.size() - 1);
    for (int i = 1; i < body.size(); ++i) {
        m_snakeItems[i - 1]->setRect(
            body[i].x() * CellSize,
            body[i].y() * CellSize,
            CellSize, CellSize
        );
    }

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

QPolygonF SnakeGame::headPolygon(QPointF dir) const
{
    const qreal S = CellSize;
    const qreal m = 2.0;

    QPolygonF poly;
    if (dir == QPointF(1, 0)) {
        poly << QPointF(m,     m)
             << QPointF(S - m, S / 2.0)
             << QPointF(m,     S - m);
    } else if (dir == QPointF(-1, 0)) {
        poly << QPointF(S - m, m)
             << QPointF(m,     S / 2.0)
             << QPointF(S - m, S - m);
    } else if (dir == QPointF(0, 1)) {
        poly << QPointF(m,         m)
             << QPointF(S / 2.0,   S - m)
             << QPointF(S - m,     m);
    } else {
        poly << QPointF(m,         S - m)
             << QPointF(S / 2.0,   m)
             << QPointF(S - m,     S - m);
    }
    return poly;
}

QRectF SnakeGame::eyeRect(QPointF dir) const
{
    const qreal S  = CellSize;
    const qreal es = 3.5;

    QPointF ep;
    if      (dir == QPointF( 1,  0)) ep = QPointF(S * 0.58, S * 0.20);
    else if (dir == QPointF(-1,  0)) ep = QPointF(S * 0.20, S * 0.20);
    else if (dir == QPointF( 0,  1)) ep = QPointF(S * 0.20, S * 0.58);
    else                              ep = QPointF(S * 0.20, S * 0.20);

    return QRectF(ep.x(), ep.y(), es, es);
}
