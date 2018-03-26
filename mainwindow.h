#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlineedit.h>
#include <QThread>
#include "tcpServer.h"
#include "tcpClient.h"
#include <QFileInfo>

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
    void sigStartListening();
    void sigStopListening();
    void sigSelectAConToRsp(QString &ip, QString &port);
    void sigDisConn();
    void sigSendDataToClient(const char *data, qint64 len);
    void sigConnectToExServer(const QString ipAddr, const int port);
    void sigSendDataToExServer(const char *data, qint64 len);
    void sigDisConnFromExServer();
    /** 接收原始数据或文件
     * @brief sigChangeRcvDataType
     * @param dataOrFile                true:文件；false:原始数据
     * @param fileName                  文件名默认为空
     */
    void sigChangeRcvDataType(bool rawDataOrFile, QString fileName=nullptr, qint64 fileBytesLen=0);
private slots:
    void onShowIpAndPort(vector<QString> ipAndPortStr);
    void on_listenPort_editingFinished();
    void on_listeningTrig_clicked();    
    void on_maxConNum_valueChanged(int arg1);
    void on_ipAndPortsDisplay_currentIndexChanged(const QString &selectConn);
    void on_refuseConnTrig_clicked();
    void on_rcvTextSelect_clicked();
    void on_rcvHexSelect_clicked();
    void on_sndTextSelect_clicked();
    void on_sndHexSelect_clicked();
    void on_sndDataTrig_clicked();
    void on_clearRcvDataTrig_clicked();
    void on_rcvFileSelect_clicked(bool checked);
    void on_clearSndDataTrig_clicked();
    void on_connectToExternalServerTrig_clicked();
    void onShowExServer(const QString &ipAndPort);

    void on_rcvTextSelect_client_clicked();

    void on_rcvHexSelect_client_clicked();

    void on_sndTextSelect_client_clicked();

    void on_sndHexSelect_client_clicked();

    void on_clearRcvDataTrig_client_clicked();

    void on_sndDataTrig_client_clicked();

    void on_clearSndDataTrig_client_clicked();

public slots:
    void onGetDataForServer(TCP::TcpSocket *tsp, const QByteArray &dataForServer);
    void onGetDataFromExServer(TCP::TcpSocket *tsp, const QByteArray &data);
private:
    Ui::MainWindow *ui;
    TCP::TcpServer *Ts;
    QString m_dataQStrSnd;//服务器使用--发送的数据
    char *m_dataSnd;//服务器使用--发送的数据
    int m_rcvtextOrHex;//服务器使用--按文本或十六进制接收 0:文本 1:十六进制
    int m_sndOrHex;//服务器使用--按文本或十六进制发送 0:文本 1:十六进制
    bool m_rcvDataOrFile;//服务器使用--接收（少量）数据或者接收文件，true:文件，false:数据
    TCP::TcpClient *Tc;
    QString m_dataQStrSnd_client;//客户端使用--发送的数据
    char *m_dataSnd_client;//客户端使用--发送的数据
    int m_rcvtextOrHex_client;//客户端使用--按文本或十六进制接收 0:文本 1:十六进制
    int m_sndOrHex_client;//客户端使用--按文本或十六进制发送 0:文本 1:十六进制
    bool m_rcvDataOrFile_client;//客户端使用--接收（少量）数据或者接收文件，true:文件，false:数据
};

#endif // MAINWINDOW_H
