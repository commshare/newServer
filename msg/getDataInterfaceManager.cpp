#include"getDataInterfaceManager.h"
#include "util.h"
#include <algorithm>
#include "encdec.h"

CGetDataInterfaceManager::CGetDataInterfaceManager()
{
	
}

CGetDataInterfaceManager::~CGetDataInterfaceManager()
{
	
}

bool CGetDataInterfaceManager::getGroupMemberInfoList(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGroupId, VEC_GRP_MEMBER_INFO& vecMemberInfo, string& strCode)
{
	std::string strReponse = "";

	string str = strAppSecret + "app_id2group_id=" + strGroupId + strAppSecret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&group_id" + strGroupId + "&sign=" + strMD5;
	
//	std::string strPost = "group_id=" + strGroupId;
	CURLcode code = CURLE_OK;
	code = m_httpClient.Post(strUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! grp_id=%s, code=%d", strGroupId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", strUrl.c_str(), strPost.c_str(), strReponse.c_str());
	if(!parseGrpMemberInfoList(strGroupId, strReponse, vecMemberInfo, strCode))
		return false;
	
	return true;
}

bool CGetDataInterfaceManager::getGroupMemberInfoList(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGroupId, MAP_GRP_MEMBER_INFO& mapMemberInfo, string& strCode)
{
	std::string strReponse = "";
	string str = strAppSecret + "app_id2group_id" + strGroupId + strAppSecret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&group_id=" + strGroupId + "&sign=" + strMD5;
	
//	std::string strPost = "group_id=" + strGroupId;
	CURLcode code = CURLE_OK;
	code = m_httpClient.Post(strUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! grp_id=%s, code=%d", strGroupId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", strUrl.c_str(), strPost.c_str(), strReponse.c_str());
	if(!parseGrpMemberInfoList(strGroupId, strReponse, mapMemberInfo, strCode))
		return false;
	
	return true;
}


bool CGetDataInterfaceManager::getFriendInfo(const std::string& strUrl, const std::string& strAppSecret, const std::string& strUsrId, const std::string& strFrdId, FRIEND_INFO_& friendInfo, string& strCode)
{
	std::string strReponse = "";
	
	string str = strAppSecret + "app_id2friend_id" + strFrdId + "user_id" + strUsrId + strAppSecret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&friend_id=" + strFrdId + "&sign=" + strMD5 +  "&user_id=" + strUsrId;
	
//	std::string strPost = "user_id=" + strUsrId + "&friend_id=" + strFrdId;
	CURLcode code = CURLE_OK;
	code = m_httpClient.Post(strUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! user_id=%s, friend_id=%s, code=%d", strUsrId.c_str(), strFrdId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", strUrl.c_str(), strPost.c_str(), strReponse.c_str());
	friendInfo.usrId = strUsrId;
	friendInfo.frdId = strFrdId;
	if(!parseFriendInfo(strReponse, friendInfo, strCode))
		return false;	
	return true;
}

bool CGetDataInterfaceManager::getGroupInfo(const std::string& strUrl, const std::string& strAppSecret, const std::string& strGrpId, GROUP_INFO_& grpInfo, string& strCode)
{
	std::string strReponse = "";

	string str = strAppSecret + "app_id2group_id" + strGrpId + strAppSecret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&group_id=" + strGrpId + "&sign=" + strMD5;
	
//	std::string strPost = "group_id=" + strGrpId;
	CURLcode code = CURLE_OK;
	code = m_httpClient.Post(strUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! grp_id=%s, code=%d", strGrpId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", strUrl.c_str(), strPost.c_str(), strReponse.c_str());
	grpInfo.grpId = strGrpId;
	if(!parseGroupInfo(strReponse, grpInfo, strCode))
		return false;
	return true;
}

bool CGetDataInterfaceManager::getUserInfo(const std::string& strUrl, const std::string& strAppSecret, const std::string& strUsrId, USER_INFO_& usrInfo, string& strCode)
{
	std::string strReponse = "";

	string str = strAppSecret + "app_id2user_id" + strUsrId + strAppSecret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&sign=" + strMD5 +  "&user_id=" + strUsrId;
	
//	std::string strPost = "user_id=" + strUsrId;
	CURLcode code = CURLE_OK;
	code = m_httpClient.Post(strUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! user_id=%s, code=%d", strUsrId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", strUrl.c_str(), strPost.c_str(), strReponse.c_str());
	usrInfo.usrId = strUsrId;
	if(!parseUserInfo(strReponse, usrInfo, strCode))
		return false;
	return true;
}

bool CGetDataInterfaceManager::parseUserInfo(const std::string& strData, USER_INFO_& usrInfo, string& strCode)
{
	//DbgLog("response: %s", strData.c_str());
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s user_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), usrInfo.usrId.c_str(), strData.c_str());
		return false;
	}
	usrInfo.newMsg = (valRepns["data"]["is_notice_newmsg"].asString() == "0") ? false : true;
	usrInfo.newCall = (valRepns["data"]["is_notice_callin"].asString() == "0") ? false : true;
	usrInfo.hideMsgSoundOn = (valRepns["data"]["is_hidden_msgtocomm"].asString() == "0") ? false : true;
	usrInfo.voipToken = valRepns["data"]["voip_token"].asString();
	usrInfo.pushToken = valRepns["data"]["push_token"].asString();
	usrInfo.pushType = atoi(valRepns["data"]["channel_type"].asString().c_str());
	usrInfo.versionCode = atoi(valRepns["data"]["version_code"].asString().c_str());
	return true;
}

