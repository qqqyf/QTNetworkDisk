#include "privatechat.h"
#include "ui_privatechat.h"

#include <QMessageBox>

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().getMyName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(pdu==NULL){
        return;
    }
    char sender[32] = {'\0'};
    memcpy(sender,pdu->caData,32);
    QString strMsg = QString("%1 :%2").arg(sender).arg(pdu->caMsg);
    ui->shwoMsg_te->append(strMsg);
}


void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    if(!strMsg.isEmpty()){
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->MsgType=ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        QString showMsg = QString("%1 :%2").arg(m_strLoginName.toStdString().c_str()).arg(strMsg.toStdString().c_str());
        ui->shwoMsg_te->append(showMsg);
        ui->inputMsg_le->clear();
        free(pdu);
        pdu=NULL;
    }
    else{
        QMessageBox::information(this,"提示","发送的聊天信息不能为空");
    }
}

