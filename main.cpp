#include <QApplication>
#include <QDebug>

#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "start";

    MainWindow w;
    w.show();

    return a.exec();
}
