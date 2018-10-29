/******************************************************************************
Filename: loginInfo.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/01
Description: 
******************************************************************************/
#include "im_loginInfo.h"

CLoginInfo::CLoginInfo(const std::string& role, const std::string& status, const std::string& cmIp,
	const std::string& cmPort, uint16_t devType, const std::string& devToken, const std::string& devVoipToken, USER_CALL_STATE callState)
	:m_sRole(role), m_sStatus(status), m_sCmPort(cmPort), m_sCmIp(cmIp), m_nDevType(devType), 
	m_sDevToken(devToken.substr(devToken.find(':') == devToken.npos ? 0 : devToken.find(':') + 1))
	, m_sDevVoipToken(devVoipToken.substr(devVoipToken.find(':') == devVoipToken.npos ? 0 : devVoipToken.find(':') + 1))
	, m_nCallState(callState),m_sDeviceID(devToken.substr(0,devToken.find(':')))
{

}

CLoginInfo::~CLoginInfo()
{

}


