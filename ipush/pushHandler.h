/*****************************************************************************************
Filename: apnspushserver.h
Author: jack lang			Version: im-1.0 		Date:2017/06/28
Description: 	
*****************************************************************************************/
#ifndef __PUSHHANDLER_H__
#define __PUSHHANDLER_H__


#include <string.h>
#include <fstream>
#include "configfilereader.h"
#include "util.h"
#include "clientlinkmgr.h"
#include "packet.h"
#include "jsoncpp/json/json.h"
#include "im.pub.pb.h"
#include "im.cm.pb.h"
#include "app_pushserver_manager.hpp"
#include "lock.h"
#include "ssl_post_mgr.h"

using namespace std;
using namespace im;
using namespace imsvr;


class CAppPushserverManager;

class CPushHandler : public CPacket
{
public:

	CPushHandler(CConfigFileReader* pConfigReader, int nNumOfInst);

	~CPushHandler();

	CAppPushserverManager *GetSvrMgr(){return m_pManage;}
	//CRecvProtoBuf *GetProtoBuf(){return m_pRecvProtoBuf;}

	bool StartCheckCacheThread();
	bool Initialize(void);

	bool OnApnsPush(std::shared_ptr<CImPdu> pPdu);

	//APNS_NOTIFY
	bool OnApnsNotify(shared_ptr<CApnsPostData> pPostData);
	void OnApnsNotifyAck(std::shared_ptr<CImPdu> pPdu);

	static CLock		*m_pLockSendResp;
	//static CLock		*m_pLockPostMutex;
protected:
	virtual bool RegistPacketExecutor(void);
private:
	void OnApnsPushAck(string sUserId, UidCode_t sSessionId, ErrCode bCode);
private:
	CConfigFileReader* m_pConfigReader;	

	//const std::string	m_strCePem;				//cetif pem file for https
	//const std::string	m_strRsaPkPem;			//the no-password rsa public key pem for https
	CAppPushserverManager	*m_pManage;			//hold the m_pSvrToLogic and m_pApnsClient

	//CRecvProtoBuf			*m_pRecvProtoBuf;				//data handle

	int 	m_nNumberOfInst;

};
#endif // __PUSHHANDLER_H__
