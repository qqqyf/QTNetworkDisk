#ifndef BOOK_H
#define BOOK_H

#include "protocol.h"


#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void createDir();
    void flushFile();
    void updateFileList(const PDU *pdu);
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void clearEnterDir();
    QString enterDirName();
    void returnPre();
    void uploadFile();

    void uploadFileData();
    void delRegFile();
    void downloadFile();
    void setDownloadStatus(bool s);
    bool getDownLoadStatus();
    QString getSaveFilePath();
    void shareFile();
    QString getShareFileName();
    void moveFile();
    void selectDestDir();

    qint64 m_iTotal;
    qint64 m_iRecved;

signals:

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pReNamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;

    QString m_pstrSaveFilePath;
    bool m_bdownload;

    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
};

#endif // BOOK_H
