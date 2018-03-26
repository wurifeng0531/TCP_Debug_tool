#include "tcpClient.h"

namespace TCP {

    TcpClient::TcpClient() {
        m_workTsp = new TcpSocket();
        //主动断开连接
        connect(this, &TcpClient::sigSockDisconnect, m_workTsp, &TcpSocket::onDisconnectFromExServer);
        //对端断开连接
        connect(m_workTsp, &TcpSocket::sigPeerIsDisconnected, this, &TcpClient::onExServerIsDisconnected);
        connect(m_workTsp, &TcpSocket::sigGetData, this, &TcpClient::onGetDataFromExServer);
        connect(this, &TcpClient::sigSendDataToExServer, m_workTsp, &TcpSocket::onSend);
    }

    TcpClient::~TcpClient()
    {
        delete m_workTsp;
        m_workTsp = nullptr;
    }

    void TcpClient::onConnectToExServer(const QString ipAddr, const int port)
    {
        m_workTsp->connectToHost(QHostAddress(ipAddr), port);
        bool connectedFlag = m_workTsp->waitForConnected();//使用默认值，等待3秒
        if(connectedFlag) {
            m_serverIpAndPortStr = m_workTsp->peerAddress().toString() + ":"
                    +QString::number(m_workTsp->peerPort(), 10);
            emit sigShowExServer(m_serverIpAndPortStr);
        }
    }

    void TcpClient::onExServerIsDisconnected(TcpSocket *tsp, QString ip, QString port)
    {
//        if((tsp == m_workTsp) && (m_serverIpAndPortStr.contains(ip))
//                && (m_serverIpAndPortStr.contains(port)) ) {
//            emit sigShowExServer("");
//        }
        emit sigShowExServer("");
    }

    void TcpClient::onGetDataFromExServer(TcpSocket *tsp, const QByteArray &data)
    {
        emit sigGetDataFromExServer(tsp, data);
    }

    void TcpClient::onSendDataToExServer(const char *data, qint64 len)
    {
        emit sigSendDataToExServer(m_workTsp, data, len);
    }


}
