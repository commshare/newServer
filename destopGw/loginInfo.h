/******************************************************************************
Filename: loginInfo.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/01
Description: 
******************************************************************************/
#ifndef __LOGININFO_H__
#define __LOGININFO_H__

#include <string>
#include "ostype.h"

#define HAS_LOGIN "1"
//语音呼叫状态
enum USER_CALL_STATE
{
	USER_CALL_STATE_IDEL,				// 空闲
	USER_CALL_STATE_CALLING,			// 正在呼叫
	USER_CALL_STATE_BEING_CALLED,		// 正在被叫
	USER_CALL_STATE_BUSY				// 正忙
};

class CLoginInfo
{
public:
	CLoginInfo(const std::string& status, const std::string& cmIp, const std::string& cmPort, const std::string& deviceToken, USER_CALL_STATE callState);
	CLoginInfo(const std::string& status, const std::string& cmIp, const std::string& cmPort, const std::string& deviceToken);
	~CLoginInfo();
	
	const std::string& GetCmIp()const		{ return m_sCmIp; }
	const std::string& GetCmPort()const		{ return m_sCmPort; }
	bool IsLogin()const						{ return HAS_LOGIN == m_sStatus; }
	bool IsValid()const						{ return !m_sCmIp.empty(); }
	bool IsCallBusy()const					{ return m_nCallState != USER_CALL_STATE_IDEL; }
	USER_CALL_STATE GetCallState()const		{ return m_nCallState; }
	const std::string& GetDeviceToken() const { return m_sDeviceToken; }

private:
	const std::string	m_sStatus = "";
	const std::string	m_sCmPort = "";
	const std::string	m_sCmIp = "";
	USER_CALL_STATE		m_nCallState = USER_CALL_STATE_IDEL;
	const std::string	m_sDeviceToken = "";
};


#endif // __LOGININFO_H__
