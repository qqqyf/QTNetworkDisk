#ifndef DBOPERATOR_H
#define DBOPERATOR_H

#include <QObject>
#include <QSqlDatabase>  //连接数据库
#include <QSqlQuery>     //查询数据库
#include <QMessageBox>
#include <QDebug>
#include <QStringList>

class DBOperator : public QObject
{
    Q_OBJECT
public:
    explicit DBOperator(QObject *parent = nullptr);
    static DBOperator& getInstance();
    void init();
    ~DBOperator();

    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    void handleOffline(const char *name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char* name);
    int handleAddFriend(const char *UsrName,const char *MyName);//处理添加好友对象状态
    void AddFriend(const char *UsrName,const char *MyName);
    QStringList handleFlushFriend(const char* MyName);
    bool handleDelFriend(const char* MyName,const char* UsrName);
signals:

private:
    QSqlDatabase m_db;
};

#endif // DBOPERATOR_H
