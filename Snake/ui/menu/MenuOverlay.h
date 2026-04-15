#pragma once

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QStringList>
#include <QVector>
#include <QTextCursor>

class MenuOverlay
{
public:
    enum class Action {
        None,
        StartNormal,
        OpenAIMenu,
        StartNeuralUI,
        StartNeuralTrain,
        StartRuleBased,
        BackToMenu,
        Scoreboard,
        Exit
    };

    MenuOverlay(QGraphicsScene *scene, qreal width, qreal height);

    void showStartMenu();
    void showAIModeMenu();
    void showGameOverMenu(int score);

    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    void moveSelectionUp();
    void moveSelectionDown();
    Action activateSelection() const;
    Action activateNumber(int number) const;

private:
    Action actionForItem(const QString &item) const;
    void setMenu(const QString &title, const QString &hint, const QStringList &items);
    void updateVisuals();
    void applyTextAlignment(QGraphicsTextItem *item, Qt::Alignment alignment);

    QGraphicsScene *m_scene = nullptr;
    QGraphicsRectItem *m_background = nullptr;
    qreal m_width = 0;
    qreal m_height = 0;
    bool m_visible = true;

    QGraphicsTextItem *m_title = nullptr;
    QGraphicsTextItem *m_hint = nullptr;
    QVector<QGraphicsTextItem *> m_itemTexts;
    QVector<QGraphicsTextItem *> m_itemMarkers;

    QStringList m_items;
    int m_selection = 0;
};
