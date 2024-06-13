#include "dboperator.h"

DBOperator::DBOperator(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

DBOperator &DBOperator::getInstance()
{
    static DBOperator instance;
    return instance;
}

void DBOperator::init()
{
    m_db.setHostName("localhost"); //数据库所在服务器IP
    m_db.setDatabaseName("D:\\Qt\\Qtprojects\\TcpServer\\cloud.db");
    if(m_db.open())
    {
        QSqlQuery query;
        //执行查询并将query指向结果集（初始query指向结果集外）
        query.exec("select * from usrInfo");
        while(query.next()){
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug()<<data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"数据库ERROR","打开数据库失败");
    }
}

DBOperator::~DBOperator()
{
    m_db.close();//关闭数据库连接
}

bool DBOperator::handleRegist(const char *name, const char *pwd)
{
    if(name==NULL||pwd==NULL){
        return false;
    }
    QString data = QString("insert into usrInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    // qDebug()<<data;
    QSqlQuery query;

    return query.exec(data);
}

bool DBOperator::handleLogin(const char *name, const char *pwd)
{
    if(name==NULL||pwd==NULL){
        return false;
    }
    QString statement = QString("select * from usrInfo where name = \'%1\'and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(statement);
    if(query.next()){
        QString s2 = QString("update usrInfo set online = 1 where name = \'%1\'and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
        query.exec(s2);
        return true;
    }
    else {
        return false;
    }
}

void DBOperator::handleOffline(const char *name)
{
    if(name == NULL){
        QMessageBox::information(NULL,"提示","name is null");
        return;
    }
    QString s = QString("update usrInfo set online = 0 where name = \'%1\' and online = 1").arg(name);
    QSqlQuery query;
    query.exec(s);
}

QStringList DBOperator::handleAllOnline()
{
    QStringList result;
    result.clear();
    QString s = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(s);
    while(query.next()){
        result.append(query.value(0).toString());
    }
    return result;
}

int DBOperator::handleSearchUsr(const char *name)
{
    if(name == NULL){
        return -1;
    }
    QString s = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(s);
    if(query.next()){
        int ret = query.value(0).toInt();
        if(ret==1){
            return 1;
        }
        else if(ret==0){
            return 0;
        }
    }
    else {
        return -1;
    }
}

int DBOperator::handleAddFriend(const char *UsrName, const char *MyName)
{
    if(UsrName == NULL || MyName == NULL){
        return -1;
    }
    QString s = QString("select * from friend where (id = (select id from usrInfo where name = \'%1\') and FriendId = (select id from usrInfo where name = \'%2\')) "
                        "or (id = (select id from usrInfo where name = \'%3\') and FriendId = (select id from usrInfo where name = \'%4\'))").arg(UsrName).arg(MyName).arg(MyName).arg(UsrName);
    // qDebug()<<s;
    QSqlQuery query;
    query.exec(s);
    if(query.next()){
        return 0;//双方已经是好友
    }
    else {
        QString s2 = QString("select online from usrInfo where name = \'%1\'").arg(UsrName);
        QSqlQuery query;
        query.exec(s2);
        qDebug()<<s2;
        if(query.next()){
            int ret = query.value(0).toInt();
            if(ret == 1){
                return 1;//添加对象在线
            }
            else if(ret == 0){
                return 2;//添加对象不在线
            }
        }
        else{
            return 3;//添加对象不存在
        }
    }
}

void DBOperator::AddFriend(const char *UsrName, const char *MyName)
{
    QSqlQuery query;
    QString s1 = QString("select id from usrInfo where name = '%1'").arg(UsrName);
    query.exec(s1);
    // qDebug()<<s1;
    int UsrId=0;
    int MyId=0;
    if(query.next()){
        UsrId = query.value(0).toInt();
    }
    s1 = QString("select id from usrInfo where name = '%1'").arg(MyName);
    query.exec(s1);
    // qDebug()<<s1;
    if(query.next()){
        MyId = query.value(0).toInt();
    }
    s1 = QString("insert into friend (id,friendId) values(%1,%2)").arg(MyId).arg(UsrId);
    query.exec(s1);
    // qDebug()<<s1;
    s1 = QString("insert into friend (id,friendId) values(%1,%2)").arg(UsrId).arg(MyId);
    query.exec(s1);
    // qDebug()<<s1;
}

QStringList DBOperator::handleFlushFriend(const char *MyName)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(MyName==NULL){
        return strFriendList;
    }
    QSqlQuery query;
    QString s = QString("select name from usrInfo where id in (select friendId from friend where id = (select id from usrInfo where name=\'%1\'))").arg(MyName);
    query.exec(s);
    while(query.next()){
        strFriendList.append(query.value(0).toString());
        qDebug()<<query.value(0).toString();
    }
    return strFriendList;
}

bool DBOperator::handleDelFriend(const char *MyName, const char *UsrName)
{
    if(MyName==NULL||UsrName==NULL){
        return false;
    }
    QString s = QString("delete from friend where id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\')").arg(MyName).arg(UsrName);
    QSqlQuery query;
    query.exec(s);
    s = QString("delete from friend where id = (select id from usrInfo where name = \'%1\') and friendId = (select id from usrInfo where name = \'%2\')").arg(UsrName).arg(MyName);
    query.exec(s);
    return true;
}
