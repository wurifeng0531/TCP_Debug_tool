#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    Ts(new TCP::TcpServer()),Tc(new TCP::TcpClient) {
    m_dataSnd = nullptr;
    m_sndOrHex = 0;
    m_rcvtextOrHex = 0;
    m_sndOrHex_client = 0;
    m_rcvDataOrFile_client = 0;
    m_rcvDataOrFile = false;
    ui->setupUi(this);

    connect(this, &MainWindow::sigStartListening, Ts, &TCP::TcpServer::onStartListening);
    connect(this, &MainWindow::sigStopListening, Ts, &TCP::TcpServer::onStopListening);
    connect(this, &MainWindow::sigSelectAConToRsp, Ts, &TCP::TcpServer::onGetRspClient);
    connect(this, &MainWindow::sigDisConn, Ts, &TCP::TcpServer::onDisConn);
    connect(this, &MainWindow::sigSendDataToClient, Ts, &TCP::TcpServer::onSendDataToClient);
    connect(this, &MainWindow::sigChangeRcvDataType, Ts, &TCP::TcpServer::sigChangeRcvDataType);

    connect(this, &MainWindow::sigConnectToExServer, Tc, &TCP::TcpClient::onConnectToExServer);
    connect(this, &MainWindow::sigDisConnFromExServer, Tc, &TCP::TcpClient::sigSockDisconnect);
    connect(this, &MainWindow::sigSendDataToExServer, Tc, &TCP::TcpClient::onSendDataToExServer);

    connect(Ts, &TCP::TcpServer::sigShowIpAndPorts, this, &MainWindow::onShowIpAndPort);
    connect(Ts, &TCP::TcpServer::sigGetDataForServer, this, &MainWindow::onGetDataForServer);

    connect(Tc, &TCP::TcpClient::sigShowExServer, this, &MainWindow::onShowExServer);
    connect(Tc, &TCP::TcpClient::sigGetDataFromExServer, this, &MainWindow::onGetDataFromExServer);

    if(ui->maxConNum->value() == 0){
        ui->listeningTrig->hide();
    }
}

MainWindow::~MainWindow() {
    delete m_dataSnd;
    m_dataSnd = nullptr;
    delete ui;
    delete Ts;
}

void MainWindow::on_listenPort_editingFinished() {
    Ts->m_port = ui->listenPort->text().toInt();
}

void MainWindow::on_listeningTrig_clicked() {
//    if( (ui->listeningTrig->text() == tr("侦听")) && (ui->listenPort->text().toInt() > 0) ) {
    if(ui->listeningTrig->text() == tr("侦听")) {
        ui->listeningTrig->setText(tr("停止"));
        Ts->m_startListen = true;
        emit sigStartListening();
    } else if(ui->listeningTrig->text() == tr("停止")) {
        ui->listeningTrig->setText(tr("侦听"));
        Ts->m_startListen = false;
        emit sigStopListening();
    }
}

void MainWindow::on_maxConNum_valueChanged(int Num) {
    if(Num > 0) {
        Ts->m_MaxConnNum = (size_t)Num;
        if(ui->listeningTrig->isHidden()) {
            ui->listeningTrig->show();
        }
    } else {
        ui->listeningTrig->hide();
    }

}

void MainWindow::onShowIpAndPort(vector<QString> ipAndPortStr)
{
    //TODO:可以考虑将ipv6地址删掉 留下ipv4
    ui->ipAndPortsDisplay->clear();
    for(size_t i = 0; i < ipAndPortStr.size(); i++) {
        if(!ipAndPortStr[i].isEmpty()
        && (ui->ipAndPortsDisplay->findText(ipAndPortStr[i]) == -1)) {
            ui->ipAndPortsDisplay->insertItem(i, ipAndPortStr[i]);
        }
    }
}

void MainWindow::on_ipAndPortsDisplay_currentIndexChanged(const QString &selectConn) {
    QStringList ipAndPort = selectConn.split(':');
    int len = ipAndPort.length();
    if (len >= 2) {
        emit sigSelectAConToRsp(ipAndPort[len-2], ipAndPort[len-1]);
    }
}

