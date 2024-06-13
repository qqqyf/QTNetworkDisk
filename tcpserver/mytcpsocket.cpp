#include "mytcpsocket.h"
#include "dboperator.h"
#include "mytcpserver.h"

#include <QDir>
#include <QDebug>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    //将socket的readyread信号封装到MyTcpSocket里
    //避免不同客户端的socket互相干扰
    connect(this,&MyTcpSocket::readyRead,this,&MyTcpSocket::recvMsg);
    //客户端与服务器断开socket发出disconnect信号
    connect(this,&MyTcpSocket::disconnected,this,&MyTcpSocket::clientOffline);
    m_bUpload = false;

    m_pTimer = new QTimer;
    connect(m_pTimer,&QTimer::timeout,this,&MyTcpSocket::sendFileToClient);
}

QString MyTcpSocket::getName()
{
    return this->m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);//产生目标目录

    dir.setPath(strSrcDir);//设置源目录
    QFileInfoList fileInfoList = dir.entryInfoList();
    //获得目录信息，产生fileinfolist对象接收

    QString srcTmp;
    QString destTmp;
    for(int i=0;i<fileInfoList.size();i++)
    {
        if(fileInfoList[i].isFile())
        {
            fileInfoList[i].fileName();
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            if(fileInfoList[i].fileName() == QString(".")||
                fileInfoList[i].fileName() == QString(".."))
            {
                continue;
            }
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            copyDir(srcTmp,destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
    {
        // qDebug()<<this->bytesAvailable();
        uint PDULen = 0;
        this->read((char*)&PDULen,sizeof(uint));//读取4个字节并赋值到PDULen
        uint MsgLen = PDULen - sizeof(PDU);
        PDU* pdu = mkPDU(MsgLen);//分配PDU存储收到的数据
        //计算起始偏移量，把后续数据存到pdu
        this->read((char*)pdu+sizeof(uint),PDULen-sizeof(uint));
        uint l = 0;
        switch(pdu->MsgType){
            case ENUM_MSG_TYPE_REGIST_REQUEST:{
                PDU *res = mkPDU(l);//回复请求
                res->MsgType=ENUM_MSG_TYPE_REGIST_RESPOND;
                char caName[32]={'\0'};
                char caPwd[32]={'\0'};//接收客户端的注册信息
                strncpy(caName,pdu->caData,32);
                strncpy(caPwd,pdu->caData+32,32);
                if(DBOperator::getInstance().handleRegist(caName,caPwd)){
                    strcpy(res->caData,REGIST_OK);
                    QDir dir;
                    dir.mkdir(QString("./%1").arg(caName));
                }
                else {
                    strcpy(res->caData,REGIST_FAILED);
                }
                this->write((char*)res,res->PDULen);
                // qDebug()<<res->caData;
                free(res);
                res=NULL;
                break;
            }
            case ENUM_MSG_TYPE_LOGIN_REQUEST:{
                PDU *res = mkPDU(l);//回复请求
                res->MsgType=ENUM_MSG_TYPE_LOGIN_RESPOND;
                char caName[32]={'\0'};
                char caPwd[32]={'\0'};
                strncpy(caName,pdu->caData,32);
                strncpy(caPwd,pdu->caData+32,32);
                if(DBOperator::getInstance().handleLogin(caName,caPwd)){
                    strcpy(res->caData,LOGIN_OK);
                    m_strName = caName;
                }
                else{
                    strcpy(res->caData,LOGIN_FAILED);
                }
                this->write((char*)res,res->PDULen);
                // qDebug()<<res->caData;
                free(res);
                res=NULL;
                break;
            }
            case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:{
                QStringList ret = DBOperator::getInstance().handleAllOnline();
                l = ret.size()*32;
                PDU *respdu = mkPDU(l);
                respdu->MsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
                for(int i=0;i<ret.size();i++){
                    std::string name = ret.at(i).toStdString();
                    memcpy((char*)(respdu->caMsg)+i*32,name.c_str(),sizeof(name));
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:{
                int ret = DBOperator::getInstance().handleSearchUsr(pdu->caData);
                // qDebug()<<ret;
                PDU *res = mkPDU(0);
                res->MsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
                if(ret == -1){
                    strcpy(res->caData,SEARCH_USR_NO);
                }
                else if(ret == 1){
                    strcpy(res->caData,SEARCH_USR_ONLINE);
                }
                else if(ret == 0){
                    strcpy(res->caData,SEARCH_USR_OFFLINE);
                }
                write((char*)res,res->PDULen);
                free(res);
                res = NULL;
                break;
            }
            case ENUM_MSG_TYPE_ADDFRIEND_REQUEST:{
                char UsrName[32]={'\0'};
                char MyName[32]={'\0'};
                strncpy(UsrName,pdu->caData,32);
                strncpy(MyName,pdu->caData+32,32);
                int ret = DBOperator::getInstance().handleAddFriend(UsrName,MyName);
                PDU *res = NULL;
                if(ret == -1){
                    res = mkPDU(0);
                    res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_RESPOND;
                    strcpy(res->caData,UNKNOW_ERRORE);
                    write((char*)res,res->PDULen);
                    free(res);
                    res = NULL;
                }
                else if(ret == 0){
                    res = mkPDU(0);
                    res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_RESPOND;
                    strcpy(res->caData,EXITED_FRIEND);
                    write((char*)res,res->PDULen);
                    free(res);
                    res = NULL;
                }
                else if(ret == 1){
                    MyTcpServer::getInstance().resend(UsrName,pdu);
                }
                else if(ret == 2){
                    res = mkPDU(0);
                    res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_RESPOND;
                    strcpy(res->caData,FRIEND_OFFLINE);
                    write((char*)res,res->PDULen);
                    free(res);
                    res = NULL;
                }
                else if(ret == 3){
                    res = mkPDU(0);
                    res->MsgType = ENUM_MSG_TYPE_ADDFRIEND_RESPOND;
                    strcpy(res->caData,FRIEND_NO_EXITED);
                    write((char*)res,res->PDULen);
                    free(res);
                    res = NULL;
                }
                break;
            }
            case ENUM_MSG_TYPE_ADDFRIEND_AGREE:{
                char UsrName[32]={'\0'};
                char MyName[32]={'\0'};
                strncpy(UsrName,pdu->caData,32);//获取UsrName
                strncpy(MyName,pdu->caData+32,32);//获取MyName
                DBOperator::getInstance().AddFriend(UsrName,MyName);
                MyTcpServer::getInstance().resend(MyName,pdu);//将添加好友回复转发到发送方
                break;
            }
            case ENUM_MSG_TYPE_ADDFRIEND_REFUSE:{
                char MyName[32]={'\0'};
                strncpy(MyName,pdu->caData+32,32);//获取MyName
                MyTcpServer::getInstance().resend(MyName,pdu);//将添加好友回复转发到发送方
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:{
                char MyName[32]={'\0'};
                strncpy(MyName,pdu->caData,32);//获取MyName
                QStringList ret = DBOperator::getInstance().handleFlushFriend(MyName);
                uint MsgLen = ret.size()*32;
                PDU *respdu = mkPDU(MsgLen);
                for(int i=0;i<ret.size();i++){
                    memcpy((char*)(respdu->caData)+i*32,
                           ret.at(i).toStdString().c_str(),
                           ret.at(i).size());
                }
                respdu->MsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu=NULL;
                break;
            }
            case ENUM_MSG_TYPE_DELETE_FIREND_REQUEST:{
                char MyName[32]={'\0'};
                char UsrName[32]={'\0'};
                strncpy(MyName,pdu->caData,32);
                strncpy(UsrName,pdu->caData+32,32);
                DBOperator::getInstance().handleDelFriend(MyName,UsrName);
                PDU *respdu = mkPDU(0);
                respdu->MsgType=ENUM_MSG_TYPE_DELETE_FIREND_RESPOND;
                strcpy(respdu->caData,DEL_FRIEND_OK);
                write((char*)respdu,respdu->PDULen);

                MyTcpServer::getInstance().resend(UsrName,pdu);
                free(respdu);
                respdu=NULL;
                break;
            }
            case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:{
                char chatName[32] = {'\0'};
                memcpy(chatName,pdu->caData+32,32);
                qDebug()<<chatName;
                MyTcpServer::getInstance().resend(chatName,pdu);
                break;
            }
            case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:{
                char caName[32] = {'\0'};
                strncpy(caName,pdu->caData,32);
                QStringList ret = DBOperator::getInstance().handleFlushFriend(caName);
                for(int i=0;i<ret.size();i++){
                    MyTcpServer::getInstance().resend(ret.at(i).toStdString().c_str(),pdu);
                }
                break;
            }
            case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:{
                QDir dir;
                QString strCurPath = QString("%1").arg(pdu->caMsg);
                PDU *respdu = NULL;
                if(dir.exists(strCurPath)){//用户目录已存在
                    char caNewDir[32] = {'\0'};
                    memcpy(caNewDir,pdu->caData+32,32);
                    QString strNewPath = strCurPath +"/"+caNewDir;
                    qDebug()<<strNewPath;
                    if(dir.exists(strNewPath)){//创建的文件夹名已存在
                        respdu = mkPDU(0);
                        respdu->MsgType =ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                        strcpy(respdu->caData,FILE_NAME_EXIST);
                    }
                    else{
                        dir.mkdir(strNewPath);
                        respdu = mkPDU(0);
                        respdu->MsgType =ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                        strcpy(respdu->caData,CREATE_DIR_OK);
                    }
                }
                else{//用户目录不存在
                    respdu = mkPDU(0);
                    respdu->MsgType =ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,DIR_NO_EXIST);
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu=NULL;
                break;
            }
            case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:{
                char *pCurPath = new char[pdu->MsgLen];
                memcpy(pCurPath,pdu->caMsg,pdu->MsgLen);
                QDir dir(pCurPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                PDU *respdu = mkPDU(iFileCount*sizeof(FileInfo));
                respdu->MsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for(int i=0;i<fileInfoList.size();i++){
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                    strFileName = fileInfoList[i].fileName();
                    memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                    if(fileInfoList[i].isDir()){
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileInfoList[i].isFile()){
                        pFileInfo->iFileType = 1;
                    }
                    // qDebug()<<fileInfoList[i].fileName();
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_DEL_DIR_REQUEST:{
                char caName[32]={'\0'};
                strcpy(caName,pdu->caData);
                char *pPath = new char[pdu->MsgLen];
                memcpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strPath = QString("%1%2%3").arg(pPath).arg("/").arg(caName);
                // qDebug()<<strPath;
                QFileInfo fileInfo(strPath);
                bool ret = false;
                if(fileInfo.isDir()){
                    QDir dir;
                    dir.setPath(strPath);
                    ret = dir.removeRecursively();
                }
                PDU *respdu = NULL;
                if(ret){
                    respdu = mkPDU(0);
                    memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
                }
                else{
                    respdu = mkPDU(0);
                    memcpy(respdu->caData,DEL_DIR_FAIL,strlen(DEL_DIR_FAIL));
                }
                respdu->MsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu=NULL;
                break;
            }
            case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:{
                char caOldName[32] = {'\0'};
                char caNewName[32] = {'\0'};
                strncpy(caOldName,pdu->caData,32);
                strncpy(caNewName,pdu->caData+32,32);
                char *pPath = new char[pdu->MsgLen];
                memcpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
                QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

                QDir dir;
                bool ret = dir.rename(strOldPath,strNewPath);
                PDU *respdu = mkPDU(0);
                respdu->MsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
                if(ret){
                    strcpy(respdu->caData,RENAME_FILE_OK);
                }
                else {
                    strcpy(respdu->caData,RENAME_FILE_FAIL);
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu=NULL;
            }
            case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:{
                char caEnterName[32]={'\0'};
                strncpy(caEnterName,pdu->caData,32);
                char *pPath = new char[pdu->MsgLen];
                strncpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
                QFileInfo fileInfo(strPath);
                PDU *respdu = NULL;

                if(fileInfo.isDir()){
                    QDir dir(strPath);
                    QFileInfoList fileInfoList = dir.entryInfoList();
                    int iFileCount = fileInfoList.size();
                    respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                    respdu->MsgType=ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                    FileInfo *pFileInfo = NULL;
                    QString strFileName;
                    for(int i=0;i<iFileCount;i++){
                        pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                        strFileName = fileInfoList[i].fileName();
                        memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                        if(fileInfoList[i].isDir()){
                            pFileInfo->iFileType=0;
                        }
                        else if(fileInfoList[i].isFile()){
                            pFileInfo->iFileType=1;
                        }
                    }
                }
                else if(fileInfo.isFile()){
                    respdu = mkPDU(0);
                    respdu->MsgType=ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                    strcpy(respdu->caData,ENTER_DIR_FAIL);
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu=NULL;
                break;
            }
            case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:{
                char caFileName[32]={'\0'};
                qint64 fileSize = 0;
                sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
                char *pPath = new char[pdu->MsgLen];
                memcpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
                delete []pPath;
                pPath = NULL;

                m_file.setFileName(strPath);
                //以只写的方式打开文件，若文件不存在，则会自动创建文件
                if(m_file.open(QIODevice::WriteOnly)){
                    m_bUpload = true;
                    m_iTotal = fileSize;
                    m_iRecved = 0;
                }
                break;
            }
            case ENUM_MSG_TYPE_DEL_FILE_REQUEST:{
                char caName[32] = {'\0'};
                strcpy(caName,pdu->caData);
                char *pPath = new char[pdu->MsgLen];
                memcpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caName);
                QFileInfo fileInfo(strPath);
                bool ret = false;
                if(fileInfo.isFile()){
                    QDir dir;
                    dir.remove(strPath);
                    ret = true;
                }
                PDU *respdu = NULL;
                if(ret){
                    respdu = mkPDU(0);
                    memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
                }
                else {
                    respdu = mkPDU(0);
                    memcpy(respdu->caData,DEL_FILE_FAILED,strlen(DEL_FILE_FAILED));
                }
                respdu->MsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:{
                char caFileName[32]={'\0'};
                strcpy(caFileName,pdu->caData);
                char *pPath = new char[pdu->MsgLen];
                memcpy(pPath,pdu->caMsg,pdu->MsgLen);
                QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
                delete []pPath;
                pPath = NULL;

                QFileInfo fileInfo(strPath);
                qint64 filesize = fileInfo.size();
                PDU *respdu = mkPDU(0);
                sprintf(respdu->caData,"%s %lld",caFileName,filesize);
                respdu->MsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;

                m_file.setFileName(strPath);
                m_file.open(QIODevice::ReadOnly);
                m_pTimer->start(1000);
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:{
                qDebug() << "分享文件请求已接收";
                char caSendName[32] = {'\0'};
                int num = 0;
                sscanf(pdu->caData,"%s %d",caSendName,&num);
                int size = num*32;
                PDU *respdu = mkPDU(pdu->MsgLen-size);
                //消息长度空间-总人数*32 = 分享文件路径空间
                respdu->MsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
                strcpy(respdu->caData,caSendName);
                memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->MsgLen-size);
                //此时respdu->caMsg保存文件地址
                qDebug() << pdu->caMsg;
                char caRecvName[32] = {'\0'};
                for(int i = 0;i<num;i++){
                    memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
                    MyTcpServer::getInstance().resend(caRecvName,respdu);
                    qDebug() << "分享文件通知已转发"<<" receiver:"<<caRecvName;
                }//将respdu转发到接收方
                free(respdu);
                respdu = NULL;

                respdu = mkPDU(0);
                respdu->MsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
                strcpy(respdu->caData,"share file ok");
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:{
                qDebug()<<"分享文件通知回复已接收，正在分享文件";
                char caRecvPath[32] = {'\0'};
                QString strRecvPath = QString("./%1").arg(pdu->caData);
                QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg));

                int index = strShareFilePath.lastIndexOf('/');
                QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
                strRecvPath = strRecvPath+'/'+strFileName;
                QFileInfo fileInfo(strShareFilePath);
                if(fileInfo.isFile())
                {
                    QFile::copy(strShareFilePath,strRecvPath);
                }
                else if(fileInfo.isDir())
                {
                    copyDir(strShareFilePath,strRecvPath);
                }
                break;
            }
            case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:{
                char caFileName[32] = {'\0'};
                int srcLen = 0;
                int desLen = 0;
                sscanf(pdu->caData,"%d%d%s",&srcLen,&desLen,caFileName);
                char *pSrcPath = new char[srcLen+1];
                char *pDestPath = new char[desLen+1+32];
                memset(pSrcPath,'\0',srcLen+1);
                memset(pDestPath,'\0',desLen+1+32);

                memcpy(pSrcPath,pdu->caMsg,srcLen);
                memcpy(pDestPath,(char*)(pdu->caMsg)+srcLen+1,desLen);
                qDebug()<<pSrcPath;
                qDebug()<<pDestPath;
                qDebug()<<caFileName;

                PDU *respdu = mkPDU(0);
                respdu->MsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
                QFileInfo fileInfo(pDestPath);
                if(fileInfo.isDir())
                {
                    strcat(pDestPath,"/");
                    strcat(pDestPath,caFileName);
                    qDebug()<<caFileName;
                    qDebug()<<pSrcPath;
                    qDebug()<<pDestPath;
                    bool ret = QFile::rename(pSrcPath,pDestPath);
                    if(ret){
                        strcpy(respdu->caData,MOVE_FILE_OK);
                    }
                    else {
                        strcpy(respdu->caData,MOVE_FILE_FAILED);
                    }
                }
                else if(fileInfo.isFile())
                {
                    strcpy(respdu->caData,MOVE_FILE_ERROR);
                }
                write((char*)respdu,respdu->PDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            default:
                break;
        }
    }
    else{
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        if(m_iTotal == m_iRecved){
            m_file.close();
            m_bUpload = false;
            PDU *respdu = mkPDU(0);
            respdu->MsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData,UPLOAD_FILE_OK);
            write((char*)respdu,respdu->PDULen);
            free(respdu);
            respdu = NULL;

        }
        else if(m_iRecved > m_iTotal){
            m_file.close();
            m_bUpload = false;
            PDU *respdu = mkPDU(0);
            respdu->MsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData,UPLOAD_FILE_FAILED);
            write((char*)respdu,respdu->PDULen);
            free(respdu);
            respdu = NULL;
        }
        //此处的respdu应在对文件接收完成后发送
        //client端对大文件读取一个buff并发送，server处于上传状态
        //server接收一个buff后判断已接收文件大小小于总大小，保持上传状态继续等待
        //server接收到最后一个buff后判断已接收文件大小等于总大小，接收完成
        //于是退出上传状态，并发送respond
    }
}

void MyTcpSocket::clientOffline()
{
    DBOperator::getInstance().handleOffline(this->m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char *pData = new char[4096];
    qint64 ret = 0;
    while(true){
        ret = m_file.read(pData,4096);
        if(ret > 0 && ret <= 4096){
            write(pData,ret);
        }
        else if(ret == 0){
            m_file.close();
            break;
        }
        else if(ret < 0){
            qDebug()<<"发送文件给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}
