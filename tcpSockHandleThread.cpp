#include "tcpSockHandleThread.h"

namespace TCP {

    tcpsockHandleThread::tcpsockHandleThread() {}

    tcpsockHandleThread::~tcpsockHandleThread() {}

    tcpsockHandleThread &tcpsockHandleThread::getClass() {
        static tcpsockHandleThread handleClass;
        return handleClass;
    }

    QThread *tcpsockHandleThread::getThread() {
        QThread *thread = new QThread();
        thread->start();
        return thread;
    }

    void tcpsockHandleThread::removeThread(QThread *tp) {
        if(tp->isRunning()) {
            tp->quit();
            tp->wait();
            delete tp;
        }
    }

}
