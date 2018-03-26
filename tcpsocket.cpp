#include "tcpSocket.h"

namespace TCP {
    TcpSocket::TcpSocket() {
        m_selfDisFlag = false;
        connect(this, &TcpSocket::readyRead, this, &TcpSocket::onRecv);
        connect(this, &TcpSocket::disconnected, [&](){
            if(m_selfDisFlag == false) {
                emit sigPeerIsDisconnected(this, this->peerAddress().toString(),
                                             QString::number(this->peerPort(), 10));
            }
           });
    }

    TcpSocket::TcpSocket(const qintptr socketDescriptor) : m_sockId(socketDescriptor) {
        m_selfDisFlag = false;
        m_rcvRawDataOrFile = false;
        qRegisterMetaType<qintptr>("qintptr");
        this->setSocketDescriptor(socketDescriptor);
        connect(this, &TcpSocket::readyRead, this, &TcpSocket::onRecv);
        m_disFlag = connect(this, &TcpSocket::disconnected, [&](){
            if(m_selfDisFlag == false) {
                emit sigPeerIsDisconnected(this, this->peerAddress().toString(),
                                             QString::number(this->peerPort(), 10));
            }
        });
    }

    TcpSocket::~TcpSocket() {
        this->close();
        m_sockId = 0;
    }

    void TcpSocket::onRecv()
    {
        if(m_rcvRawDataOrFile == false) {//接收原始数据
            m_sockInfoAndData = this->readAll();
            emit sigGetData(this, m_sockInfoAndData);
        } else {//接收文件
            QFile rcvFile(m_rcvFileName);
            if(!rcvFile.open(QIODevice::WriteOnly)) {
                //TODO:记录日志
            }
            QDataStream saveFile(&rcvFile);
            //TODO:保存文件！
            rcvFile.close();
        }
    }

    void TcpSocket::onSend(TcpSocket *tsp, const char *data, qint64 len) {
        if(tsp == this) {
            if(this->state() == QAbstractSocket::ConnectedState) {
                this->write(data, len);
                this->flush();
            }
        }
    }

    void TcpSocket::onSockDisconnect(QThread *tp, const qintptr socketDescriptor) {
        if(socketDescriptor == m_sockId) {
            m_selfDisFlag = true;
            this->disconnectFromHost();
            emit sigSockIsDisconnected(tp);
        }
    }

    void TcpSocket::onDisconnectFromExServer() {
        if(this->state() == QAbstractSocket::ConnectedState) {
            m_selfDisFlag = true;
            this->disconnectFromHost();
        }
    }

    void TcpSocket::onChangeRcvDataType(bool rawDataOrFile, QString fileName, qint64 fileBytesLen)
    {
        m_rcvRawDataOrFile = rawDataOrFile;//true:file;false:raw data
        m_rcvFileName = fileName;
        m_rcvFileBytesLen = fileBytesLen;
    }

}
