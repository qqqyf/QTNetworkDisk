#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include "privatechat.h"

#include <QInputDialog>
#include <QString>
#include <QDebug>
#include <QMessageBox>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("发送信息");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    QHBoxLayout *pOnlHBL = new QHBoxLayout;
    pOnlHBL->addWidget(m_pOnline);

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addLayout(pOnlHBL);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlineUsrPB,&QPushButton::clicked,this,&Friend::showOnline);
    connect(m_pSearchUsrPB,&QPushButton::clicked,this,&Friend::searchUsr);
    connect(m_pFlushFriendPB,&QPushButton::clicked,this,&Friend::flushFriend);
    connect(m_pDelFriendPB,&QPushButton::clicked,this,&Friend::deleteFriend);
    connect(m_pPrivateChatPB,&QPushButton::clicked,this,&Friend::privateChat);
    connect(m_pMsgSendPB,&QPushButton::clicked,this,&Friend::groupChat);
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(pdu==NULL){
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::searchUsr()
{
    QString name = QInputDialog::getText(this,"搜索","用户名");
    m_strSearchName = name;
    if(!name.isEmpty()){
        // qDebug()<<name;
        PDU* pdu = mkPDU(0);
        pdu->MsgType=ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        memcpy(pdu->caData,name.toStdString().c_str(),name.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Friend::flushFriend()
{
    QString MyName = TcpClient::getInstance().getMyName();
    PDU *pdu = mkPDU(0);
    pdu->MsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,MyName.toStdString().c_str(),32);
    TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
    free(pdu);
    pdu=NULL;
}

void Friend::updateFriendList(PDU *pdu)
{
    if(pdu==NULL){
        return;
    }
    m_pFriendListWidget->clear();
    uint num = pdu->MsgLen/32;
    char caName[32]={'\0'};
    for(uint i=0;i<num;i++){
        memcpy(caName,(char*)(pdu->caData)+i*32,32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::deleteFriend()
{
    if(m_pFriendListWidget->currentItem()!=NULL){
        QString strName = m_pFriendListWidget->currentItem()->text();
        PDU *pdu = mkPDU(0);
        pdu->MsgType=ENUM_MSG_TYPE_DELETE_FIREND_REQUEST;
        memcpy(pdu->caData+32,strName.toStdString().c_str(),32);
        memcpy(pdu->caData,TcpClient::getInstance().getMyName().toStdString().c_str(),32);
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Friend::privateChat()
{
    if(m_pFriendListWidget->currentItem()!=NULL){
        QString strChatName = m_pFriendListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(strChatName);
        if(PrivateChat::getInstance().isHidden()){
            PrivateChat::getInstance().show();
        }
    }
    else{
        QMessageBox::information(this,"提示","请选择私聊对象");
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if(!strMsg.isEmpty()){
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->MsgType=ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        strncpy(pdu->caData,TcpClient::getInstance().getMyName().toStdString().c_str(),32);
        strncpy((char*)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        m_pInputMsgLE->clear();
        m_pShowMsgTE->append(QString("%1 :%2").arg(TcpClient::getInstance().getMyName()).arg(strMsg));
    }
    else{
        QMessageBox::information(this,"提示","信息不能为空");
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg = QString("%1 :%2").arg(pdu->caData).arg(pdu->caMsg);
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden()){
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->MsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
    else{
        m_pOnline->hide();
    }
}
