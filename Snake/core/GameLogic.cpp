#include "GameLogic.h"


#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
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
    m_stepsSinceLastFood = 0;

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

QPointF GameLogic::directionToVector(Direction dir) const
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

bool GameLogic::isSafeMove(QPointF head, QPointF dir) const
{
    return !m_snake.contains(head + dir);
}

// Opens a short-lived TCP connection, sends payload, returns trimmed reply.
static QByteArray tcpExchange(const QJsonObject &payload, bool *ok)
{
    *ok = false;

    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 12345);
    if (!socket.waitForConnected(500)) {
        qWarning() << "TCP connect failed:" << socket.errorString();
        return {};
    }

    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact) + '\n';
    if (socket.write(data) < 0) {
        qWarning() << "TCP write failed:" << socket.errorString();
        return {};
    }
    socket.flush();
    if (socket.bytesToWrite() > 0)
        socket.waitForBytesWritten(500);

    if (!socket.waitForReadyRead(500)) {
        qWarning() << "TCP no reply:" << socket.errorString();
        return {};
    }

    *ok = true;
    return socket.readAll().trimmed();
}

QJsonArray GameLogic::buildObservationState(QPointF head, QPointF apple) const
{
    const auto wrapCoord = [](int value, int maxValue) {
        if (value < 0)
            return maxValue - 1;
        if (value >= maxValue)
            return 0;
        return value;
    };

    const auto wrappedDelta = [](int from, int to, int size) {
        const int direct = to - from;
        const int wrapped = (direct > 0) ? (direct - size) : (direct + size);
        return (qAbs(direct) <= qAbs(wrapped)) ? direct : wrapped;
    };

    QJsonArray state;

    const QPointF dirVec = m_currentDir;
    const QPointF dirRight(-dirVec.y(), dirVec.x());
    const QPointF dirLeft(dirVec.y(), -dirVec.x());

    auto isUnsafe = [&](QPointF p) {
        const int nx = wrapCoord(static_cast<int>(p.x()), Columns);
        const int ny = wrapCoord(static_cast<int>(p.y()), Rows);

        const bool grows = (nx == static_cast<int>(apple.x())
                            && ny == static_cast<int>(apple.y()));
        const int collisionSegments = grows ? m_snake.size() : (m_snake.size() - 1);

        for (int i = 0; i < collisionSegments; ++i) {
            const QPointF &seg = m_snake.at(i);
            if (static_cast<int>(seg.x()) == nx && static_cast<int>(seg.y()) == ny)
                return 1;
        }
        return 0;
    };

    const int dx = wrappedDelta(static_cast<int>(head.x()), static_cast<int>(apple.x()), Columns);
    const int dy = wrappedDelta(static_cast<int>(head.y()), static_cast<int>(apple.y()), Rows);

    state.append(isUnsafe(head + dirVec));
    state.append(isUnsafe(head + dirRight));
    state.append(isUnsafe(head + dirLeft));

    state.append(m_currentDir == directionToVector(Direction::Left) ? 1 : 0);
    state.append(m_currentDir == directionToVector(Direction::Right) ? 1 : 0);
    state.append(m_currentDir == directionToVector(Direction::Up) ? 1 : 0);
    state.append(m_currentDir == directionToVector(Direction::Down) ? 1 : 0);

    state.append(dx < 0 ? 1 : 0);
    state.append(dx > 0 ? 1 : 0);
    state.append(dy < 0 ? 1 : 0);
    state.append(dy > 0 ? 1 : 0);

    return state;
}

GameLogic::Direction GameLogic::fallbackDirection(QPointF head) const
{
    const auto vectorToDirection = [&](const QPointF &dir) {
        if (dir == directionToVector(Direction::Left))
            return Direction::Left;
        if (dir == directionToVector(Direction::Right))
            return Direction::Right;
        if (dir == directionToVector(Direction::Down))
            return Direction::Down;
        return Direction::Up;
    };

    if (!m_snake.isEmpty() && isSafeMove(head, m_currentDir))
        return vectorToDirection(m_currentDir);

    const QList<Direction> dirs = {
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right
    };

    for (Direction dir : dirs) {
        const QPointF vec = directionToVector(dir);
        if (m_currentDir + vec == QPointF(0, 0))
            continue;
        if (isSafeMove(head, vec))
            return dir;
    }

    return vectorToDirection(m_currentDir);
}

