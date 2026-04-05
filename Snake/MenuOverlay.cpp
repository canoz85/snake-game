#include "MenuOverlay.h"

#include <QColor>
#include <QFont>
#include <QBrush>
#include <QTextDocument>
#include <QTextBlockFormat>

MenuOverlay::MenuOverlay(QGraphicsScene *scene, qreal width, qreal height)
    : m_scene(scene), m_width(width), m_height(height)
{

    // 1. Create the Background Rectangle
    // We make it slightly smaller than the full screen for a "floating window" look
    qreal rectWidth = m_width * 0.8;
    qreal rectHeight = m_height * 0.6;
    qreal x = (m_width - rectWidth) / 2;
    qreal y = (m_height - rectHeight) / 2;

    m_background = m_scene->addRect(x, y, rectWidth, rectHeight);
    
    // Aesthetic: Dark semi-transparent background with a green border
    m_background->setBrush(QColor(20, 20, 20, 220)); // 220 = slight transparency
    m_background->setPen(QPen(QColor(120, 255, 120), 2)); // Green border
    m_background->setZValue(5); // Lower than text (10)

    QFont titleFont("Consolas", 18, QFont::Bold);
    QFont menuFont("Consolas", 12);

    m_title = m_scene->addText(QString(), titleFont);
    m_title->setDefaultTextColor(QColor(120, 255, 120));
    m_title->setZValue(10);
    m_title->setTextWidth(rectWidth);
    applyTextAlignment(m_title, Qt::AlignCenter);

    m_hint = m_scene->addText(QString(), menuFont);
    m_hint->setDefaultTextColor(QColor(140, 180, 140));
    m_hint->setZValue(10);
    m_hint->setTextWidth(rectWidth);
    applyTextAlignment(m_hint, Qt::AlignCenter);

    for (int i = 0; i < 4; ++i) {
        QGraphicsTextItem *item = m_scene->addText(QString(), menuFont);
        item->setZValue(10);
        m_itemTexts.append(item);

        QGraphicsTextItem *marker = m_scene->addText(QString(), menuFont);
        marker->setDefaultTextColor(QColor(190, 255, 190));
        marker->setZValue(10);
        m_itemMarkers.append(marker);
    }

    m_title->setPos(x, m_height * 0.18);
    m_hint->setPos(x, m_height * 0.28);
}

void MenuOverlay::showStartMenu()
{
    setMenu(
        "SNAKE GAME // ʙᴀᴛᴜ & ᴄᴀɴ",
        "arrows to navigate • enter to select",
        {"Start", "Exit"}
    );
    setVisible(true);
}

void MenuOverlay::showGameOverMenu(int score)
{
    setMenu(
        QString("GAME OVER // SCORE: %1").arg(score),
        "arrows to navigate • enter to select",
        {"Restart", "Back to Menu", "Exit"}
    );
    setVisible(true);
}

void MenuOverlay::setVisible(bool visible)
{
    m_visible = visible;

    m_background->setVisible(visible);
    m_title->setVisible(visible);
    m_hint->setVisible(visible);
    for (auto *item : m_itemTexts)
        item->setVisible(visible);
    for (auto *marker : m_itemMarkers)
        marker->setVisible(visible);
}

void MenuOverlay::moveSelectionUp()
{
    if (m_items.isEmpty())
        return;

    m_selection = (m_selection - 1 + m_items.size()) % m_items.size();
    updateVisuals();
}

void MenuOverlay::moveSelectionDown()
{
    if (m_items.isEmpty())
        return;

    m_selection = (m_selection + 1) % m_items.size();
    updateVisuals();
}

MenuOverlay::Action MenuOverlay::activateSelection() const
{
    if (m_items.isEmpty())
        return Action::None;

    const QString current = m_items.at(m_selection);
    if (current == "Start")
        return Action::Start;
    if (current == "Restart")
        return Action::Restart;
    if (current == "Back to Menu")
        return Action::BackToMenu;
    if (current == "Exit")
        return Action::Exit;

    return Action::None;
}

void MenuOverlay::setMenu(const QString &title, const QString &hint, const QStringList &items)
{
    m_items = items;
    m_selection = 0;

    const QRectF panel = m_background ? m_background->rect() : QRectF(0, 0, m_width, m_height);

    m_title->setPlainText(title);
    m_title->setTextWidth(panel.width());
    m_title->setPos(panel.x(), m_height * 0.18);
    applyTextAlignment(m_title, Qt::AlignCenter);

    m_hint->setPlainText(hint);
    m_hint->setTextWidth(panel.width());
    m_hint->setPos(panel.x(), m_height * 0.28);
    applyTextAlignment(m_hint, Qt::AlignCenter);

    updateVisuals();
}

void MenuOverlay::updateVisuals()
{
    const QRectF panel = m_background ? m_background->rect() : QRectF(0, 0, m_width, m_height);
    const qreal itemY = m_height * 0.40;
    const qreal markerGap = 8.0;
    qreal maxItemWidth = 0.0;

    for (int i = 0; i < m_itemTexts.size(); ++i) {
        const bool active = i < m_items.size();
        if (!active) {
            // Inactive: clear text and hide
            m_itemTexts[i]->setPlainText("");
            m_itemTexts[i]->setVisible(false);
            if (i < m_itemMarkers.size()) {
                m_itemMarkers[i]->setPlainText("");
                m_itemMarkers[i]->setVisible(false);
            }
            continue;
        }

        // Active item
        m_itemTexts[i]->setVisible(m_visible);
        if (i < m_itemMarkers.size())
            m_itemMarkers[i]->setVisible(m_visible);
        const bool selected = (i == m_selection);
        const QString labelText = "[ " + m_items.at(i) + " ]";
        m_itemTexts[i]->setDefaultTextColor(selected ? QColor(190, 255, 190) : QColor(90, 140, 90));
        m_itemTexts[i]->setHtml(
            selected
                ? QString("<div style=\"background-color: rgba(50, 120, 50, 0.55); padding: 3px 10px;\">%1</div>")
                      .arg(labelText.toHtmlEscaped())
                : QString("<div style=\"padding: 3px 10px;\">%1</div>").arg(labelText.toHtmlEscaped())
        );

        maxItemWidth = qMax(maxItemWidth, m_itemTexts[i]->boundingRect().width());
    }

    const qreal labelX = panel.x() + (panel.width() - maxItemWidth) / 2.0;

    for (int i = 0; i < m_itemTexts.size(); ++i) {
        if (i >= m_items.size())
            continue;

        const bool selected = (i == m_selection);
        const qreal labelY = itemY + i * 34;
        m_itemTexts[i]->setPos(labelX, labelY);

        if (i < m_itemMarkers.size()) {
            m_itemMarkers[i]->setPlainText(selected ? "→" : "");
            if (selected) {
                const QRectF markerBounds = m_itemMarkers[i]->boundingRect();
                m_itemMarkers[i]->setPos(labelX - markerBounds.width() - markerGap, labelY);
            }
        }
    }
}

void MenuOverlay::applyTextAlignment(QGraphicsTextItem *item, Qt::Alignment alignment)
{
    if (!item || !item->document())
        return;

    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor(item->document());
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
}
