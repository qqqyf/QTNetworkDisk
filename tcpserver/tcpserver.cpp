#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"

#include <QFile>
#include <QMessageBox>
#include <QByteArray>
#include <QDebug>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();//加载ip，port
    //监听客户端
    MyTcpServer::getInstance().listen(QHostAddress(m_strip),m_usport);

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QString strdata = data.toStdString().c_str();
        file.close();
        strdata.replace("\r\n"," ");
        this->m_strip = strdata.mid(0,strdata.indexOf(" "));
        this->m_usport = strdata.mid(strdata.indexOf(" ")+1,-1).toUShort();
        // qDebug()<<this->m_strip;
        // qDebug()<<this->m_usport;

    }
    else
    {
        QMessageBox::information(this,"tip","fail to open config file.");
    }
}
