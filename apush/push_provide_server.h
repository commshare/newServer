/*****************************************************************************************
Filename: push_provide_server.h
Author: lang
Description: 	
*****************************************************************************************/
#ifndef PUSH_PROVIDE_SERVER_H
#define PUSH_PROVIDE_SERVER_H

#include <string.h>
#include <fstream>
#include <curl/curl.h>
#include "configfilereader.h"
#include "util.h"
#include "clientlinkmgr.h"
#include "packet.h"
#include "jsoncpp/json/json.h"
#include "im.pub.pb.h"
#include "apush_server_manager.h"
#include "lock.h"
#include "protobuf_phase.h"



using namespace std;
using namespace im;
using namespace imsvr;


class CApushLocalManager;

class CApushLocalSvr : public CPacket
{
public:
	CApushLocalSvr(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CApushLocalSvr();

	bool Initialize(void);

	bool OnAndroidPush(std::shared_ptr<CImPdu> pPdu);

	void OnAndroidPushAck(string sUserId,UidCode_t sessionId,ErrCode bCode);

	//APNS_NOTIFY
	bool OnNotify(shared_ptr<APushData> shared_APushData);
	void OnNotifyAck(std::shared_ptr<CImPdu> pPdu);

	//奇葩华为接口,要获取token
	string  	GetToken(bool bNecessary = false);
	static CLock		*m_pLockSendResp;
protected:
	virtual bool RegistPacketExecutor(void);
private:
	CConfigFileReader* m_pConfigReader;	

	CApushLocalManager	*m_pManage;			//hold the m_pSvrToLogic and m_pApnsClient
	int 				m_nNumberOfInst;
	

	string 		m_strVer;
	string 		m_strClientId;
	string 		m_strToken;

};

#endif
