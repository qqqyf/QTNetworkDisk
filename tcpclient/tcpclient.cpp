#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "protocol.h"


#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <cstring>
#include <QString>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadconfig();

    connect(m_tcpSocket,&QTcpSocket::connected,[=](){
        QMessageBox::information(this,"tip","connect to server successfully");
    });//当客户端与服务器连接成功弹出提示

    //客户端收到数据发出readyread信号，调用recvMsg接收
    connect(m_tcpSocket,&QTcpSocket::readyRead,this,&TcpClient::recvMsg);

    m_tcpSocket->connectToHost(QHostAddress(m_strip),m_usport,QIODevice::ReadWrite);

}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadconfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QString strdata = data.toStdString().c_str();
        // qDebug() << strdata;
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

TcpClient& TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket *&TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getMyName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurpath)
{
    m_strCurPath = strCurpath;
}



/*
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();//获取文本框输入字符串
    if(!strMsg.isEmpty())
    {
        PDU* pdu = mkPDU(strMsg.size());//分配PDU内存空间
        pdu->MsgType = 8888;
        std::string s = strMsg.toStdString();
        const char* c = s.c_str();
        memcpy(pdu->caMsg, c, strMsg.size());
        m_tcpSocket->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::information(this,"提示","发送的信息不能为空!");
    }
}
*/

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_lineEdit->text();
    m_strLoginName = strName;
    QString strPwd = ui->pwd_lineEdit->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty()){
        std::string name = strName.toStdString();
        std::string pwd = strPwd.toStdString();
        PDU *login = mkPDU(0);
        login->MsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(login->caData,name.c_str(),32);
        strncpy(login->caData+32,pwd.c_str(),32);
        m_tcpSocket->write((char*)login,login->PDULen);
    }
    else {
        QMessageBox::information(NULL,"提示","账号或密码为空");
    }

}


void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_lineEdit->text();
    QString strPwd = ui->pwd_lineEdit->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty()){
        PDU* pdu = mkPDU(0);
        pdu->MsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        std::string name = strName.toStdString();
        std::string pwd = strPwd.toStdString();
        strncpy(pdu->caData,name.c_str(),32);
        strncpy(pdu->caData+32,pwd.c_str(),32);
        m_tcpSocket->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
    else{
        QMessageBox::information(this,"提示","注册失败：用户名或密码为空!");
    }
}


