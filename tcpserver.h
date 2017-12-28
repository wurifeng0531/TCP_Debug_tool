#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QThread>
#include <vector>
//#include "mainwindow.h"
using namespace std;

namespace TCP {
    /** 客户端连接管理类
     * @brief The TcpServer class
     * 1.同时接受多个客户端的连接，客户端的ip和端口号在ui中显示
     * 2.同时接收多个客户端发来的数据
     * 3.任一时刻只能向其中一个客户端发送数据（在ui中选择客户端）
     */
    class TcpServer : public QObject {
        Q_OBJECT
    public:
//        TcpServer(MainWindow *pMw);
        TcpServer();
        ~TcpServer();
    private:
        size_t m_ConnNum;//当前连接数
//        bool m_threadQuit;//线程退出标志
        bool m_sndConDisconnected;//断连接标志
    public:
//        MainWindow *m_pMw;
        QTcpServer *m_tcpServer;
        vector<QTcpSocket*> m_connSocks;//可连接的socket队列 有新连接到来则记入队列中
        QTcpSocket* m_dataSndSock;//某一时刻相应的客户端socket
        bool m_startListen;//true:开始侦听，false：停止侦听
        int m_port;//侦听的端口号
        size_t m_MaxConnNum;//最大连接数
    public:
        /** 接收客户端的文本或十六进制数据
        * @brief recv
        * @param textOrHex          0：按文本方式接收；1：按十六进制/二进制方式接收
        * @param data               要接收的数据
        * @param len                要接收的数据长度
        */
        qint64 recv(int textOrHex, char *data, qint64 len);
    signals:
        /** 析构信号
         * @brief sigDestruct
         */
        void sigDestruct();
        /** 通知ui显示客户端的ip和端口号
         * @brief showIpAndPortsSig
         * @param ipAndPorts
         * 有新连接到来/有连接断开/服务端停止侦听时，发送信号
         */
        void sigShowIpAndPorts(QString ipAndPorts);
        /** 在线程run()内获得的已连接socket，通过信号发送给主线程的Ts对象
         * @brief getConnectionsSig
         * @param connSocks         已连接socket
         */
        void sigGetConnections(vector<QTcpSocket*> connSocks);
        /**
         * @brief sigRspToClientFinish
         */
//        void sigRspToClientFinish(char *data);
        void sigRspToClientFinish();
    public slots:
        /** 启动侦听 管理客户端连接
         * @brief startListening
         */
        void onStartListening();
        /** 选中客户端
         * @brief getRspClient
         * @param selectConn        字符串中包含ip地址和端口号
         */
        void onGetRspClient(const QString &selectConn);
        /** 断开/拒绝客户端连接
         * @brief refuseConn
         * @return                  true:断开成功；false:断开异常
         */
        void onRefuseConn();
        /** 向客户端发送文本或十六进制数据
        * @brief send
        * @param textOrHex          0:按文本方式发送；1:按十六进制/二进制方式发送
        * @param data               要发送的数据
        * @param len                要发送的数据长度
        */
        qint64 onSend(int textOrHex, char *data, qint64 len);
    private slots:
        /** 析构槽函数
         * @brief onDestruct
         */
        void onDestruct();
        /** 赋值给m_connSocks
         * @brief onGetConns
         * @param connSocks        已连接socket
         */
        void onGetConns(vector<QTcpSocket*> connSocks);
    };
}


#endif // TCPSERVER_H
