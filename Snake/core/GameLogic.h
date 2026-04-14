#pragma once

#include <QPointF>
#include <QVector>
#include <QRandomGenerator>

class QJsonArray;

// Pure game-logic class — no Qt graphics dependency.
// All positions are in grid coordinates (col, row).
class GameLogic
{
public:
    static constexpr int Rows       = 20;
    static constexpr int Columns    = 30;
    static constexpr int InitLength = 5;

    GameLogic();

    // Reset to a fresh game state.
    void reset();

    // Request a direction change. dir must be (±1,0) or (0,±1).
    // 180-degree reversals are silently ignored.
    void setDirection(QPointF dir);

    // Advance one game tick. Returns true if the snake ate the apple.
    // After a game-over tick, isGameOver() returns true and
    // subsequent calls to step() are no-ops.
    bool step();

    //simple ai mode
    bool stepAI();
    void setAIMode(bool enabled) { m_aiMode = enabled;}
    bool isAIMode() const { return m_aiMode; }  

    // --- State accessors ---
    const QVector<QPointF>& snakeBody()  const { return m_snake; } // head at [0]
    QPointF                 applePos()   const { return m_apple; }
    QPointF                 currentDir() const { return m_currentDir; }
    bool                    isGameOver() const { return m_gameOver; }
    int                     score()      const { return m_score; }

    QPointF                 m_lastDirection;

private:
    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };

    QVector<QPointF> m_snake;       // head at index 0, grid coords
    QPointF          m_apple;
    QPointF          m_currentDir;
    QPointF          m_queuedDir;
    bool             m_hasQueuedDir = false;
    bool             m_gameOver = false;
    bool             m_aiMode = false;
    int              m_score    = 0;
    int              m_stepsSinceLastFood = 0;

    void placeApple();

    QPointF directionToVector(Direction dir) const;
    bool isSafeMove(QPointF head, QPointF dir) const;
    void processMove(Direction dir);

    // RL helpers
    QJsonArray buildObservationState(QPointF head, QPointF apple) const;
    Direction  fallbackDirection(QPointF head) const;
    Direction  actionToDirection(int action, bool *ok) const;
    int  sendActRequest(const QJsonArray &state, bool *ok) const;
    bool sendTrainPacket(const QJsonArray &state, int action, double reward,
                         const QJsonArray &nextState, bool done, int snakeSize, bool starved) const;

};
