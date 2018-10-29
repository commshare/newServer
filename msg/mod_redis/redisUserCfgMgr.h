/******************************************************************************
Filename: redisUserCfgMgr.h
Author:TongHuaizhi 			Version:im-1.4 		Date:2018/04/24
Description: 
******************************************************************************/
#ifndef __REDISUSERCFGMGR_H__
#define __REDISUSERCFGMGR_H__


#include <memory>
#include <string>
class CUserCfg
{
public:
	CUserCfg(int msgOff = 0, int hideMsgSoundOn = 0, int callMsgOff = 0)
		:m_msgOff(msgOff), m_HideMsgSoundOn(hideMsgSoundOn), m_callMsgOff_(callMsgOff)
	{}
	bool IsGlobalNoInterruption()const{ return m_msgOff; }
	bool IsHidenMsgSoundOn()const{ return m_HideMsgSoundOn; }
	bool IsCallMsgOff() const { return m_callMsgOff_; }

private:
	const int m_msgOff = 0;				//全局消息免打搅
	const int m_HideMsgSoundOn = 0;			//隐藏消息免打搅
	const int m_callMsgOff_ =0;			// 电话消息免打扰
};

class CUserCfgMgr
{
public:
	CUserCfgMgr();
	~CUserCfgMgr();
	static const std::shared_ptr<CUserCfg> GetUserCfg(const std::string& userId);
};

#endif // __REDISUSERCFGMGR_H__

