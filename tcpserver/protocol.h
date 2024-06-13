#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>

typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed: name exited"
#define LOGIN_OK "登录成功"
#define LOGIN_FAILED "登录失败"
#define SEARCH_USR_NO "No such people!"
#define SEARCH_USR_ONLINE "ONLINE"
#define SEARCH_USR_OFFLINE "OFFLINE"

#define UNKNOW_ERRORE "unknow error"
#define EXITED_FRIEND "friend already added"
#define FRIEND_OFFLINE "usr offline"
#define FRIEND_NO_EXITED "usr not exit"

#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NO_EXIST "dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAIL "delete dir failed"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAIL "rename file failed"

#define ENTER_DIR_FAIL "enter dir failed: not a dir"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILED "upload file failed"

#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILED "delete file failed"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILED "move file failed"
#define MOVE_FILE_ERROR "target is not dir"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,   //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,   //注册回复
    ENUM_MSG_TYPE_LOGIN_REQUEST,   //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,   //登录回复
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,  //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,  //在线用户回复
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,  //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,  //搜索用户回复
    ENUM_MSG_TYPE_ADDFRIEND_REQUEST,  //添加好友请求
    ENUM_MSG_TYPE_ADDFRIEND_RESPOND,  //添加好友回复

    ENUM_MSG_TYPE_ADDFRIEND_AGREE,    //同意好友
    ENUM_MSG_TYPE_ADDFRIEND_REFUSE,   //拒绝好友

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,//刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,//刷新好友回复

    ENUM_MSG_TYPE_DELETE_FIREND_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_DELETE_FIREND_RESPOND,//删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,//私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,//私聊回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,//群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,//群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,//创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,//创建文件夹回复

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,//刷新文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,//刷新文件回复

    ENUM_MSG_TYPE_DEL_DIR_REQUEST, //删除目录请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND, //删除目录回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST, //重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND, //重命名文件回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST, //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND, //进入文件夹回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,//上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,//上传文件回复

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,//删除文件请求
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,//删除文件回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,//下载文件回复

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,//共享文件请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,//共享文件回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTE,   //共享文件通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,//共享文件通知回复

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,//移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,//移动文件回复
    ENUM_MSG_TYPE_MAX=0x00ffffff,
};

struct FileInfo
{
    char caFileName[32];//文件名字
    int iFileType;      //文件类型
};

struct PDU
{
    uint PDULen;      //总协议数据单元长度
    uint MsgType;     //消息类型
    char caData[64];  //存放文件名或用户信息
    uint MsgLen;      //实际消息长度
    char caMsg[];        //实际消息(弹性数组)
};

PDU* mkPDU(uint MsgLen);//动态分配PDU内存

#endif // PROTOCOL_H
