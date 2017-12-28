#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    Ts(new TCP::TcpServer()), m_thread(new QThread()) {
    m_dataSnd = nullptr;
    m_sndOrHex = 0;
    m_rcvtextOrHex = 0;
//    Dp = new TCP::DataProcess();
//    m_thread = new QThread();
//    Dp->moveToThread(m_thread);
//    m_thread->start();
//    Ts = new TCP::TcpServer(*Dp);
    Ts->moveToThread(m_thread);

    ui->setupUi(this);
    connect(Ts, &TCP::TcpServer::sigShowIpAndPorts, this, &MainWindow::onShowIpAndPort,Qt::DirectConnection);
//    connect(Ts, &TCP::TcpServer::sigRspToClientFinish, this, &MainWindow::onRspToClientFinish, Qt::DirectConnection);
    connect(this, &MainWindow::sigStartListening, Ts, &TCP::TcpServer::onStartListening, Qt::QueuedConnection);
    connect(this, &MainWindow::sigSelectAConToRsp, Ts, &TCP::TcpServer::onGetRspClient, Qt::QueuedConnection);
    connect(this, &MainWindow::sigRefuseConn, Ts, &TCP::TcpServer::onRefuseConn, Qt::QueuedConnection);
    connect(this, &MainWindow::sigSend, Ts, &TCP::TcpServer::onSend, Qt::QueuedConnection);


    m_thread->start();
    connect(m_thread, SIGNAL(started()), Ts, SLOT(onStartListening()));
//    emit sigStartListening();
//    Ts->startListening();
}

MainWindow::~MainWindow() {
    m_thread->quit();
    m_thread->wait();
    m_dataSnd = nullptr;
    delete ui;
    delete Ts;
//    delete Dp;
}

//void MainWindow::on_IpAddr_editingFinished(){
//    Ts->m_ipAddr = ui->IpAddr->text();
//}

void MainWindow::on_listenPort_editingFinished() {
    Ts->m_port = ui->listenPort->text().toInt();
}

void MainWindow::on_listeningTrig_clicked() {
    if( (ui->listeningTrig->text() == tr("侦听")) && (!ui->listenPort->text().isEmpty()) ) {
        ui->listeningTrig->setText(tr("停止"));
        Ts->m_startListen = true;
    } else if( (ui->listeningTrig->text() == tr("停止")) && (!ui->listenPort->text().isEmpty()) ) {
        ui->listeningTrig->setText(tr("侦听"));
        Ts->m_startListen = false;
    }
}

void MainWindow::on_maxConNum_valueChanged(int Num) {
    if(Num > 0) {
        Ts->m_MaxConnNum = (size_t)Num;
    } else {
        Ts->m_MaxConnNum = 1;//保证至少能接受一个连接
    }

}

void MainWindow::onShowIpAndPort(QString ipAndPortStr) {
    QStringList clientList = ipAndPortStr.split("\n");
    ui->ipAndPortsDisplay->clear();
    for(int i = 0; i < clientList.length(); i++) {
        if(!clientList[i].isEmpty()) {
            ui->ipAndPortsDisplay->insertItem(i, clientList[i]);
        }
    }
}

void MainWindow::on_ipAndPortsDisplay_currentIndexChanged(const QString &selectConn) {
    emit sigSelectAConToRsp(selectConn);
}

void MainWindow::on_refuseConnTrig_clicked() {
    emit sigRefuseConn();
}

void MainWindow::on_rcvTextSelect_clicked() {
    m_rcvtextOrHex = 0;
}

void MainWindow::on_rcvHexSelect_clicked() {
    m_rcvtextOrHex = 1;
}

void MainWindow::on_sndTextSelect_clicked() {
    m_sndOrHex = 0;
}

void MainWindow::on_sndHexSelect_clicked() {
    m_sndOrHex = 1;
}

void MainWindow::on_sndDataTrig_clicked() {
//    QString m_dataQStrSnd;
//    char *m_dataSnd = nullptr;
    QByteArray ba;
    int len = 0;
    switch (m_sndOrHex) {
    case 0:
        m_dataQStrSnd = ui->sndDataprepare->toPlainText();
        len = m_dataQStrSnd.length();
        ba = m_dataQStrSnd.toLatin1();
        m_dataSnd = ba.data();
        break;
    case 1:
        break;
    default:
        break;
    }

//    connect(Ts, &TCP::TcpServer::sigRspToClientFinish, [&data](){
//        delete data;
//        data = nullptr;
//    });
    emit sigSend(m_sndOrHex, m_dataSnd, len);
}

//void MainWindow::onRspToClientFinish(char *data) {
//    delete data;
//    data = nullptr;
//}
