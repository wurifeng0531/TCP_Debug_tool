#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlineedit.h>
#include "tcpserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void selectAConToRsp(const QString &selectConn);
    void refuseConn();
private slots:
//    void on_IpAddr_editingFinished();
    void on_listenPort_editingFinished();
    void on_listeningTrig_clicked();    
    void on_maxConNum_valueChanged(int arg1);
    void onShowIpAndPort(QString ipAndPortStr);
    void on_ipAndPortsDisplay_currentIndexChanged(const QString &arg1);

    void on_refuseConnTrig_clicked();

private:
    Ui::MainWindow *ui;
    TCP::TcpServer *Ts;
};

#endif // MAINWINDOW_H
