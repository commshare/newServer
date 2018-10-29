/******************************************************************************
Filename: userInfoMgr.cpp
Author:TongHuaizhi			Version:im-1.0 		Date:2017/10/10
Description: 
******************************************************************************/
#include "userInfoMgr.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h> 
using namespace std;

std::pair<string, string> GetUserInfoFromString(const string& line)
{
	int firstPos = line.find(',', 0);
	int secondPos = line.find(',', firstPos+1);
	int thirdPos = line.find(',', secondPos+1);
	int lastPos = line.find_last_of(',');

	string userId = line.substr(secondPos + 1, thirdPos - secondPos-1);
	string userToken = line.substr(lastPos+1);

//	cout << secondPos << "\t" << thirdPos << "\t" <<  userId << "\t" << userToken << endl;
	return std::pair<string, string>(userId, userToken);
}

std::map<std::string, std::string> GetAllUserInfoFromFile(const string& fileName)
{
	std::map<std::string, std::string> userInfos;
	ifstream in(fileName);
	string line;

	if (in) // 有该文件  
	{
		while (getline(in, line)) // line中不包括每行的换行符  
		{
			/*cout << line << endl;*/
			userInfos.insert(GetUserInfoFromString(line));
		}
	}
	else // 没有该文件  
	{
		cout << "no such file" << endl;
	}

	return userInfos;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UserInfoMgr::setFileName(const std::string& filename)
{
	if (m_fileName == filename)
	{
		return;
	}
	m_userInfos.clear();
	m_fileName = filename;
	m_vUserInfos.clear();

	m_userInfos = GetAllUserInfoFromFile(m_fileName);
	std::map<std::string, std::string>::iterator it = m_userInfos.begin();
	while(it != m_userInfos.end())
	{
		m_vUserInfos.push_back(UserInfo(it->first, it->second));
		++it;
	}
	srand((unsigned)time(NULL));
}

std::map<std::string, std::string> UserInfoMgr::GetAllUserInfo()
{
	return m_userInfos;
}



std::string UserInfoMgr::GetUserToken(const string& usrId)
{
	if (m_userInfos.find(usrId) != m_userInfos.end())
	{
		return m_userInfos[usrId];
	}
	return string();
}

//UserInfoMgr* UserInfoMgr::GetInstance()
//{
//	static UserInfoMgr userInfoMgr;
//	return &userInfoMgr;
//}

UserInfoMgr::~UserInfoMgr()
{

}

std::shared_ptr<UserInfo> UserInfoMgr::getRandomUserInfo()
{
	if (m_vUserInfos.empty())
	{
		return std::shared_ptr<UserInfo>(NULL);
	}

	int index = rand() % m_vUserInfos.size();
	return std::shared_ptr<UserInfo>(new UserInfo(m_vUserInfos[index]));
}

std::map<std::string, std::string> UserInfoMgr::m_userInfos;
std::vector<UserInfo> UserInfoMgr::m_vUserInfos;
std::string UserInfoMgr::m_fileName;
