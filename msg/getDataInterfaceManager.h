#ifndef __GET_DATA_INTERFACE_MANAGER_H__
#define __GET_DATA_INTERFACE_MANAGER_H__

#include "singleton.h"
#include "HttpClient.h"
#include "jsoncpp/json/json.h"


typedef struct struct_member_info
{
	std::string usrId;
	bool newMsg;
	bool newCall;
	bool hideMsgSoundOn;
	bool undisturb;
	bool isHide;
	int pushType;
	std::string pushToken;
	std::string voipToken;
	int versionCode;
} GRP_MEMBER_INFO_, *P_GRP_MEMBER_INFO_;

typedef std::vector<GRP_MEMBER_INFO_> VEC_GRP_MEMBER_INFO;
typedef VEC_GRP_MEMBER_INFO::const_iterator VEC_GRP_MEMBER_INFO_CITOR;

typedef std::map<std::string, GRP_MEMBER_INFO_> MAP_GRP_MEMBER_INFO;
typedef MAP_GRP_MEMBER_INFO::const_iterator MAP_GRP_MEMBER_INFO_CITOR;


enum FRIEND_STATUS
{
	IS_NORMAL = 1,		// 正常
	IS_BLACK,			// 拉黑
	IS_DEL,				// 删除
	NO_RELA,			// 无关系
	IS_STRANGER			// 正常且是隐藏好友
};

typedef struct struct_friend_info
{
	std::string usrId;
	std::string frdId;
	FRIEND_STATUS friendStatus;
	FRIEND_STATUS userStatus;
	bool newMsg;
	bool newCall;
	bool hideMsgSoundOn;
	bool undisturb;			// 免打扰
	bool isHide;
	int pushType;
	std::string pushToken;
	std::string voipToken;
	int versionCode;
} FRIEND_INFO_, *P_FRIEND_INFO_;

enum GRP_STATUS
{
	GRP_DEPOSIT = 1,		// 未激活
	GRP_NORMAL,				// 正常
	GRP_DISMISS				// 解散
};

typedef struct struct_group_info
{
	std::string grpId;
	GRP_STATUS status;
	std::string masterId;
} GROUP_INFO_, *P_GROUP_INFO_;

typedef struct struct_user_info
{
	std::string usrId;
	bool newMsg;
	bool newCall;
	bool hideMsgSoundOn;
	int pushType;
	std::string pushToken;
	std::string voipToken;
	int versionCode;
} USER_INFO_, *P_USER_INFO_;

class CGetDataInterfaceManager : public Singleton<CGetDataInterfaceManager>
{
public:
	CGetDataInterfaceManager();
	~CGetDataInterfaceManager();

public:
	bool getGroupMemberInfoList(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGroupId, VEC_GRP_MEMBER_INFO& vecMemberInfo, string& strCode);
	bool getGroupMemberInfoList(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGroupId, MAP_GRP_MEMBER_INFO& mapMemberInfo, string& strCode);
	bool getFriendInfo(const std::string& strUrl, const std::string& strAppSecret,const std::string& strUsrId, const std::string& strFrdId, FRIEND_INFO_& friendInfo, string& strCode);
	bool getGroupInfo(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGrpId, GROUP_INFO_& grpInfo, string& strCode);
	bool getUserInfo(const std::string& strUrl, const std::string& strAppSecret, const std::string& strUsrId, USER_INFO_& usrInfo, string& strCode);

private:
	bool parseUserInfo(const std::string& strData, USER_INFO_& usrInfo, string& strCode);
	bool parseGroupInfo(const std::string& strData, GROUP_INFO_& grpInfo, string& strCode);
	bool parseFriendInfo(const std::string& strData, FRIEND_INFO_& friendInfo, string& strCode);
	bool parseGrpMemberInfoList(const std::string& strGrpId,  const std::string& strData, VEC_GRP_MEMBER_INFO& vecMemberInfo, string& strCode);
	bool parseGrpMemberInfoList(const std::string& strGrpId,  const std::string& strData, MAP_GRP_MEMBER_INFO& mapMemberInfo, string& strCode);
	void parseGrpMemberInfo(const Json::Value& valItem, GRP_MEMBER_INFO_& memberInfo);

private:
	CHttpClient		m_httpClient;
	
};


#endif // __GET_DATA_INTERFACE_MANAGER_H__
