#pragma once

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QVector>
#include "data/ScoreDB.h"

// In-scene overlay that displays the top-5 leaderboard.
// Shown automatically after every game and accessible from the start menu.
// The caller drives the Continue action by forwarding key events and
// calling poll() after each handleKey().
class ScoreboardOverlay
{
public:
    enum class Action { None, Continue };

    ScoreboardOverlay(QGraphicsScene *scene, qreal width, qreal height);

    void show(const QVector<ScoreEntry> &entries);
    void handleKey(QKeyEvent *event);
    void setVisible(bool visible);

    // Returns Action::Continue once (then resets to None).
    Action poll();

private:
    void buildPanel();
    void populateTable(const QVector<ScoreEntry> &entries);

    QGraphicsScene    *m_scene       = nullptr;
    qreal              m_width       = 0;
    qreal              m_height      = 0;

    QGraphicsRectItem *m_bg          = nullptr;
    QGraphicsTextItem *m_title       = nullptr;
    QVector<QGraphicsTextItem *> m_rows;
    QGraphicsTextItem *m_continueBtn = nullptr;

    Action m_action = Action::None;

    static constexpr int MaxRows = 5;
};
