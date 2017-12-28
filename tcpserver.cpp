#include "tcpserver.h"

namespace TCP {
    TcpServer::TcpServer() {
        m_startListen = false;
        m_sndConDisconnected = false;
        m_MaxConnNum = 0;
        m_ConnNum = 0;
    }

    /*在析构函数中delete掉run所new的对象会报错:QSocketNotifier: socket notifiers cannot be disabled from another thread.
    ASSERT failure in QCoreApplication::sendEvent: “Cannot send events to objects owned by a different thread.
    */
    TcpServer::~TcpServer() {
        emit sigDestruct();
    }

    void TcpServer::onGetRspClient(const QString &selectConn) {
        for(vector<QTcpSocket*>::iterator it = m_connSocks.begin(); it != m_connSocks.end();) {
            if(selectConn.contains((*it)->peerAddress().toString())
                    && selectConn.compare(QString::number((*it)->peerPort(), 10))) {
                m_dataSndSock = *it;
                break;
            } else { ++it; }
        }
    }

    void TcpServer::onRefuseConn(){
        if(m_dataSndSock->state() == QAbstractSocket::ConnectedState) {
//            m_dataSndSock->close();
            QByteArray q(10, '0');
            m_dataSndSock->write(q);
            m_dataSndSock->flush();
        }
    }

    /*这里的实现会导致错误：QSocketNotifier: Multiple socket notifiers for same socket..*/
    qint64 TcpServer::onSend(int textOrHex, char *data, qint64 len) {
        qint64 ret = 0;
//        QTcpSocket *sock = new QTcpSocket();
        QByteArray *qa = new QByteArray();
        if(textOrHex == 0) {
            if(m_dataSndSock->state() == QAbstractSocket::ConnectedState) {
//                QTcpSocket *sock = new QTcpSocket();
//                sock->setSocketDescriptor(m_dataSndSock->socketDescriptor());
                qa->setRawData(data,len);
                ret = m_dataSndSock->write(*qa, len);
                m_dataSndSock->flush();
//                emit sigRspToClientFinish(data);
                emit sigRspToClientFinish();
            } else { ret = -1; }
        }
        return ret;
    }

    void TcpServer::onDestruct() {
        m_tcpServer->close();
        delete m_tcpServer;
        m_tcpServer = nullptr;
        m_connSocks.clear();
    }

    void TcpServer::onGetConns(vector<QTcpSocket *> connSocks) {
        m_connSocks.assign(connSocks.begin(), connSocks.end());
    }

    void TcpServer::onStartListening() {
//        QTcpServer *tcpServer = new QTcpServer();
//        vector<QTcpSocket*> connSocks;
        m_tcpServer = new QTcpServer();
        QTcpSocket *pQTcpSock;
        QString ipAndPortStr;
        QString delIpAndPortStr;
        bool isConsStateChanged = false;//有新连接到来/有连接断开
        bool hasNewConn = false;
        //TODO: connect来自界面的信号与槽函数send/recv

        connect(this, &TCP::TcpServer::sigDestruct, this, &TCP::TcpServer::onDestruct);
        connect(this, &TcpServer::sigGetConnections, this, &TcpServer::onGetConns);

        while(true) {
            //侦听并等待连接
            if(m_startListen == true) {
                //最大连接数被修改 则增加vector<QTcpSocket> connSocks的长度
                if(m_connSocks.capacity() < m_MaxConnNum) {
                    m_connSocks.reserve(m_MaxConnNum);
                    m_tcpServer->setMaxPendingConnections(m_MaxConnNum);
                } else if(m_connSocks.capacity() > m_MaxConnNum) {
                    //比较上次循环时已接入的连接数和最大连接数
                    if(m_connSocks.size() > m_MaxConnNum) {
                        //最后接入的(connSocks.size() - m_MaxConnNum)个连接最先被删除
                        m_connSocks.resize(m_MaxConnNum);
                    } else { ; }//多余的空间暂时不删除
                    m_tcpServer->setMaxPendingConnections(m_MaxConnNum);
                }
                //删除已断开的连接
                for(vector<QTcpSocket*>::iterator it = m_connSocks.begin(); it != m_connSocks.end() ;) {
                    //客户端主动断开连接时 服务端的通信socket会进入CLOSE_WAIT状态 if中的语句不会执行
                    //TODO:可考虑在recv和send函数中作处理 读写不成功则根据err类型断连接
                    if((*it)->state() == QAbstractSocket::UnconnectedState) {
                        it = m_connSocks.erase(it);
                        if(!isConsStateChanged) {
                            isConsStateChanged = true;
                        }
                        delIpAndPortStr = (*it)->peerAddress().toString() + ":"
                                + QString::number((*it)->peerPort(), 10) + "\n";
                        ipAndPortStr.remove(delIpAndPortStr);
                    } else { ++it; }
                }
                //返回当前连接的ip和port
                if(isConsStateChanged) {
                    emit sigShowIpAndPorts(ipAndPortStr);//通知ui更新连接信息
//                    emit sigGetConnections(connSocks);//将获取的socket通过信号槽机制传递给主线程的TcpServer对象
                    isConsStateChanged = false;
                }
                //计算当前连接数
                m_ConnNum = m_connSocks.size();
                //未达到最大连接数
                if(m_MaxConnNum > m_ConnNum) {
                    //侦听
                    if(!m_tcpServer->isListening()) {
                        m_tcpServer->listen(QHostAddress::Any, m_port);
                    }
                    //连接保活 默认时间又OS决定 通常是7200s--这里可能有问题 无法检测客户端的断开
//                    pQTcpSock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
                    hasNewConn = m_tcpServer->waitForNewConnection();
                    if(hasNewConn) {
                        //有新连接到来
                        isConsStateChanged = true;
                        pQTcpSock = m_tcpServer->nextPendingConnection();
                        m_connSocks.push_back(pQTcpSock);
                        ipAndPortStr += pQTcpSock->peerAddress().toString() + ":"
                                + QString::number(pQTcpSock->peerPort(), 10) + "\n";
                        m_ConnNum++;
                    } else {
                        //暂无新连接 等待
                        hasNewConn = false;
                        QThread::usleep(10);
                    }
                } else {//已达到最大连接数
                    m_tcpServer->close();
                    QThread::sleep(3);//检查最大连接数的时间间隔可以放宽一点
                    break;
                }
            } else {
                if(m_tcpServer->isListening()) {
                    m_tcpServer->close();
                }
                //断开所有连接
                while(m_ConnNum > 0) {
                    if((m_connSocks[m_ConnNum-1])->state() == QAbstractSocket::ConnectedState) {
                        (m_connSocks[m_ConnNum-1])->close();
                    }
                    m_ConnNum--;
                }
                if(ipAndPortStr.length()) {
                    ipAndPortStr = "";
                    emit sigShowIpAndPorts(ipAndPortStr);//通知ui更新连接信息
                    m_connSocks.clear();
//                    emit sigGetConnections(connSocks);//将获取的socket通过信号槽机制传递给主线程的TcpServer对象
                }
                QThread::usleep(20);
            }

//            //TcpServer析构时释放tcpServer
//            if(m_threadQuit) {
//                tcpServer->close();
//                delete tcpServer;
//                tcpServer = nullptr;
//                break;//break不能少！！！
//            }

//            //断开连接 不能在run()外面关断socket
//            if(m_sndConDisconnected) {
//                m_dp->m_dataSndSock->disconnectFromHost();
//                m_dp->m_dataSndSock->waitForConnected();//默认3秒超时
//                m_sndConDisconnected = false;
//                break;
//            }

        }
    }
}
