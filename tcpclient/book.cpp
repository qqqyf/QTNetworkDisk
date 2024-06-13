#include "book.h"
#include "protocol.h"
#include "tcpclient.h"
#include "sharefile.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();
    m_pTimer = new QTimer;

    m_bdownload = false;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pReNamePB = new QPushButton("重命名文件");
    m_pFlushFilePB = new QPushButton("刷新文件");



    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pReNamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB,&QPushButton::clicked,this,&Book::createDir);
    connect(m_pFlushFilePB,&QPushButton::clicked,this,&Book::flushFile);
    connect(m_pDelDirPB,&QPushButton::clicked,this,&Book::delDir);
    connect(m_pReNamePB,&QPushButton::clicked,this,&Book::renameFile);
    connect(m_pBookListW,&QListWidget::doubleClicked,this,&Book::enterDir);
    connect(m_pReturnPB,&QPushButton::clicked,this,&Book::returnPre);
    connect(m_pUploadPB,&QPushButton::clicked,this,&Book::uploadFile);
    connect(m_pTimer,&QTimer::timeout,this,&Book::uploadFileData);
    connect(m_pDelFilePB,&QPushButton::clicked,this,&Book::delRegFile);
    connect(m_pDownloadPB,&QPushButton::clicked,this,&Book::downloadFile);
    connect(m_pShareFilePB,&QPushButton::clicked,this,&Book::shareFile);
    connect(m_pMoveFilePB,&QPushButton::clicked,this,&Book::moveFile);
    connect(m_pSelectDirPB,&QPushButton::clicked,this,&Book::selectDestDir);
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新文件夹名");
    if(!strNewDir.isEmpty()){
        if(strNewDir.size()>32){
            QMessageBox::warning(this,"新建文件夹","文件夹名不能超过32字符");
        }
        else{
            QString loginName = TcpClient::getInstance().getMyName();
            QString strCurPath = TcpClient::getInstance().curPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->MsgType=ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,loginName.toStdString().c_str(),loginName.size());
            strncpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());

            TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
            free(pdu);
            pdu=NULL;
        }
    }
    else {
        QMessageBox::information(this,"新建文件夹","文件夹名不能为空");
    }
}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->MsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
    free(pdu);
    pdu=NULL;
}

void Book::updateFileList(const PDU *pdu)
{
    if(pdu==NULL){
        return;
    }
    FileInfo *pFileInfo = NULL;
    m_pBookListW->clear();
    int iCount = pdu->MsgLen/sizeof(FileInfo);
    for(int i=2;i<iCount;i++){
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        qDebug()<<pFileInfo->caFileName<<pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo->iFileType==0){
            pItem->setIcon(QIcon(QPixmap(":/cloudIcon/dir.png")));
        }
        else if(pFileInfo->iFileType==1){
            pItem->setIcon(QIcon(QPixmap(":/cloudIcon/file.png")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"删除文件夹","请选择要删除的文件夹");
    }
    else{
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(sizeof(strCurPath)+1);
        pdu->MsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        memcpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem!=NULL){
        QString strOldName = pItem->text();
        PDU *pdu = mkPDU(sizeof(strCurPath)+1);
        pdu->MsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
        QString strNewName = QInputDialog::getText(this,"重命名文件","请输入新文件名");
        memcpy(pdu->caData,strOldName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,strNewName.toStdString().c_str(),32);
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;
    }
    else {
        QMessageBox::information(this,"重命名文件","请选择要重命名的文件");
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;//临时保存进入文件夹
    // qDebug()<<strDirName;
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->MsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
    free(pdu);
    pdu=NULL;
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::enterDirName()
{
    return m_strEnterDir;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRoot = "./"+TcpClient::getInstance().getMyName();
    if(strCurPath == strRoot){
        QMessageBox::information(this,"返回上一级","返回失败：已经在根目录");
    }
    else {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index,strCurPath.size()-index);
        TcpClient::getInstance().setCurPath(strCurPath);
        clearEnterDir();
        flushFile();
    }
}

void Book::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(!m_strUploadFilePath.isEmpty()){
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);

        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();//获得文件大小

        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->MsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);

        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu=NULL;

        m_pTimer->start(1000);
    }
    else {
        QMessageBox::information(this,"提示","上传文件不能为空");
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::information(this,"上传文件","打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true){
        ret = file.read(pBuffer,4096);//ret保存这一次的读取字节数
        if(ret > 0 && ret <= 4096){
            TcpClient::getInstance().getTcpSocket()->write(pBuffer,ret);
        }
        else if(ret == 0){
            break;
        }
        else {
            QMessageBox::information(this,"上传文件","上传文件出错");
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL){
        QMessageBox::warning(this,"删除文件","请选择要删除的文件");
    }
    else {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->MsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        memcpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL){
        QMessageBox::warning(this,"下载文件","请选择要下载的文件");
    }
    else {
        QString strCurPath = TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        QString fileName = pItem->text();
        memcpy(pdu->caData,fileName.toStdString().c_str(),fileName.size());
        pdu->MsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;

        QString strSaveFilePath = QFileDialog::getSaveFileName();
        //弹出一窗口选择文件保存路径
        if(strSaveFilePath.isEmpty()){
            QMessageBox::information(this,"下载文件","请指定文件保存位置");
            m_pstrSaveFilePath.clear();
        }
        else {
            m_pstrSaveFilePath = strSaveFilePath;
        }

        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::setDownloadStatus(bool s)
{
    m_bdownload = s;
}

bool Book::getDownLoadStatus()
{
    return m_bdownload;
}

QString Book::getSaveFilePath()
{
    return m_pstrSaveFilePath;
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"分享文件","请选择要分享的文件");
        return;
    }
    else {
        m_strShareFileName = pItem->text();
    }
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden()){
        ShareFile::getInstance().show();
    }
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem != NULL)
    {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCurPath+'/'+m_strMoveFileName;

        m_pSelectDirPB->setEnabled(true);
    }
    else
    {
        QMessageBox::information(this,"移动文件","请选择要移动的文件");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem != NULL)
    {
        QString strDestDir = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strDestDir = strCurPath+'/'+strDestDir;

        int srcLen = m_strMoveFilePath.size();
        int desLen = m_strDestDir.size();
        PDU *pdu = mkPDU(srcLen+desLen+2);
        pdu->MsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,desLen,m_strMoveFileName.toStdString().c_str());

        memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
        memcpy((char*)(pdu->caMsg)+srcLen+1,m_strDestDir.toStdString().c_str(),desLen);

        TcpClient::getInstance().getTcpSocket()->write((char*)pdu,pdu->PDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::information(this,"移动文件","请选择目标文件夹");
    }
    m_pSelectDirPB->setEnabled(false);
}
