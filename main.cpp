#include <QApplication>

#include "mainwindow.h"
#include "shclientwidget.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SHClientWidget w;
    w.show();

    return a.exec();
}
