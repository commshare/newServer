/******************************************************************************
Filename: userInfoMgr.h
Author:TongHuaizhi			Version:im-1.0 		Date:2017/10/10
Description: 
******************************************************************************/
#include <string>
#include <map>
#include <vector>
#include <memory>

class UserInfo
{
public:
	UserInfo(const std::string& userId, const std::string& userToken) :m_userId(userId), m_userToken(userToken){}
	std::string GetUserId()const{ return m_userId; }
	std::string GetUserToken()const{ return m_userToken; }
private:
	std::string m_userId;
	std::string m_userToken;
};

class UserInfoMgr
{
public:
	static void setFileName(const std::string& filename);
	static std::map<std::string, std::string> GetAllUserInfo();
	static std::string GetUserToken(const std::string& usrId);
	static std::shared_ptr<UserInfo> getRandomUserInfo();
	~UserInfoMgr();
private:
	UserInfoMgr(){}
private:
	static std::string	m_fileName;
	static std::map<std::string, std::string> m_userInfos;
	static std::vector<UserInfo> m_vUserInfos;

};