void MainWindow::on_refuseConnTrig_clicked() {
    emit sigDisConn();
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
    QByteArray rawData;
    QByteArray tmpData;
    QStringList ql;
    int i = 0;
    bool ok;
    m_dataQStrSnd = ui->sndDataprepare->toPlainText();
    switch (m_sndOrHex) {
    case 0://按文本格式发送
        tmpData = m_dataQStrSnd.toLocal8Bit();
        rawData.append(QTextCodec::codecForName("GB2312")->toUnicode(tmpData));
        break;
    case 1://按十六进制发送 在输入框中 十六进制数以空格分隔
        ql = m_dataQStrSnd.split(' ');
        for(i = 0; i< ql.length(); i++) {
            rawData.append(ql[i].toInt(&ok, 16));
        }
        break;
    default:
        break;
    }
    m_dataSnd = rawData.data();
    emit sigSendDataToClient(m_dataSnd, rawData.length());
}

void MainWindow::on_clearRcvDataTrig_clicked()
{
    ui->rcvDataDisplay->clear();
}

void MainWindow::onGetDataForServer(TCP::TcpSocket *tsp, const QByteArray &dataForServer) {
    QString sockInfoAndData;
    sockInfoAndData = "IP:" + tsp->peerAddress().toString() +
            "\nPort:" + QString::number(tsp->peerPort(), 10) +
            "\nsockId:" + QString::number(tsp->m_sockId, 10) + "\nrecvData:";   
    switch(m_rcvtextOrHex) {
    case 0://按文本格式接收
//        sockInfoAndData += QTextCodec::codecForName("GB2312")->fromUnicode(dataForServer)
//                + "\nlength:" + QString::number(dataForServer.length(), 10) + "\n";
        sockInfoAndData += dataForServer + "\nlength:" +
                QString::number(dataForServer.length(), 10) + "\n";
        ui->rcvDataDisplay->append(sockInfoAndData);
        break;
    case 1://按十六进制接收 接到数据每个字节间添加空格再显示
        for(int i = 0; i < dataForServer.length(); i++) {
            sockInfoAndData += QString::number((quint8)dataForServer[i], 16) + ' ';
        }
        sockInfoAndData += "\nlength:" + QString::number(dataForServer.length(), 10) + "\n";
        ui->rcvDataDisplay->append(sockInfoAndData);
        break;
    default:
        break;
    }
}

//与服务器的处理方法相同，若合并成一个槽函数，用不同信号来连接，并发可能有问题
void MainWindow::onGetDataFromExServer(TCP::TcpSocket *tsp, const QByteArray &data)
{
    QString sockInfoAndData;
    sockInfoAndData = "IP:" + tsp->peerAddress().toString() +
            "\nPort:" + QString::number(tsp->peerPort(), 10) + "\nrecvData:";
    switch(m_rcvtextOrHex_client) {
    case 0://按文本格式接收
//        sockInfoAndData += QTextCodec::codecForName("GB2312")->toUnicode(data)
//                + "\nlength:" + QString::number(data.length(), 10) + "\n";
        sockInfoAndData += data + "\nlength:" + QString::number(data.length(), 10) + "\n";
        ui->rcvDataDisplay_client->append(sockInfoAndData);
        break;
    case 1://按十六进制接收 接到数据每个字节间添加空格再显示
        for(int i = 0; i < data.length(); i++) {
            sockInfoAndData += QString::number((quint8)data[i], 16) + ' ';
        }
        sockInfoAndData += "\nlength:" + QString::number(data.length(), 10) + "\n";
        ui->rcvDataDisplay_client->append(sockInfoAndData);
        break;
    default:
        break;
    }
}


void MainWindow::on_rcvFileSelect_clicked(bool checked)
{
    QString fileName;
    QFileInfo fileInfo;//不行，获取不到对端文件的信息
    if(checked == true) {
        fileName = ui->rcvFileName->text();
        ui->rcvFileName->setDisabled(false);
        emit sigChangeRcvDataType(true, fileName);
        ui->rcvTextSelect->setVisible(false);
        ui->rcvHexSelect->setVisible(false);
        ui->clearRcvDataTrig->setVisible(false);
    } else {
        ui->rcvFileName->setDisabled(true);
        emit sigChangeRcvDataType(false);
        ui->rcvTextSelect->setVisible(true);
        ui->rcvHexSelect->setVisible(true);
        ui->clearRcvDataTrig->setVisible(true);
    }
}