bool CGetDataInterfaceManager::parseGroupInfo(const std::string& strData, GROUP_INFO_& grpInfo, string& strCode)
{
	//DbgLog("response: %s", strData.c_str());
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s grp_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), grpInfo.grpId.c_str(), strData.c_str());
		return false;
	}
	grpInfo.masterId = valRepns["data"]["user_id"].asString();
	grpInfo.status = (GRP_STATUS)atoi(valRepns["data"]["status"].asString().c_str());
	return true;
}

bool CGetDataInterfaceManager::parseFriendInfo(const std::string& strData, FRIEND_INFO_& friendInfo, string& strCode)
{
	//DbgLog("response: %s", strData.c_str());
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s user_id=%s friend_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), friendInfo.usrId.c_str(), friendInfo.frdId.c_str(), strData.c_str());
		return false;
	}
	friendInfo.friendStatus = (FRIEND_STATUS)atoi(valRepns["data"]["friend_status"].asString().c_str());
	friendInfo.userStatus = (FRIEND_STATUS)atoi(valRepns["data"]["user_status"].asString().c_str());
	if(IS_NORMAL == friendInfo.friendStatus || IS_STRANGER == friendInfo.friendStatus)
	{
		friendInfo.newMsg = (valRepns["data"]["is_notice_newmsg"].asString() == "0") ? false : true;
		friendInfo.newCall = (valRepns["data"]["is_notice_callin"].asString() == "0") ? false : true;
//		friendInfo.isHide = (valRepns["data"]["is_hidden"].asString() == "0") ? false : true;
		friendInfo.hideMsgSoundOn = (valRepns["data"]["is_hidden_msgtocomm"].asString() == "0") ? false : true;
		friendInfo.undisturb = (valRepns["data"]["is_undisturb"].asString() == "0") ? false : true;
		friendInfo.pushToken = valRepns["data"]["push_token"].asString();
		friendInfo.pushType = atoi(valRepns["data"]["channel_type"].asString().c_str());
		friendInfo.voipToken = valRepns["data"]["voip_token"].asString();
		friendInfo.versionCode = atoi(valRepns["data"]["version_code"].asString().c_str());
	}
	return true;
}

bool CGetDataInterfaceManager::parseGrpMemberInfoList(const std::string& strGrpId, const std::string& strData, VEC_GRP_MEMBER_INFO& vecMemberInfo, string& strCode)
{
	//DbgLog("response: %s", strData.c_str());
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s grp_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), strGrpId.c_str(), strData.c_str());
		return false;
	}

	int  nSize = valRepns["data"].size();
	for(int i = 0; i < nSize; ++i)
	{
		GRP_MEMBER_INFO_ memberInfo;
		parseGrpMemberInfo(valRepns["data"][i], memberInfo);
		vecMemberInfo.push_back(memberInfo);
	}
	return true;
}

bool CGetDataInterfaceManager::parseGrpMemberInfoList(const std::string& strGrpId,  const std::string& strData, MAP_GRP_MEMBER_INFO& mapMemberInfo, string& strCode)
{
	//DbgLog("response: %s", strData.c_str());
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s grp_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), strGrpId.c_str(), strData.c_str());
		return false;
	}

	int  nSize = valRepns["data"].size();
	for(int i = 0; i < nSize; ++i)
	{
		GRP_MEMBER_INFO_ memberInfo;
		parseGrpMemberInfo(valRepns["data"][i], memberInfo);
		mapMemberInfo.insert(MAP_GRP_MEMBER_INFO::value_type(memberInfo.usrId, memberInfo));
	}
	return true;
}

void CGetDataInterfaceManager::parseGrpMemberInfo(const Json::Value& valItem, GRP_MEMBER_INFO_& memberInfo)
{
	memberInfo.usrId = valItem["user_id"].asString().c_str();
	memberInfo.newMsg = (valItem["is_notice_newmsg"].asString() == "0") ? false : true;
	//memberInfo.newCall = (valItem["is_notice_callin"].asString() == "0") ? false : true;
	memberInfo.isHide = (valItem["is_hidden"].asString() == "0") ? false : true;
	memberInfo.hideMsgSoundOn = (valItem["is_hidden_msgtocomm"].asString() == "0") ? false : true;
	memberInfo.voipToken = valItem["voip_token"].asString();
	memberInfo.pushToken = valItem["push_token"].asString();
	memberInfo.pushType = atoi(valItem["channel_type"].asString().c_str());
	memberInfo.undisturb = atoi(valItem["is_undisturb"].asString().c_str());
	memberInfo.versionCode = atoi(valItem["version_code"].asString().c_str());
}



