#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "tcpsocket.h"
namespace TCP {
    class TcpClient : public QObject{
        Q_OBJECT
    public:
        TcpClient();
        ~TcpClient();
    private:
        TcpSocket *m_workTsp;
        QString m_serverIpAndPortStr;
    public slots:
        void onConnectToExServer(const QString ipAddr, const int port);
        void onExServerIsDisconnected(TcpSocket *tsp, QString ip, QString port);
        void onGetDataFromExServer(TcpSocket *tsp, const QByteArray &data);
        void onSendDataToExServer(const char *data, qint64 len);
    signals:
        void sigShowExServer(const QString &ipAndPort);
        void sigGetDataFromExServer(TCP::TcpSocket *tsp, const QByteArray &data);
        void sigSockDisconnect();
        void sigSendDataToExServer(TcpSocket *tsp, const char *data, qint64 len);
    };
}

#endif // TCPCLIENT_H
