#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>

#include "opewidget.h"
#include "privatechat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadconfig();
    static TcpClient& getInstance();
    QTcpSocket* &getTcpSocket();
    QString getMyName();
    QString curPath();
    void setCurPath(QString strCurpath);


private slots:
    // void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_delete_pb_clicked();

    void recvMsg();


private:
    Ui::TcpClient *ui;
    QString m_strip;
    quint16 m_usport;

    QTcpSocket* m_tcpSocket = new QTcpSocket(this);//socket对象连接服务器，和服务器数据交互
    QString m_strLoginName;
    QString m_strCurPath;

    QFile m_file;
};
#endif // TCPCLIENT_H
