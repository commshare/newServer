/******************************************************************************
Filename: im_userInfo.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/14
Description: 
******************************************************************************/
#ifndef __IM_USERINFO_H__
#define __IM_USERINFO_H__

#include <string>
#include "ostype.h"
using std::string;

class  CUserInfo
{
public:
	CUserInfo(const string& userId, const string& photoUrl,const string& devToken, uint16_t devType, uint64_t lastLoginTime);
	 ~CUserInfo();

	 const string&	GetUserId()const	{ return m_sUserId; }
	 const string&	GetUserName()const	{ return m_sUserName; }
	 const string&	GetPhotoUrl()const	{ return m_sPhotoUrl; }
	 const string&	GetDevToken()const	{ return m_sDevToken; }
	 uint16_t		GetDevType()const	{ return m_nDevType; }
	 uint64_t		GetDesc()const		{ return m_nLastLoginTime; }
private:
	const string		m_sUserId;
	const string		m_sUserName = "";
	const string		m_sPhotoUrl;
	const string		m_sDevToken;
	uint16_t			m_nDevType;
	uint64_t			m_nLastLoginTime;
};
#endif // __IM_USERINFO_H__

