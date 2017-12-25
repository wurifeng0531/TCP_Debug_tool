#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QThread>
#include <vector>
using namespace std;

namespace TCP {
    /**
     * @brief The TcpServer class
     * 1.同时接受多个客户端的连接，客户端的ip和端口号在ui中显示
     * 2.同时接收多个客户端发来的数据
     * 3.任一时刻只能向其中一个客户端发送数据（在ui中选择客户端）
     */
    class TcpServer : public QThread {
        Q_OBJECT
    public:
        TcpServer();
        ~TcpServer();
    private:
//        QTcpServer *m_tcpServer;
        vector<QTcpSocket*> m_connSocks;//可连接的socket队列 有新连接到来则记入队列中
        QTcpSocket* m_dataSndSock;//某一时刻相应的客户端socket
        size_t m_ConnNum;//当前连接数
        bool m_threadQuit;//线程退出标志
        bool m_sndConDisconnected;//断连接标志
    public:
        bool m_startListen;//true:开始侦听，false：停止侦听
        int m_port;//侦听的端口号
        size_t m_MaxConnNum;//最大连接数
        bool m_textOrHex;//按文本或十六进制发送/接收 true:文本 false:十六进制
    public:
        /** 向客户端发送数据
        * @brief send
        * @param textOrHex          0：按文本方式发送；1：按十六进制/二进制方式发送
        * @param data               要发送的数据
        * @param len                要发送的数据长度
        */
        quint64 send(bool textOrHex, char *data, quint64 len);
        /** 接收客户端的数据
        * @brief recv
        * @param textOrHex          0：按文本方式接收；1：按十六进制/二进制方式接收
        * @param data               要接收的数据
        * @param len                要接收的数据长度
        */
        quint64 recv(bool textOrHex, char *data, quint64 len);
    protected:
        void run() override;
    signals:
        /** 通知ui显示客户端的ip和端口号
         * @brief showIpAndPorts
         * @param ipAndPorts
         * 有新连接到来/有连接断开/服务端停止侦听时，发送信号
         */
        void showIpAndPorts(QString ipAndPorts);
        /**
         * @brief getConnections
         * @param connSocks         在线程run()内获得的已连接socket，通过信号发送给主线程的Ts对象
         */
        void getConnections(vector<QTcpSocket*> connSocks);
    public slots:
        /**
         * @brief getRspClient
         * @param selectConn        选中的客户端，字符串中包含ip地址和端口号
         */
        void onGetRspClient(const QString &selectConn);
        /** 断开/拒绝客户端连接
         * @brief refuseConn
         * @return                  true:断开成功；false:断开异常
         */
        void onRefuseConn();
    private slots:
        /**
         * @brief onGetConns
         * @param connSocks         赋值给m_connSocks
         */
        void onGetConns(vector<QTcpSocket*> connSocks);
    };

}


#endif // TCPSERVER_H
