#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    w.Ts->moveToThread(w.m_thread);
//    w.m_thread->start();
//    w.Ts->startListening();

    return a.exec();
}
