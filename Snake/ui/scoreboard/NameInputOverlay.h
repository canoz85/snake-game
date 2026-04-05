#pragma once

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>

// In-scene overlay that lets the player type their name after setting a new
// all-time high score. Confirmed when the user presses Enter or Escape.
// typedName() returns an empty string if they chose to skip (Escape).
class NameInputOverlay
{
public:
    NameInputOverlay(QGraphicsScene *scene, qreal width, qreal height);

    // Populate title with the given score and make the overlay visible.
    void show(int score);

    // Forward a key event. After this call isConfirmed() may become true.
    void handleKey(QKeyEvent *event);

    void setVisible(bool visible);

    bool    isConfirmed() const { return m_confirmed; }
    QString typedName()   const { return m_name; }   // empty → player skipped

private:
    void buildPanel();
    void updateDisplay();

    QGraphicsScene    *m_scene     = nullptr;
    qreal              m_width     = 0;
    qreal              m_height    = 0;

    QGraphicsRectItem *m_bg        = nullptr;
    QGraphicsTextItem *m_titleText = nullptr;
    QGraphicsTextItem *m_inputText = nullptr;
    QGraphicsTextItem *m_hintText  = nullptr;

    QString m_name;
    bool    m_confirmed = false;

    static constexpr int MaxNameLength = 15;
};
