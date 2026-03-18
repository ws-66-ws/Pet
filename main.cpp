#include "petwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // PetWidget w;
    // w.show();
    PetWidget::getInstance().show();
    return a.exec();
}
