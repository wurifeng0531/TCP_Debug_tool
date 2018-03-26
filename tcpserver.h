#ifndef TCPSERVER_H
#define TCPSERVER_H


#include <QHash>
#include "tcpSocket.h"
#include "tcpSockHandleThread.h"

using namespace std;

namespace TCP {
    /** 客户端连接管理类
     * @brief The TcpServer class
     * 1.同时接受多个客户端的连接，客户端的ip和端口号在ui中显示
     * 2.同时接收多个客户端发来的数据
     * 3.任一时刻只能向其中一个客户端发送数据（在ui中选择客户端）
     */
    class TcpServer : public QTcpServer {
        Q_OBJECT
    public:
        TcpServer();
        ~TcpServer();
    private:
        size_t m_ConnNum;//当前连接数
        bool m_sndConDisconnected;//断连接标志
        vector<QString> m_ipAndPortStr;//客户端列表；有新连接到来，则将其ip和port记录到QString中，返回给UI；连接断开时，将其ip和port记录下来，在m_ipAndPortStr中删除
//        QBuffer m_qbRcv;//从客户端读取的数据
    public:
        QHash<QThread*, TcpSocket*> m_tcpClients;//可连接的“线程-socket”表 有新连接到来则创建线程并记入队列中
        QThread *m_RspClientThread;//某一时刻相应的客户端线程
        TcpSocket *m_RspClientSock;//某一时刻相应的客户端线程
        bool m_startListen;//true:开始侦听，false：停止侦听
        int m_port;//侦听的端口号
        size_t m_MaxConnNum;//最大连接数
    signals:
        /** 通知ui显示客户端的ip和端口号
         * @brief showIpAndPortsSig
         * @param ipAndPorts
         *   有新连接到来/有连接断开/服务端停止侦听时，发送信号
         */
        void sigShowIpAndPorts(vector<QString> ipAndPorts);
        /** 服务器主动断开连接
         * @brief sigSockDisconnect
         * @param tp                        线程指针
         * @param socketDescriptor          socket描述符，当赋值为-1时表示断开所有连接
         */
        void sigSockDisconnect(QThread *tp, const qintptr socketDescriptor);
        /**  给选中的客户端发送数据
         * @brief sigSendDataToClient
         * @param tsp                       socket指针
         * @param data                      要发送的数据
         * @param len                       要发送的数据长度
         */
        void sigSendDataToClient(TcpSocket *tsp, const char *data, qint64 len);
        /** 接收客户端数据
         * @brief sigGetDataForServer
         * @param tsp                       socket指针
         * @param dataForServer             接收的数据
         */
        void sigGetDataForServer(TcpSocket *tsp, const QByteArray &dataForServer);
        /** 客户端断开连接
         * @brief sigClientIsDisconnected
         * @param tp
         */
        void sigClientIsDisconnected(QThread *tp);
        void sigChangeRcvDataType(bool rawDataOrFile, QString fileName=nullptr, qint64 fileBytesLen=0);//true:文件，false:原始数据
    protected:
        /** 重写方法 有新连接到来时的动作
         * @brief incomingConnection
         * @param socketDescriptor          socket描述符
         */
        void incomingConnection(const qintptr socketDescriptor) override;
    public slots:
        /** 启动侦听
         * @brief startListening
         */
        void onStartListening();
        /** 停止侦听
         * @brief onStopListening
         */
        void onStopListening();
        /** 选中客户端
         * @brief onGetRspClient
         * @param ip                            ip
         * @param port                          端口
         */
        void onGetRspClient(QString &ip, QString &port);
        /** 断开/拒绝客户端连接 由UI发信号调用
         * @brief onDisConn
         * @return                              true:断开成功；false:断开异常
         */
        void onDisConn();
        /** 移除线程
         * @brief onRemoveSelectThread
         * @param tp
         */
        void onRemoveSelectThread(QThread *tp);
        /** 给选中的客户端发送数据
         * @brief onSendDataToClient
         * @param data                      要发送的数据
         * @param len                       要发送的数据长度
         */
        void onSendDataToClient(const char *data, qint64 len);
        /** 接收客户端数据
         * @brief onGetDataForServer
         * @param tsp                       socket指针
         * @param dataForServer             接收的数据
         */
        void onGetDataForServer(TcpSocket *tsp, const QByteArray &dataForServer);

        void onClientIsDisconnected(TcpSocket *tsp, QString ip, QString port);
    };
}


#endif // TCPSERVER_H
