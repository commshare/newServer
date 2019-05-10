/******************************************************************************
Filename: loginInfo.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/01
Description: 
******************************************************************************/
#include "loginInfo.h"

CLoginInfo::CLoginInfo(const std::string& status, const std::string& cmIp, const std::string& cmPort, const std::string& deviceToken, USER_CALL_STATE callState)
	: m_sStatus(status)
	, m_sCmPort(cmPort)
	, m_sCmIp(cmIp)
	, m_nCallState(callState)
	, m_sDeviceToken(deviceToken)
{

}

CLoginInfo::CLoginInfo(const std::string& status, const std::string& cmIp, const std::string& cmPort, const std::string& deviceToken)
	: m_sStatus(status)
	, m_sCmPort(cmPort)
	, m_sCmIp(cmIp)
	, m_sDeviceToken(deviceToken)
{

}
CLoginInfo::~CLoginInfo()
{

}