void MainWindow::on_clearSndDataTrig_clicked() {
    ui->sndDataprepare->clear();
}

void MainWindow::on_connectToExternalServerTrig_clicked()
{
    if(ui->connectToExternalServerTrig->text() == tr("连接")) {
        QString pattern("^(?:(?:2[0-4][0-9]\\.)|(?:25[0-5]\\.)"
                        "|(?:1[0-9][0-9]\\.)|(?:[1-9][0-9]\\.)"
                        "|(?:[0-9]\\.)){3}(?:(?:2[0-5][0-5])"
                        "|(?:25[0-5])|(?:1[0-9][0-9])|(?:[1-9][0-9])"
                        "|(?:[0-9]))$");
        QRegExp regEp(pattern);
        bool match = regEp.exactMatch(ui->externalServerIpAddr->text());
        if(match) {
            if(ui->externalServerPort->text().toInt() > 0
                && ui->externalServerPort->text().toInt() <65536) {
                emit sigConnectToExServer(ui->externalServerIpAddr->text(),
                        ui->externalServerPort->text().toInt());
                //这里有个bug：若对端服务器侦听没开始，也会显示“断开”？用户体验不好。
                ui->connectToExternalServerTrig->setText(tr("断开"));
            } else {
                ui->externalServerPort->clear();
            }
        } else {
            ui->externalServerIpAddr->clear();
        }
    } else if(ui->connectToExternalServerTrig->text() == tr("断开")) {
        ui->connectToExternalServerTrig->setText(tr("连接"));
        emit sigDisConnFromExServer();
        ui->externalServerInfo->clear();
    }
}

void MainWindow::onShowExServer(const QString &ipAndPort)
{
    ui->externalServerInfo->clear();
    ui->externalServerInfo->setText(ipAndPort);
    if((ui->connectToExternalServerTrig->text() == tr("断开"))
        && (ui->externalServerInfo->text() == "")) {
        ui->connectToExternalServerTrig->setText(tr("连接"));
    }
//    else if( (ui->connectToExternalServerTrig->text() == tr("连接"))
//        && (ui->externalServerInfo->text().contains(ui->externalServerIpAddr->text()))
//        && (ui->externalServerInfo->text().contains(ui->externalServerPort->text())) ) {
//        ui->connectToExternalServerTrig->setText(tr("断开"));
//    }
}

void MainWindow::on_rcvTextSelect_client_clicked()
{
    m_rcvtextOrHex_client = 0;
}

void MainWindow::on_rcvHexSelect_client_clicked()
{
    m_rcvtextOrHex_client = 1;
}

void MainWindow::on_sndTextSelect_client_clicked()
{
    m_sndOrHex_client = 0;
}

void MainWindow::on_sndHexSelect_client_clicked()
{
    m_sndOrHex_client = 1;
}

void MainWindow::on_clearRcvDataTrig_client_clicked()
{
    ui->rcvDataDisplay_client->clear();
}

void MainWindow::on_sndDataTrig_client_clicked()
{
    QByteArray rawData;
    QByteArray tmpData;
    QStringList ql;
    int i = 0;
    bool ok;
    m_dataQStrSnd_client = ui->sndDataprepare_client->toPlainText();
    switch (m_sndOrHex_client) {
    case 0://按文本格式发送
        tmpData = m_dataQStrSnd_client.toLocal8Bit();
        rawData.append(QTextCodec::codecForName("GB2312")->toUnicode(tmpData));
        break;
    case 1://按十六进制发送 在输入框中 十六进制数以空格分隔
        ql = m_dataQStrSnd_client.split(' ');
        for(i = 0; i< ql.length(); i++) {
            rawData.append(ql[i].toInt(&ok, 16));
        }
        break;
    default:
        break;
    }
    m_dataSnd_client = rawData.data();
    emit sigSendDataToExServer(m_dataSnd_client, rawData.length());
}

void MainWindow::on_clearSndDataTrig_client_clicked()
{
    ui->sndDataprepare_client->clear();
}
