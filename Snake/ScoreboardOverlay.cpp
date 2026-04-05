#include "ScoreboardOverlay.h"

#include <QColor>
#include <QFont>
#include <QPen>
#include <QBrush>

ScoreboardOverlay::ScoreboardOverlay(QGraphicsScene *scene, qreal width, qreal height)
    : m_scene(scene), m_width(width), m_height(height)
{
    buildPanel();
    setVisible(false);
}

void ScoreboardOverlay::buildPanel()
{
    const qreal panelW = m_width  * 0.82;
    const qreal panelH = m_height * 0.74;
    const qreal panelX = (m_width  - panelW) / 2.0;
    const qreal panelY = (m_height - panelH) / 2.0;

    m_bg = m_scene->addRect(panelX, panelY, panelW, panelH);
    m_bg->setBrush(QColor(20, 20, 20, 235));
    m_bg->setPen(QPen(QColor(120, 255, 120), 2));
    m_bg->setZValue(20);

    QFont titleFont("Consolas", 15, QFont::Bold);
    QFont rowFont  ("Consolas", 11);
    QFont btnFont  ("Consolas", 12);

    m_title = m_scene->addText(QString(), titleFont);
    m_title->setHtml("<div style='text-align:center;'>TOP SCORES</div>");
    m_title->setDefaultTextColor(QColor(120, 255, 120));
    m_title->setTextWidth(panelW);
    m_title->setZValue(21);
    m_title->setPos(panelX, panelY + panelH * 0.06);

    for (int i = 0; i < MaxRows; ++i) {
        QGraphicsTextItem *row = m_scene->addText(QString(), rowFont);
        // Gold for #1, light green for the rest
        row->setDefaultTextColor(i == 0 ? QColor(255, 200, 60) : QColor(190, 220, 190));
        row->setZValue(21);
        m_rows.append(row);
    }

    m_continueBtn = m_scene->addText("[ Continue ]", btnFont);
    m_continueBtn->setDefaultTextColor(QColor(120, 255, 120));
    m_continueBtn->setZValue(21);
}

void ScoreboardOverlay::show(const QVector<ScoreEntry> &entries)
{
    m_action = Action::None;
    populateTable(entries);
    setVisible(true);
}

void ScoreboardOverlay::populateTable(const QVector<ScoreEntry> &entries)
{
    const QRectF bg      = m_bg->rect();
    const qreal rowStartY = bg.y() + bg.height() * 0.22;
    const qreal rowSpacing = bg.height() * 0.115;

    for (int i = 0; i < MaxRows; ++i) {
        if (i < entries.size()) {
            const ScoreEntry &e  = entries.at(i);
            const QString rank   = QString("#%1").arg(i + 1);
            const QString name   = e.name.leftJustified(15, QLatin1Char(' '));
            const QString score  = QString::number(e.score).rightJustified(5, QLatin1Char(' '));
            m_rows[i]->setPlainText(QString("%1  %2  %3").arg(rank, name, score));
        } else {
            m_rows[i]->setPlainText(QString("#%1  ---").arg(i + 1));
        }

        const qreal rowW = m_rows[i]->boundingRect().width();
        m_rows[i]->setPos(bg.x() + (bg.width() - rowW) / 2.0,
                          rowStartY + i * rowSpacing);
        m_rows[i]->setVisible(true);
    }

    const qreal btnW = m_continueBtn->boundingRect().width();
    m_continueBtn->setPos(bg.x() + (bg.width() - btnW) / 2.0,
                          bg.y() + bg.height() * 0.875);
}

void ScoreboardOverlay::handleKey(QKeyEvent *event)
{
    const int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape)
        m_action = Action::Continue;
}

void ScoreboardOverlay::setVisible(bool visible)
{
    m_bg->setVisible(visible);
    m_title->setVisible(visible);
    for (auto *r : m_rows)
        r->setVisible(visible);
    m_continueBtn->setVisible(visible);
}

ScoreboardOverlay::Action ScoreboardOverlay::poll()
{
    const Action a = m_action;
    m_action = Action::None;
    return a;
}
