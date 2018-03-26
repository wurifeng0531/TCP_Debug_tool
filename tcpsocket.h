#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QThread>
#include <vector>
#include <QMetaType>
#include <QByteArray>
#include <QTextCodec>
#include <QFile>
#include <QDataStream>

using namespace std;

namespace TCP {
    class TcpSocket : public QTcpSocket{
        Q_OBJECT
    public:
        TcpSocket();
        TcpSocket(const qintptr socketDescriptor);
        ~TcpSocket();
    private:
        QMetaObject::Connection m_disFlag;
        QByteArray  m_sockInfoAndData;//接收到的数据
        bool m_selfDisFlag;//主动断开标志位 true则表示主动断开连接
        bool m_rcvRawDataOrFile;//true:文件，false:原始数据
        QString m_rcvFileName;//接收文件后保存的文件名-包括绝对路径
        qint64 m_rcvFileBytesLen;//接收文件的长度
    public:
         qintptr m_sockId;
    private:
        void onRecv();
    public slots:
        /** 发送文本或十六进制数据
         * @brief onSend
         * @param tsp                       对端的socket指针
         * @param data                      要发送的数据
         * @param len                       要发送的数据长度
         */
        void onSend(TcpSocket *tsp, const char *data, qint64 len);
        /** 服务器使用--主动断开与客户端的连接
         * @brief onSockDisconnect
         * @param tp                        线程指针
         * @param socketDescriptor          socke描述符，当赋值为-1时表示断开所有连接
         */
        void onSockDisconnect(QThread *tp, const qintptr socketDescriptor);
        /** 客户端使用--主动断开与外部服务器的连接
         * @brief onDisconnectFromExServer
         */
        void onDisconnectFromExServer();
        /** 改变接收的数据的类型
         * @brief onChangeRcvDataType
         * @param dataOrFile                true:文件，false:原始数据
         * @param fileName                  文件名
         */
        void onChangeRcvDataType(bool rawDataOrFile, QString fileName=nullptr, qint64 fileBytesLen=0);
    signals:
        /** 服务器使用--连接断开后 通知服务器
         * @brief sigSockIsDisconnected
         */
        void sigSockIsDisconnected(QThread *tp);
        /** 接收对端数据
         * @brief sigGetData
         * @param tsp                     socket指针
         * @param dataForServer            接收的数据
         */
        void sigGetData(TcpSocket *tsp, const QByteArray &dataForServer);
        /** 对端主动断开连接
         * @brief sigPeerIsDisconnected
         * @param tsp
         */
        void sigPeerIsDisconnected(TcpSocket *tsp, QString ip, QString port);
    };
}

#endif // TCPSOCKET_H
