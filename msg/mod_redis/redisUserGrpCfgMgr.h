/******************************************************************************
Filename: redisUserGrpCfgMgr.h
Author:TongHuaizhi 			Version:im-1.4 		Date:2018/04/24
Description: 
******************************************************************************/
#ifndef __REDISUSERGRPCFGMGR_H__
#define __REDISUSERGRPCFGMGR_H__


#include <memory>
#include <string>
class CUserGrpCfg
{
public:
	CUserGrpCfg(int msgNoInterruption = 0, int IsGrpInHidenModel = 0)
		:m_msgNoInterruption(msgNoInterruption), m_hidenFlag(IsGrpInHidenModel)
	{}
	bool IsMsgNoInterruption()const{ return m_msgNoInterruption; }
	bool IsInHidenModel()const{ return m_hidenFlag; }
private:
	const int m_msgNoInterruption = 0;				//全局消息免打搅
	const int m_hidenFlag = 0;			//隐藏消息免打搅
};

class CUserGrpCfgMgr
{
public:
	CUserGrpCfgMgr();
	~CUserGrpCfgMgr();
	static const std::shared_ptr<CUserGrpCfg> GetUserGrpCfg(const std::string& userId, const std::string& grpId);
};

#endif // __REDISUSERGRPCFGMGR_H__


