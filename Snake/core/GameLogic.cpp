#include "GameLogic.h"

#include <QtGlobal>
#include <limits>

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
    struct ANode {
        int x = 0;
        int y = 0;
    };

    const auto wrapCoord = [](int value, int maxValue) {
        if (value < 0)
            return maxValue - 1;
        if (value >= maxValue)
            return 0;
        return value;
    };

    const auto toNode = [](const QPointF &p) {
        return ANode{static_cast<int>(p.x()), static_cast<int>(p.y())};
    };

    const auto toIndex = [](int x, int y) {
        return y * Columns + x;
    };

    const auto stateId = [toIndex](int x, int y, int t) {
        return t * (Rows * Columns) + toIndex(x, y);
    };

    const auto stateCell = [](int sid) {
        return sid % (Rows * Columns);
    };

    const auto stateTime = [](int sid) {
        return sid / (Rows * Columns);
    };

    const auto torusHeuristic = [](const ANode &a, const ANode &b) {
        const int dx = qAbs(a.x - b.x);
        const int dy = qAbs(a.y - b.y);
        return qMin(dx, Columns - dx) + qMin(dy, Rows - dy);
    };

    const auto nodeFromIndex = [](int idx) {
        return ANode{idx % Columns, idx / Columns};
    };

    const ANode start = toNode(head);
    const ANode goal  = toNode(apple);

    const int total = Rows * Columns;
    const int startIdx = toIndex(start.x, start.y);
    const int goalIdx  = toIndex(goal.x, goal.y);
    const int maxTime = total;
    const int totalStates = (maxTime + 1) * total;
    const int startState = stateId(start.x, start.y, 0);

    // Release step for each initially occupied cell: cell is blocked while step < releaseStep.
    QVector<int> releaseStep(total, 0);
    for (int i = 0; i < m_snake.size(); ++i) {
        const QPointF seg = m_snake.at(i);
        const int idx = toIndex(static_cast<int>(seg.x()), static_cast<int>(seg.y()));
        releaseStep[idx] = m_snake.size() - i;
    }

    QVector<int> gScore(totalStates, std::numeric_limits<int>::max());
    QVector<int> fScore(totalStates, std::numeric_limits<int>::max());
    QVector<int> cameFrom(totalStates, -1);
    QVector<bool> inOpen(totalStates, false);
    QVector<bool> closed(totalStates, false);
    QVector<int> openSet;
    openSet.reserve(total * 2);

    gScore[startState] = 0;
    fScore[startState] = torusHeuristic(start, goal);
    inOpen[startState] = true;
    openSet.append(startState);

    const QList<Direction> dirs = {
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right
    };

    int goalState = -1;

    while (!openSet.isEmpty()) {
        int bestPos = 0;
        for (int i = 1; i < openSet.size(); ++i) {
            if (fScore[openSet[i]] < fScore[openSet[bestPos]])
                bestPos = i;
        }

        const int currentState = openSet.at(bestPos);
        openSet.removeAt(bestPos);
        inOpen[currentState] = false;
        closed[currentState] = true;

        const int currentCell = stateCell(currentState);
        const int currentT = stateTime(currentState);
        const ANode currentNode = nodeFromIndex(currentCell);

        if (currentCell == goalIdx) {
            goalState = currentState;
            break;
        }

        if (currentT >= maxTime)
            continue;

        for (Direction dir : dirs) {
            const QPointF vec = directionToVector(dir);

            // Respect no-reverse rule for the immediate next move.
            if (currentT == 0 && (m_currentDir + vec == QPointF(0, 0)))
                continue;

            const int nx = wrapCoord(currentNode.x + static_cast<int>(vec.x()), Columns);
            const int ny = wrapCoord(currentNode.y + static_cast<int>(vec.y()), Rows);
            const int neighborCell = toIndex(nx, ny);
            const int nextT = currentT + 1;
            const int neighborState = stateId(nx, ny, nextT);

            if (closed[neighborState])
                continue;

            // Initially occupied body cells become available only after their release step.
            if (nextT < releaseStep[neighborCell])
                continue;

            // Prevent self-intersection against planned recent head trail.
            bool hitsPlannedBody = false;
            int back = currentState;
            int depth = 0;
            while (back != -1 && depth < (m_snake.size() - 1)) {
                if (stateCell(back) == neighborCell) {
                    hitsPlannedBody = true;
                    break;
                }
                back = cameFrom[back];
                ++depth;
            }
            if (hitsPlannedBody)
                continue;

            const int tentativeG = gScore[currentState] + 1;
            if (tentativeG >= gScore[neighborState])
                continue;

            cameFrom[neighborState] = currentState;
            gScore[neighborState] = tentativeG;
            fScore[neighborState] = tentativeG + torusHeuristic(ANode{nx, ny}, goal);

            if (!inOpen[neighborState]) {
                inOpen[neighborState] = true;
                openSet.append(neighborState);
            }
        }
    }

    if (goalState != -1) {
        int stepState = goalState;
        while (cameFrom[stepState] != startState)
            stepState = cameFrom[stepState];

        const int stepIdx = stateCell(stepState);
        const ANode step = nodeFromIndex(stepIdx);
        const int dx = (step.x - start.x + Columns) % Columns;
        const int dy = (step.y - start.y + Rows) % Rows;

        if (dx == 1)
            return Direction::Right;
        if (dx == Columns - 1)
            return Direction::Left;
        if (dy == 1)
            return Direction::Down;
        if (dy == Rows - 1)
            return Direction::Up;
    }

    // Fallback: pick the safest local move with best wrap-aware heuristic.
    Direction bestDir = Direction::Up;
    int bestH = std::numeric_limits<int>::max();
    bool foundSafe = false;

    for (Direction dir : dirs) {
        const QPointF vec = directionToVector(dir);
        if (m_currentDir + vec == QPointF(0, 0))
            continue;

        const int nx = wrapCoord(start.x + static_cast<int>(vec.x()), Columns);
        const int ny = wrapCoord(start.y + static_cast<int>(vec.y()), Rows);
        const int nextCell = toIndex(nx, ny);

        if (1 < releaseStep[nextCell])
            continue;

        const int h = torusHeuristic(ANode{nx, ny}, goal);
        if (h < bestH) {
            bestH = h;
            bestDir = dir;
            foundSafe = true;
        }
    }

    if (foundSafe)
        return bestDir;

    // Last resort: keep current direction to preserve deterministic behavior.
    if (m_currentDir == QPointF(1, 0))
        return Direction::Right;
    if (m_currentDir == QPointF(-1, 0))
        return Direction::Left;
    if (m_currentDir == QPointF(0, 1))
        return Direction::Down;

    return Direction::Up;
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
