#include "mainwindow.h"
#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 禁用所有声音
    //QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();
    return a.exec();
}
