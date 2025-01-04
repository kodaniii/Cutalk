#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <singleton.h>
#include "userdata.h"
#include <vector>
class UserMgr
    : public QObject,public Singleton<UserMgr>
    , public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~UserMgr();
    /*设置当前登录用户的信息*/
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    /*绑定token*/
    void SetToken(QString token);

    int GetUid();
    QString GetName();
    QString GetIcon();

    /*返回_user_info对象，记录了完整user_info信息*/
    std::shared_ptr<UserInfo> GetUserInfo();

    /*添加多个JsonArray好友申请信息到申请列表_apply_list*/
    void AppendApplyList(QJsonArray array);
    /*添加好友信息到好友列表_friend_list，并存储uid到FriendInfo的映射_friend_map*/
    void AppendFriendList(QJsonArray array);
    /*获取好友申请列表*/
    std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();
    /*添加好友申请到好友申请列表_apply_list中*/
    void AddApplyList(std::shared_ptr<ApplyInfo> app);
    /*检查是否已经向某个用户发送过好友申请*/
    bool AlreadyApply(int uid);


    /*获取一页聊天列表，返回列表的每一个项，如果空，返回空vector*/
    //通过_chat_loaded返回本次加载的条目，但不对_chat_loaded更新
    std::vector<std::shared_ptr<FriendInfo>> GetChatListPerPage();
    /*是否已经加载完所有聊天页表*/
    bool IsLoadChatFin();
    /*更新已加载的聊天条目计数_chat_loaded*/
    //更新_chat_loaded
    void UpdateChatLoadedCount();

    /*获取一页联系人列表项*/
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    bool IsLoadConFin();
    void UpdateContactLoadedCount();

    /*从_friend_map中检查是否存在指定uid的好友*/
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<FriendInfo> friend_info);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    /*根据uid获取好友信息*/
    std::shared_ptr<FriendInfo> GetFriendById(int uid);
    void AppendFriendChatMsg(int friend_id, std::vector<std::shared_ptr<TextChatData>>);
private:
    UserMgr();
    /*当前登录的用户信息(uid, name, nick, icon, sex)*/
    std::shared_ptr<UserInfo> _user_info;
    /*好友申请列表，vector*/
    //TODO 这里将_apply_list改为map形式，value为bool AlreadyApply
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    /*好友列表*/
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    /*uid到FriendInfo的映射表*/
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
    //用户对应的token
    QString _token;

    /*已加载的聊天列表条目数，分页加载，懒加载策略，避免一次性加载所有数据*/
    size_t _chat_loaded;
    /*已加载的联系人列表条目数，分页加载*/
    size_t _contact_loaded;

public slots:
    void SlotAddFriendRsp(std::shared_ptr<AuthRsp> rsp);
    void SlotAddFriendAuth(std::shared_ptr<AuthInfo> auth);
};

#endif // USERMGR_H
