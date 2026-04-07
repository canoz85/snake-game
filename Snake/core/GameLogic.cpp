#include "GameLogic.h"

#include <QtGlobal>

GameLogic::GameLogic()
{
    reset();
}

void GameLogic::reset()
{
    m_snake.clear();
    m_gameOver   = false;
    m_score      = 0;
    m_currentDir = QPointF(1, 0); // start moving right
    m_queuedDir  = m_currentDir;
    m_hasQueuedDir = false;

    // Build initial snake horizontally in the middle of the grid.
    // Segment 0 is the head (rightmost), subsequent segments trail left.
    const QPointF start(Columns / 2, Rows / 2);
    for (int i = 0; i < InitLength; ++i)
        m_snake.append(QPointF(start.x() - i, start.y()));

    placeApple();
}

void GameLogic::setDirection(QPointF dir)
{
    if (m_gameOver)
        return;

    // Ignore invalid or repeated requests and allow at most one turn per tick.
    if (dir == QPointF(0, 0) || dir == m_currentDir || m_hasQueuedDir)
        return;

    // Block 180-degree reversal against the direction used for the current tick.
    if (m_currentDir + dir == QPointF(0, 0))
        return;

    m_queuedDir = dir;
    m_hasQueuedDir = true;
}

bool GameLogic::step()
{
    if (m_gameOver)
        return false;

    if (m_hasQueuedDir) {
        m_currentDir = m_queuedDir;
        m_hasQueuedDir = false;
    }

    // Compute candidate head position.
    QPointF head = m_snake.first() + m_currentDir;

    // Wrap around grid edges.
    if (head.x() < 0)             head.setX(Columns - 1);
    else if (head.x() >= Columns) head.setX(0);

    if (head.y() < 0)             head.setY(Rows - 1);
    else if (head.y() >= Rows)    head.setY(0);

    const bool ateApple = (head == m_apple);

    // On non-growth moves the tail vacates its cell this tick, so it should not
    // count as a collision target.
    const int collisionSegments = ateApple ? m_snake.size() : (m_snake.size() - 1);
    for (int i = 0; i < collisionSegments; ++i) {
        if (m_snake.at(i) == head) {
            m_gameOver = true;
            return false;
        }
    }

    // Move: insert new head at front.
    m_snake.prepend(head);

    if (ateApple) {
        // Keep the tail — snake grows by one cell.
        ++m_score;
        placeApple();
    } else {
        // Remove the tail — snake length stays the same.
        m_snake.removeLast();
    }

    return ateApple;
}

void GameLogic::placeApple()
{
    // Collect all grid cells not occupied by the snake.
    QVector<QPointF> freeCells;
    freeCells.reserve(Rows * Columns);
    for (int r = 0; r < Rows; ++r)
        for (int c = 0; c < Columns; ++c)
            freeCells.append(QPointF(c, r));

    for (const QPointF &seg : std::as_const(m_snake))
        freeCells.removeAll(seg);

    if (freeCells.isEmpty())
        return; // board is full — this is a win condition

    m_apple = freeCells.at(QRandomGenerator::global()->bounded(freeCells.size()));
}

QPointF GameLogic::directionToVector(Direction dir)
{
    switch (dir) {
    case Direction::Up:
        return QPointF(0, -1);
    case Direction::Down:
        return QPointF(0, 1);
    case Direction::Left:
        return QPointF(-1, 0);
    case Direction::Right:
        return QPointF(1, 0);
    }
    return QPointF(0, 0); // should never reach here
}

bool GameLogic::isSafeMove(QPointF head, QPointF dir)
{
    return !m_snake.contains(head + dir);
}

GameLogic::Direction GameLogic::decideDirection(QPointF head, QPointF apple)
{
    Direction nextDir = Direction::Up; // default value to silence compiler warning, will be overwritten by logic below

    if (apple.x() < head.x())
       nextDir = Direction::Left;
   else if (apple.x() > head.x())
       nextDir = Direction::Right;
   else if (apple.y() < head.y())
       nextDir = Direction::Up;
   else
       nextDir = Direction::Down;

    if (isSafeMove(head, directionToVector(nextDir)))
        return nextDir;

    // If the most direct path is blocked, try other directions in order of preference: straight, left, right, back.
    QList<Direction> allDirs = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    allDirs.removeAll(nextDir); // already checked the preferred direction
    for (Direction dir : allDirs) {
        if (isSafeMove(head, directionToVector(dir))) {
            return dir; 
        }
    }
    
    return nextDir;
}

void GameLogic::processMove(Direction dir)
{
    QPointF vector = directionToVector(dir);
    m_lastDirection = vector;
    setDirection(vector);
}

bool GameLogic::stepAI()
{
    Direction nextDir = decideDirection(m_snake.first(), m_apple);
    processMove(nextDir);
    
    return step();
}
