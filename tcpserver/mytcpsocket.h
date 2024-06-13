#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include "protocol.h"
#include "mytcpsocket.h"

#include <QTcpSocket>
#include <QFile>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);



    QString getName();
    void copyDir(QString strSrcDir,QString strDestDir);


signals:
    void offline(MyTcpSocket *socket);

public slots:
    void recvMsg();
    void clientOffline();
    void sendFileToClient();

private:
    QString m_strName;
    QFile m_file;//文件操作对象
    qint64 m_iTotal;//文件总大小
    qint64 m_iRecved;//已接收大小
    bool m_bUpload;//是否处于上传状态

    QTimer *m_pTimer;
};

#endif // MYTCPSOCKET_H
