#include "sharefile.h"
#include <QAbstractButton>
#include <QList>
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    connect(m_pCancelSelectPB,&QPushButton::clicked,this,&ShareFile::canaelSelect);
    connect(m_pSelectAllPB,&QPushButton::clicked,this,&ShareFile::selectAll);
    connect(m_pOKPB,&QPushButton::clicked,this,&ShareFile::okShare);
    connect(m_pCancelPB,&QPushButton::clicked,this,&ShareFile::cancelShare);
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL == pFriendList){
        return;
    }
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    QAbstractButton *temp = NULL;
    for(int i=0;i<preFriendList.size();i++){
        temp = preFriendList[i];
        m_pFriendWVBL->removeWidget(preFriendList[i]);
        m_pButtonGroup->removeButton(preFriendList[i]);
        preFriendList.removeOne(temp);
        delete temp;
        temp = NULL;
    }
    QCheckBox *pCB = NULL;
    for(int i = 0;i<pFriendList->count();i++){
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::canaelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0;i<cbList.size();i++){
        cbList[i]->setChecked(true);
    }
}

void ShareFile::okShare()
{
    QString strName = TcpClient::getInstance().getMyName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();

    QString strPath = strCurPath+"/"+strShareFileName;
    qDebug()<<"strCurPath:"<<strCurPath
             <<" strShareFileName:"<<strShareFileName
             <<" strPath:"<<strPath;

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num = 0;
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            num++;
        }
    }

    PDU *pdu = mkPDU(num*32+strPath.size()+1);
    pdu->MsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);
    //写入分享者姓名和总分享人数
    qDebug()<<"分享者姓名："<<strName<<" 待分享人数："<<num;
    qDebug()<<pdu->caData;
    int j = 0;
    for(int i = 0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());
            j++;
        }
    }//写入被分享者姓名
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.size());
    //写入分享文件路径
    qDebug()<<(pdu->caMsg)+num*32;
    TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
    qDebug() << "分享文件请求已发送";
}

void ShareFile::cancelShare()
{
    hide();
}
