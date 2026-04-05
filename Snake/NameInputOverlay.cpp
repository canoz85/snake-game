#include "NameInputOverlay.h"

#include <QColor>
#include <QFont>
#include <QPen>
#include <QBrush>

NameInputOverlay::NameInputOverlay(QGraphicsScene *scene, qreal width, qreal height)
    : m_scene(scene), m_width(width), m_height(height)
{
    buildPanel();
    setVisible(false);
}

void NameInputOverlay::buildPanel()
{
    const qreal panelW = m_width  * 0.78;
    const qreal panelH = m_height * 0.44;
    const qreal panelX = (m_width  - panelW) / 2.0;
    const qreal panelY = (m_height - panelH) / 2.0;

    m_bg = m_scene->addRect(panelX, panelY, panelW, panelH);
    m_bg->setBrush(QColor(20, 20, 20, 235));
    m_bg->setPen(QPen(QColor(255, 200, 60), 2));   // gold border = new record
    m_bg->setZValue(20);

    QFont titleFont("Consolas", 13, QFont::Bold);
    QFont inputFont("Consolas", 16, QFont::Bold);
    QFont hintFont ("Consolas", 10);

    m_titleText = m_scene->addText(QString(), titleFont);
    m_titleText->setDefaultTextColor(QColor(255, 200, 60));
    m_titleText->setTextWidth(panelW);
    m_titleText->setZValue(21);
    m_titleText->setPos(panelX, panelY + panelH * 0.10);

    m_inputText = m_scene->addText(QString(), inputFont);
    m_inputText->setDefaultTextColor(QColor(230, 255, 230));
    m_inputText->setZValue(21);

    m_hintText = m_scene->addText("Enter to save  •  Escape to skip", hintFont);
    m_hintText->setDefaultTextColor(QColor(130, 160, 130));
    m_hintText->setZValue(21);
    m_hintText->setPos(panelX + (panelW - m_hintText->boundingRect().width()) / 2.0,
                       panelY + panelH * 0.74);
}

void NameInputOverlay::show(int score)
{
    m_name      = QString();
    m_confirmed = false;

    m_titleText->setHtml(
        QString("<div style='text-align:center;'>"
                "NEW HIGH SCORE: %1<br/>Enter your name:"
                "</div>").arg(score)
    );

    updateDisplay();
    setVisible(true);
}

void NameInputOverlay::handleKey(QKeyEvent *event)
{
    if (m_confirmed)
        return;

    const int key = event->key();

    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        m_confirmed = true;    // empty name = "skip"
        setVisible(false);
        return;
    }

    if (key == Qt::Key_Escape) {
        m_name      = QString();
        m_confirmed = true;
        setVisible(false);
        return;
    }

    if (key == Qt::Key_Backspace) {
        if (!m_name.isEmpty())
            m_name.chop(1);
        updateDisplay();
        return;
    }

    if (event->text().isEmpty())
        return;

    const QChar ch = event->text().at(0);
    if (m_name.length() < MaxNameLength && (ch.isLetterOrNumber() || ch == QLatin1Char(' ')))
        m_name.append(ch);

    updateDisplay();
}

void NameInputOverlay::setVisible(bool visible)
{
    m_bg->setVisible(visible);
    m_titleText->setVisible(visible);
    m_inputText->setVisible(visible);
    m_hintText->setVisible(visible);
}

void NameInputOverlay::updateDisplay()
{
    const QRectF bg  = m_bg->rect();
    const QString cur = m_name + QLatin1Char('|');   // simulated cursor

    m_inputText->setPlainText(cur);

    const qreal textW = m_inputText->boundingRect().width();
    m_inputText->setPos(bg.x() + (bg.width() - textW) / 2.0,
                        bg.y() + bg.height() * 0.44);
}
