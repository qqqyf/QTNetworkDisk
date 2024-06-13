#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include "mytcpsocket.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT//一个类使用信号和槽的两个条件：继承QObject基类；有Q_OBJECT宏
public:
    MyTcpServer();
    static MyTcpServer& getInstance();//单例模式
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char* UsrName,PDU *pdu);

public slots:
    void deleteSocket(MyTcpSocket *socket);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
