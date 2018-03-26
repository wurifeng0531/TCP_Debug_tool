#include "tcpServer.h"

namespace TCP {
    TcpServer::TcpServer() {
        qRegisterMetaType<qintptr>("qintptr");
        m_RspClientThread = nullptr;
        m_RspClientSock = nullptr;
        m_startListen = false;
        m_sndConDisconnected = false;
        m_MaxConnNum = 0;
        m_ConnNum = 0;
    }

    TcpServer::~TcpServer() {}

    void TcpServer::incomingConnection(const qintptr socketDescriptor) {
        if(m_ConnNum == m_MaxConnNum) {
            this->close();
            return;
        }
        m_ConnNum++;
        QThread *tp = tcpsockHandleThread::getClass().getThread();
        TcpSocket *tmpTs = new TcpSocket(socketDescriptor);
        m_tcpClients.insert(tp, tmpTs);
        m_ipAndPortStr.push_back(tmpTs->peerAddress().toString() + ":"
                + QString::number(tmpTs->peerPort(), 10));
        emit sigShowIpAndPorts(m_ipAndPortStr);
        connect(this, &TcpServer::sigSockDisconnect, tmpTs, &TcpSocket::onSockDisconnect);
        connect(this, &TcpServer::sigSendDataToClient, tmpTs, &TcpSocket::onSend);
        connect(this, &TcpServer::sigChangeRcvDataType, tmpTs, &TcpSocket::onChangeRcvDataType);
        connect(tmpTs, &TcpSocket::sigSockIsDisconnected, this, &TcpServer::onRemoveSelectThread);
        connect(tmpTs, &TcpSocket::sigGetData, this, &TcpServer::onGetDataForServer);
        connect(tmpTs, &TcpSocket::sigPeerIsDisconnected, this, &TcpServer::onClientIsDisconnected);
        connect(this, &TcpServer::sigClientIsDisconnected, this, &TcpServer::onRemoveSelectThread);
        tmpTs->moveToThread(tp);
    }

    void TcpServer::onStartListening() {
        m_startListen = true;
        if(m_ipAndPortStr.capacity() < m_MaxConnNum) {
            m_ipAndPortStr.reserve(m_MaxConnNum);
        } else if(m_ipAndPortStr.capacity() > m_MaxConnNum) {
            if(m_ipAndPortStr.size() > m_MaxConnNum) {
                m_ipAndPortStr.resize(m_MaxConnNum);
            } else { ; }//多余的空间暂时不删除
        }
        this->listen(QHostAddress::Any, m_port);
    }

    void TcpServer::onStopListening() {
        m_startListen = false;
        this->close();
        for(auto itC = m_tcpClients.begin(); itC != m_tcpClients.end(); itC++) {
            emit sigSockDisconnect(itC.key(), itC.value()->socketDescriptor());
            QThread::msleep(100);
        }
        if(m_tcpClients.size() > 0) {
            m_tcpClients.clear();
        }
        if(m_ipAndPortStr.size() > 0) {
            m_ipAndPortStr.clear();
            emit sigShowIpAndPorts(m_ipAndPortStr);
        }
    }

    void TcpServer::onGetRspClient(QString &ip, QString &port) {
        for(auto it = m_tcpClients.begin(); it != m_tcpClients.end();) {
            if((it.value()->peerAddress().toString()).contains(ip)
            && (QString::compare(QString::number(it.value()->peerPort(), 10), port) == 0)) {
                m_RspClientThread = it.key();
                m_RspClientSock = it.value();
                break;
            } else { ++it; }
        }
    }

    void TcpServer::onDisConn(){
        auto itC = m_tcpClients.find(m_RspClientThread);
        QStringList ipAndPort;
        int len = 0;

        if((itC != m_tcpClients.end()) && (itC.key() == m_RspClientThread) && (itC.value() == m_RspClientSock)) {
            if(m_RspClientSock->state() == QAbstractSocket::ConnectedState) {
                for(auto itS = m_ipAndPortStr.begin(); itS != m_ipAndPortStr.end();) {
                    ipAndPort = (*itS).split(':');
                    len = ipAndPort.length();
                    if(len >= 2) {
                        if((m_RspClientSock->peerAddress().toString().contains(ipAndPort[len-2]))
                        && (QString::compare(QString::number(m_RspClientSock->peerPort(), 10), ipAndPort[len-1]) == 0)) {
                            itS = m_ipAndPortStr.erase(itS);
                            break;
                        } else { ++itS; }
                    }
                }
                emit sigSockDisconnect(m_RspClientThread, m_RspClientSock->socketDescriptor());
            } else {
                ;//TODO:可以考虑在UI上弹出警告
            }
        } else {
            ;//TODO:可以考虑在UI上弹出警告
        }
    }

    void TcpServer::onRemoveSelectThread(QThread *tp) {
        m_ConnNum--;
        tcpsockHandleThread::getClass().removeThread(tp);
        m_tcpClients.remove(tp);
        if(m_startListen == true) {
            emit sigShowIpAndPorts(m_ipAndPortStr);
        }
    }

    void TcpServer::onSendDataToClient(const char *data, qint64 len) {
        emit sigSendDataToClient(m_RspClientSock, data, len);
    }

    void TcpServer::onGetDataForServer(TcpSocket * tsp, const QByteArray &dataForServer) {
        emit sigGetDataForServer(tsp, dataForServer);
    }

    void TcpServer::onClientIsDisconnected(TcpSocket *tsp, QString ip, QString port) {
        QStringList ipAndPort;
        int len = 0;
        for(auto itS = m_ipAndPortStr.begin(); itS != m_ipAndPortStr.end();) {
            ipAndPort = (*itS).split(':');
            len = ipAndPort.length();
            if(len >= 2) {
                if( (ip.contains(ipAndPort[len-2]))
                && (QString::compare(port, ipAndPort[len-1]) == 0)) {
                    itS = m_ipAndPortStr.erase(itS);
                    break;
                } else { ++itS; }
            }
        }
        for(auto itC = m_tcpClients.begin(); itC != m_tcpClients.end();) {
            if(itC.value() == tsp){
                emit sigClientIsDisconnected(itC.key());
                break;
            } else {
                ++itC;
            }
        }
    }

}
