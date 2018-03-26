#ifndef TCPSOCKTHREAD_H
#define TCPSOCKTHREAD_H
#include <QThread>

namespace TCP {
    /** 线程池管理类 单例 每一个连接建立一个线程
     * @brief The TcpSockThread class
     */
    class tcpsockHandleThread {
    private:
        tcpsockHandleThread();
    public:
        ~tcpsockHandleThread();
        static tcpsockHandleThread & getClass();
        QThread * getThread();
        void removeThread(QThread *tp);
    };
}

#endif // TCPSOCKTHREAD_H