void TcpClient::on_delete_pb_clicked()
{

}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getDownLoadStatus()){
        uint PDULen=0;
        //读取数据长度
        m_tcpSocket->read((char*)&PDULen,sizeof(uint));
        uint MsgLen = PDULen - sizeof(PDU);
        PDU *res = mkPDU(MsgLen);
        m_tcpSocket->read((char*)res+sizeof(uint),PDULen - sizeof(uint));
        switch(res->MsgType){
        case ENUM_MSG_TYPE_REGIST_RESPOND:{
            if(strcmp(res->caData,REGIST_OK)==0){
                qDebug()<<res->caData;
                QMessageBox::information(NULL,"提示",REGIST_OK);
            }
            else if(strcmp(res->caData,REGIST_FAILED)==0){
                QMessageBox::information(NULL,"提示",REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:{
            if(strcmp(res->caData,LOGIN_OK)==0){
                m_strCurPath = QString("./%1").arg(m_strLoginName);
                //登录时记录用户文件的当前路径
                QMessageBox::information(this,"提示",LOGIN_OK);
                OpeWidget::getInstance().show();
                this->hide();
            }
            else if(strcmp(res->caData,LOGIN_FAILED)==0){
                QMessageBox::information(this,"提示",LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:{
            //操作界面-》好友界面-》更新在线好友列表
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(res);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:{
            if(strcmp(res->caData,SEARCH_USR_NO)==0){
                QMessageBox::information(NULL,"查询结果",QString("%1 not exit").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if(strcmp(res->caData,SEARCH_USR_ONLINE)==0){
                QMessageBox::information(NULL,"查询结果",QString("%1 is online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if(strcmp(res->caData,SEARCH_USR_OFFLINE)==0){
                QMessageBox::information(NULL,"查询结果",QString("%1 is offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            break;
        }

        case ENUM_MSG_TYPE_ADDFRIEND_REQUEST:{
            char caName[32] = {'\0'};
            strncpy(caName,res->caData+32,32);
            int ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend?").arg(caName),QMessageBox::Yes,QMessageBox::No);
            PDU *addf_res = mkPDU(0);
            memcpy(addf_res->caData,res->caData,32);       //拷贝UsrName
            memcpy(addf_res->caData+32,res->caData+32,32); //拷贝MyName
            if(ret == QMessageBox::Yes){
                addf_res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_AGREE;
            }
            else{
                addf_res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_REFUSE;
            }
            m_tcpSocket->write((char*)addf_res,addf_res->PDULen);
            free(addf_res);
            addf_res=NULL;
            break;
        }

        case ENUM_MSG_TYPE_ADDFRIEND_AGREE:{
            char UsrName[32]={'\0'};
            strncpy(UsrName,res->caData,32);
            QMessageBox::information(this,"好友添加结果",QString("%1 接受了你的好友申请").arg(UsrName));
            break;
        }
        case ENUM_MSG_TYPE_ADDFRIEND_REFUSE:{
            char UsrName[32]={'\0'};
            strncpy(UsrName,res->caData,32);
            QMessageBox::information(this,"好友添加结果",QString("%1 拒绝了你的好友申请").arg(UsrName));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:{
            OpeWidget::getInstance().getFriend()->updateFriendList(res);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FIREND_REQUEST:{
            char MyName[32]={'\0'};
            strncpy(MyName,res->caData,32);
            QMessageBox::information(this,"提示",QString("%1 将你移出好友列表").arg(MyName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FIREND_RESPOND:{
            QMessageBox::information(this,"提示","删除好友成功");
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:{
            if(PrivateChat::getInstance().isHidden()){
                PrivateChat::getInstance().show();
            }
            char sender[32]={'\0'};
            memcpy(sender,res->caData,32);
            PrivateChat::getInstance().setChatName(sender);
            PrivateChat::getInstance().updateMsg(res);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:{
            OpeWidget::getInstance().getFriend()->updateGroupMsg(res);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:{
            QMessageBox::information(this,"创建文件夹",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:{
            OpeWidget::getInstance().getBook()->updateFileList(res);
            QString strEnterDir = OpeWidget::getInstance().getBook()->enterDirName();
            if(!strEnterDir.isEmpty()){
                m_strCurPath = m_strCurPath+"/"+strEnterDir;
                //进入文件名非空，更新当前所在路径
                // qDebug()<<m_strCurPath;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:{
            QMessageBox::information(this,"删除文件夹",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:{
            QMessageBox::information(this,"重命名文件",res->caData);

            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:{
            OpeWidget::getInstance().getBook()->clearEnterDir();
            QMessageBox::information(this,"进入文件夹",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:{
            QMessageBox::information(this,"上传文件",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:{
            QMessageBox::information(this,"删除文件",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:{
            char caFileName[32] = {'\0'};
            sscanf(res->caData,"%s %lld",caFileName,&(OpeWidget::getInstance().getBook()->m_iTotal));
            if(strlen(caFileName)>0 && OpeWidget::getInstance().getBook()->m_iTotal>0){
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly)){
                    QMessageBox::warning(this,"下载文件","获得保存文件路径失败");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:{
            QMessageBox::information(this,"共享文件",res->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:{
            qDebug() << "分享文件通知已接收";
            char *pPath = new char[res->MsgLen];
            memcpy(pPath,res->caMsg,res->MsgLen);
            char *pos = strrchr(pPath, '/');//从后往前找一个字符
            qDebug()<<QString("test     %1").arg(pos);
            if(pos!=NULL){
                pos++;
                QString strNote = QString("%1 share file -> %2 \n Do you accept?").arg(res->caData).arg(pos);
                int ret = QMessageBox::question(NULL,"共享文件",strNote);
                if(QMessageBox::Yes == ret){
                    PDU *respdu = mkPDU(res->MsgLen);
                    respdu->MsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg,res->caMsg,res->MsgLen);
                    QString strName = TcpClient::getInstance().getMyName();
                    strcpy(respdu->caData,strName.toStdString().c_str());
                    m_tcpSocket->write((char*)respdu,respdu->PDULen);
                    free(respdu);
                    respdu=NULL;
                    qDebug() << "分享文件通知回复已发送";
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:{
            QMessageBox::information(this,"移动文件",res->caData);
            break;
        }
        default:
            break;
        }
        free(res);
        res=NULL;
    }
    else {
        QByteArray buffer = m_tcpSocket->readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if(pBook->m_iTotal == pBook->m_iRecved){
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this,"下载文件","下载文件成功");
        }
        else if(pBook->m_iRecved > pBook->m_iTotal){
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this,"下载文件","下载文件失败");
        }
    }
}

