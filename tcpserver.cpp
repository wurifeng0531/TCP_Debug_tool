#include "tcpserver.h"

namespace TCP {
    TcpServer::TcpServer(){
        m_startListen = false;
        m_threadQuit = false;
        m_sndConDisconnected = false;
        m_dataSndSock = nullptr;
        m_MaxConnNum = 0;
        m_ConnNum = 0;
    }

    TcpServer::~TcpServer(){
        /*在析构函数中delete掉run所new的对象会报错:QSocketNotifier: socket notifiers cannot be disabled from another thread.
        ASSERT failure in QCoreApplication::sendEvent: “Cannot send events to objects owned by a different thread.
        */
        m_threadQuit = true;
        quit();
        wait();
    }

    quint64 TcpServer::send(bool textOrHex, char *data, quint64 len){
        return 0;
    }

    quint64 TcpServer::recv(bool textOrHex, char *data, quint64 len){
        return 0;
    }

    void TcpServer::onGetRspClient(const QString &selectConn) {
        for(vector<QTcpSocket*>::iterator it = m_connSocks.begin(); it != m_connSocks.end();) {
            if(selectConn.contains((*it)->peerAddress().toString())
                    && selectConn.compare(QString::number((*it)->peerPort(), 10))) {
                m_dataSndSock = *it;
                break;
            } else {
                ++it;
            }
        }
    }

    void TcpServer::onRefuseConn(){
        if(m_dataSndSock->state() == QAbstractSocket::ConnectedState) {
            QByteArray q(10, '0');
            m_dataSndSock->write(q);
            m_dataSndSock->flush();
            QThread::sleep(1);
            m_sndConDisconnected = true;

        }
    }

    void TcpServer::onGetConns(vector<QTcpSocket *> connSocks) {
        m_connSocks.assign(connSocks.begin(), connSocks.end());
    }

    void TcpServer::run() {
//        m_tcpServer = new QTcpServer();//不能放在构造函数中
        QTcpServer *tcpServer = new QTcpServer();
        vector<QTcpSocket*> connSocks;
        QTcpSocket *pQTcpSock;
        QString ipAndPortStr;
        QString delIpAndPortStr;
        bool isConsStateChanged = false;//有新连接到来/有连接断开
        bool hasNewConn = false;
        //TODO: connect来自界面的信号与槽函数send/recv

        connect(this, &TcpServer::getConnections, &TcpServer::onGetConns);
        while(true) {
            //侦听并等待连接
            if(m_startListen == true) {
                //最大连接数被修改 则增加vector<QTcpSocket> connSocks的长度
                if(connSocks.capacity() < m_MaxConnNum) {
                    connSocks.reserve(m_MaxConnNum);
                    tcpServer->setMaxPendingConnections(m_MaxConnNum);
                } else if(connSocks.capacity() > m_MaxConnNum) {
                    //比较上次循环时已接入的连接数和最大连接数
                    if(connSocks.size() > m_MaxConnNum) {
                        //最后接入的(connSocks.size() - m_MaxConnNum)个连接最先被删除
                        connSocks.resize(m_MaxConnNum);
                    } else {
                        ;//多余的空间暂时不删除
                    }
                    tcpServer->setMaxPendingConnections(m_MaxConnNum);
                }
                //删除已断开的连接
                for(vector<QTcpSocket*>::iterator it = connSocks.begin(); it != connSocks.end() ;) {
                    //客户端主动断开连接时 服务端的通信socket会进入CLOSE_WAIT状态 if中的语句不会执行
                    //TODO:可考虑在recv和send函数中作处理 读写不成功则根据err类型断连接
                    if((*it)->state() == QAbstractSocket::UnconnectedState) {
                        it = connSocks.erase(it);
                        if(!isConsStateChanged) {
                            isConsStateChanged = true;
                        }
                        delIpAndPortStr = (*it)->peerAddress().toString() + ":"
                                + QString::number((*it)->peerPort(), 10) + "\n";
                        ipAndPortStr.remove(delIpAndPortStr);
                    } else {
                        ++it;
                    }
                }
                //返回当前连接的ip和port
                if(isConsStateChanged) {
                    emit showIpAndPorts(ipAndPortStr);//通知ui更新连接信息
                    emit getConnections(connSocks);//将获取的socket通过信号槽机制传递给主线程的TcpServer对象
                    isConsStateChanged = false;
                }
                //计算当前连接数
                m_ConnNum = connSocks.size();
                //未达到最大连接数
                if(m_MaxConnNum > m_ConnNum) {
                    //侦听
                    if(!tcpServer->isListening()) {
                        tcpServer->listen(QHostAddress::Any, m_port);
                    }
                    //连接保活 默认时间又OS决定 通常是7200s--这里可能有问题 无法检测客户端的断开
//                    pQTcpSock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
                    hasNewConn = tcpServer->waitForNewConnection();
                    if(hasNewConn) {
                        //有新连接到来
                        isConsStateChanged = true;
                        pQTcpSock = tcpServer->nextPendingConnection();
                        connSocks.push_back(pQTcpSock);
                        ipAndPortStr += pQTcpSock->peerAddress().toString() + ":"
                                + QString::number(pQTcpSock->peerPort(), 10) + "\n";
//                        pQTcpSock->peerAddress().toIPv4Address();
                        m_ConnNum++;
                    } else {
                        //暂无新连接 等待
                        hasNewConn = false;
                        QThread::usleep(10);
                    }
                } else {//已达到最大连接数
                    tcpServer->close();
                    QThread::sleep(3);//检查最大连接数的时间间隔可以放宽一点
                }
            } else {
                if(tcpServer->isListening()) {
                    tcpServer->close();
                }
                //断开所有连接
                while(m_ConnNum > 0) {
                    if((connSocks[m_ConnNum-1])->state() == QAbstractSocket::ConnectedState) {
                        (connSocks[m_ConnNum-1])->close();
                    }
                    m_ConnNum--;
                }
                if(ipAndPortStr.length()) {
                    ipAndPortStr = "";
                    emit showIpAndPorts(ipAndPortStr);//通知ui更新连接信息
                    connSocks.clear();
                    emit getConnections(connSocks);//将获取的socket通过信号槽机制传递给主线程的TcpServer对象
                }
                QThread::usleep(20);
            }

            //TcpServer析构时释放tcpServer
            if(m_threadQuit) {
                tcpServer->close();
                delete tcpServer;
                tcpServer = nullptr;
                break;//break不能少！！！
            }
            //断开连接 不能在run()外面关断socket
            if(m_sndConDisconnected) {
                m_dataSndSock->disconnectFromHost();
                m_dataSndSock->waitForConnected();//默认3秒超时
                m_sndConDisconnected = false;
            }
        }
    }
}
