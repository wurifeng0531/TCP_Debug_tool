#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlineedit.h>
#include <QThread>
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
    void sigSelectAConToRsp(const QString &selectConn);
    void sigRefuseConn();
    qint64 sigSend(int textOrHex, char *data, qint64 len);
    void sigStartListening(char *data);
private slots:
//    void on_IpAddr_editingFinished();
    void on_listenPort_editingFinished();
    void on_listeningTrig_clicked();    
    void on_maxConNum_valueChanged(int arg1);
    void onShowIpAndPort(QString ipAndPortStr);
    void on_ipAndPortsDisplay_currentIndexChanged(const QString &arg1);
    void on_refuseConnTrig_clicked();

    void on_rcvTextSelect_clicked();

    void on_rcvHexSelect_clicked();

    void on_sndTextSelect_clicked();

    void on_sndHexSelect_clicked();

    void on_sndDataTrig_clicked();
public slots:
//    void onRspToClientFinish(const char *data);
private:
    Ui::MainWindow *ui;
//public:
    TCP::TcpServer *Ts;
//    TCP::DataProcess *Dp;
    QString m_dataQStrSnd;
    char *m_dataSnd;
    QThread *m_thread;
    int m_rcvtextOrHex;//按文本或十六进制接收 0:文本 1:十六进制
    int m_sndOrHex;//按文本或十六进制发送 0:文本 1:十六进制
};

#endif // MAINWINDOW_H
