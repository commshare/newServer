/*****************************************************************************************
Filename: template.h
Author: jack			Version: im-1.0 		Date:2017/06/28
Description: 	用户登录/登出、权限检查、用户信息管理类定义
*****************************************************************************************/
#ifndef __USERMANAGE_H__
#define __USERMANAGE_H__
#include <string.h>
#include <fstream>
#include <curl/curl.h>
#include "configfilereader.h"
#include "util.h"
#include "clientlinkmgr.h"
#include "packet.h"
#include "cache.h"
#include "jsoncpp/json/json.h"
#include "im.pub.pb.h"
#include "im.cm.pb.h"

using namespace std;
using namespace im;
using namespace imsvr;


typedef struct _UserAuth
{
	string sUserId;			//Return by user center if auth checking success.
	uint8_t bRole;			//Return by user center if auth checking success;
	string sLoginToken;		//Token  for validation .
} UserAuth_t;


class CTemplate : public CPacket
{
public:
	CTemplate(CConfigFileReader* pConfigReader);
	~CTemplate();

	bool Initialize(void);
	bool OnLogin(std::shared_ptr<CImPdu> pPdu);
	bool OnLogout(std::shared_ptr<CImPdu> pPdu);
	bool OnTimer(std::shared_ptr<CImPdu> pPdu);	
protected:	
	virtual bool RegistPacketExecutor(void);
	virtual bool SetCheckAuthUrl(void);
	virtual bool SetCheckAuthInterval(void);
private:
	CConfigFileReader* m_pConfigReader;	
	string m_sCheckAuthUrl;
	uint32_t m_nCheckInterval;

	
};

#endif
