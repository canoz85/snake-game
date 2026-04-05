#include <QApplication>
#include "SnakeGame.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SnakeGame view;
    view.show();

    return a.exec();
}
