#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    Ts(new TCP::TcpServer())
{
    ui->setupUi(this);
    connect(Ts, &TCP::TcpServer::showIpAndPorts, this, &MainWindow::onShowIpAndPort);
    connect(this, &MainWindow::selectAConToRsp, Ts, &TCP::TcpServer::onGetRspClient);
    connect(this, &MainWindow::refuseConn, Ts, &TCP::TcpServer::onRefuseConn);
    Ts->start();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete Ts;
}

//void MainWindow::on_IpAddr_editingFinished()
//{
//    Ts->m_ipAddr = ui->IpAddr->text();
//}

void MainWindow::on_listenPort_editingFinished()
{
    Ts->m_port = ui->listenPort->text().toInt();
}

void MainWindow::on_listeningTrig_clicked()
{
    if(ui->listeningTrig->text() == tr("侦听")) {
        ui->listeningTrig->setText(tr("停止"));
        Ts->m_startListen = true;
    }
    else if(ui->listeningTrig->text() == tr("停止")) {
        ui->listeningTrig->setText(tr("侦听"));
        Ts->m_startListen = false;
    }

}

void MainWindow::on_maxConNum_valueChanged(int Num)
{
    Ts->m_MaxConnNum = (size_t)Num;
}

void MainWindow::onShowIpAndPort(QString ipAndPortStr) {
//    ui->ipAndPortsDisplay0->setText(ipAndPortStr);
    QStringList clientList = ipAndPortStr.split("\n");
    ui->ipAndPortsDisplay->clear();
    for(int i = 0; i < clientList.length(); i++) {
        ui->ipAndPortsDisplay->insertItem(i, clientList[i]);
    }
}

void MainWindow::on_ipAndPortsDisplay_currentIndexChanged(const QString &selectConn)
{
    ui->ipAndPortsDisplay0->setText(selectConn);
    emit selectAConToRsp(selectConn);
}

void MainWindow::on_refuseConnTrig_clicked()
{
    emit refuseConn();
}