int GameLogic::sendActRequest(const QJsonArray &state, bool *ok) const
{
    QJsonObject payload;
    payload["mode"]  = "act";
    payload["state"] = state;

    bool exchangeOk = false;
    const QByteArray response = tcpExchange(payload, &exchangeOk);
    if (!exchangeOk) {
        *ok = false;
        return -1;
    }

    bool parseOk = false;
    const int action = response.toInt(&parseOk);
    if (!parseOk) {
        qWarning() << "act: unexpected response:" << response;
        *ok = false;
        return -1;
    }

    //qDebug() << "act: received action" << action << "for state" << state;
    *ok = true;
    return action;
}

bool GameLogic::sendTrainPacket(const QJsonArray &state, int action, double reward,
                                 const QJsonArray &nextState, bool done, int snakeSize, bool starved) const
{
    QJsonObject payload;
    payload["mode"]       = "train";
    payload["state"]      = state;
    payload["action"]     = action;
    payload["reward"]     = reward;
    payload["next_state"] = nextState;
    payload["done"]       = done;
    payload["size"]       = snakeSize;
    payload["starved"]    = starved;

    bool ok = false;
    const QByteArray response = tcpExchange(payload, &ok);
    if (!ok)
        return false;

    if (response != "ok")
        qWarning() << "train: unexpected response:" << response;

    return true;
}

GameLogic::Direction GameLogic::actionToDirection(int action, bool *ok) const
{
    *ok = true;

    switch (action) {
    case 0:
        return Direction::Up;
    case 1:
        return Direction::Down;
    case 2:
        return Direction::Left;
    case 3:
        return Direction::Right;
    default:
        *ok = false;
        return Direction::Up;
    }
}

/*
GameLogic::Direction GameLogic::decideDirection_(QPointF head, QPointF apple)
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
    */

void GameLogic::processMove(Direction dir)
{
    QPointF vector = directionToVector(dir);
    m_lastDirection = vector;
    setDirection(vector);
}

bool GameLogic::stepAI()
{
    // --- Step 1: ask the server for an action ---
    const QJsonArray state = buildObservationState(m_snake.first(), m_apple);

    bool gotAction = false;
    const int actionInt = sendActRequest(state, &gotAction);

    Direction dir = fallbackDirection(m_snake.first());
    if (gotAction) {
        bool validAction = false;
        dir = actionToDirection(actionInt, &validAction);
        if (!validAction) {
            qWarning() << "stepAI: action out of range:" << actionInt;
            dir = fallbackDirection(m_snake.first());
            gotAction = false;
        }
    }

    struct ANode {
        int x = 0;
        int y = 0;
    };

    const auto torusHeuristic = [](const ANode &a, const ANode &b) {
        const int dx = qAbs(a.x - b.x);
        const int dy = qAbs(a.y - b.y);
        return qMin(dx, Columns - dx) + qMin(dy, Rows - dy);
    };

    const auto distanceToApple = [&](QPointF head, QPointF apple) {
        const ANode a{static_cast<int>(head.x()), static_cast<int>(head.y())};
        const ANode b{static_cast<int>(apple.x()), static_cast<int>(apple.y())};
        return torusHeuristic(a, b);
    };

    QPointF prevHead = m_snake.first();

    // --- Step 2: apply action, evaluate outcome ---
    ++m_stepsSinceLastFood;
    const int starvationLimit = 100 * m_snake.size();

    processMove(dir);

    const bool ateApple = step();
    const bool died     = m_gameOver;

    if (ateApple)
        m_stepsSinceLastFood = 0;

    const bool starved = !died && (m_stepsSinceLastFood > starvationLimit);

    float prevDist = distanceToApple(prevHead, m_apple);
    float dist = distanceToApple(m_snake.first(), m_apple);
    //qDebug() << "prevDist" << prevDist << "dist" << dist;

    const double reward = (died || starved) ? -10.0 : (ateApple ? 10.0 : ((dist < prevDist) ? 0.1 : -0.2));
    const bool   done   = died || starved;

    // --- Step 3: send training packet ---
    if (gotAction) {
        const QJsonArray nextState = buildObservationState(m_snake.first(), m_apple);
        sendTrainPacket(state, actionInt, reward, nextState, done, m_snake.size(), starved);
    }

    // --- Step 4: auto-reset episode on death or starvation (keeps the timer running) ---
    if (done)
        reset();

    return ateApple;
}
