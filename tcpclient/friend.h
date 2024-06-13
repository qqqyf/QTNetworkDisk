#ifndef FRIEND_H
#define FRIEND_H

#include "online.h"
#include "protocol.h"

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


class Friend : public QWidget
{
    Q_OBJECT
public:
    QString m_strSearchName;

    explicit Friend(QWidget *parent = nullptr);
    void showOnline();
    void showAllOnlineUsr(PDU* pdu);
    void searchUsr();
    void flushFriend();
    void updateFriendList(PDU* pdu);
    void deleteFriend();
    void privateChat();
    void groupChat();
    void updateGroupMsg(PDU* pdu);
    QListWidget* getFriendList();

signals:

public slots:


private:
    QTextEdit *m_pShowMsgTE;
    QListWidget *m_pFriendListWidget;
    QLineEdit *m_pInputMsgLE;
    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlineUsrPB;
    QPushButton *m_pSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;

    Online *m_pOnline;


};

#endif // FRIEND_H
