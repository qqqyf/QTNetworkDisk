#include "online.h"
#include "ui_online.h"
#include "protocol.h"
#include <QDebug>
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(pdu == NULL){
        return;
    }
    uint uiSize = pdu->MsgLen/32;
    char caTmp[32];
    ui->listWidget->clear();
    for (uint i=0;i<uiSize;i++){
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        ui->listWidget->addItem(caTmp);
    }
}

void Online::on_adFriend_pb_clicked()
{
    QListWidgetItem *pItem = ui->listWidget->currentItem();
    if(pItem == NULL){
        return;
    }
    else{
        QString UsrName = pItem->text();
        // qDebug()<<UsrName;
        QString MyName = TcpClient::getInstance().getMyName();
        PDU* pdu = mkPDU(0);
        pdu->MsgType = ENUM_MSG_TYPE_ADDFRIEND_REQUEST;
        memcpy(pdu->caData,UsrName.toStdString().c_str(),UsrName.size());
        memcpy(pdu->caData+32,MyName.toStdString().c_str(),MyName.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
}

