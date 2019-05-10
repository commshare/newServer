/******************************************************************************
Filename: redisLoginInfoMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/

#ifndef __REDISLOGININFOMGR_H__
#define __REDISLOGININFOMGR_H__

#include <memory>
#include <string>
class CLoginInfo;

class CLoginInfoMgr
{
public:
	CLoginInfoMgr();
	~CLoginInfoMgr();
	static std::shared_ptr<CLoginInfo> GetLoginInfo(const std::string& userId);
	static bool UpdateCallBusyState(const std::string& userId, uint16_t callState);
	static std::string getDeviceLastUserID(const std::string& deviceToken);
};

#endif // __REDISLOGININFOMGR_H__
