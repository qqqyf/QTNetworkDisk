#include "mytcpserver.h"
#include "mytcpsocket.h"

#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    // 局部静态特性的方式实现单实例。
    // 静态局部变量只在当前函数内有效，其他函数无法访问。
    // 静态局部变量只在第一次被调用的时候初始化，也存储在静态存储区，生命周期从第一次被初始化起至程序结束止。
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    // qDebug()<<"new client connected";
    //有客户端连接到服务器，实例化一个MyTcpSocket对象进行数据收发
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    //将MyTcpSocket对象添加到Socket列表
    //监听列表中对象是否发出offline信号，发出则删除对应Socket对象
    m_tcpSocketList.append(pTcpSocket);
    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deleteSocket(MyTcpSocket*)));
}

void MyTcpServer::resend(const char *UsrName, PDU *pdu)
{
    if(UsrName == NULL || pdu == NULL){
        return;
    }
    QString name = UsrName;
    for(int i=0;i<m_tcpSocketList.size();i++){
        if(m_tcpSocketList.at(i)->getName() == name){
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->PDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *socket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();iter++){
        if(*iter == socket){
            delete *iter;
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    for(int i=0;i<m_tcpSocketList.size();i++){
        qDebug()<<m_tcpSocketList.at(i)->getName();
    }
}
