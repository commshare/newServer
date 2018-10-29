/******************************************************************************
Filename: im_userInfo.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/14
Description: 
******************************************************************************/
#include "im_userInfo.h"

CUserInfo::CUserInfo(const string& userId, const string& photoUrl, 
	const string& devToken, uint16_t devType, uint64_t lastLoginTime)
	:m_sUserId(userId), m_sPhotoUrl(photoUrl), m_sDevToken(devToken.substr(devToken.find(':')==devToken.npos ? 0 : devToken.find(':') + 1)),
	m_nDevType(devType), m_nLastLoginTime(lastLoginTime)
{

}

CUserInfo::~CUserInfo()
{

}
