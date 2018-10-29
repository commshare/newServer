/**
 * hw_push_client.h 
 *  
 * create by liulang 
 * desc: 
 * class CHwGetPushTokenClient: Get hw pushServer token 
 * class CHWPushClient: push the message to hw push Server
 *  
 * 2017-07-11 
 * datuhao@foxmail.com 
 */

#ifndef HW_PUSH_CLIENT_H
#define HW_PUSH_CLIENT_H

#include <openssl/ssl.h>
#include <openssl/bio.h>
 
#include <string> 
#include <sys/socket.h>


#include "configfilereader.h"
#include "utility.hpp"
#include "apush_server_manager.h"
#include "protobuf_phase.h"
#include "ssl_event.h"

#include "ssl_post_mgr.h"
//class CHWPushClient;
//class CSslSocket;
class CPostPoolMgr;
class CApushLocalManager;

class CHWPushClient : public CBaseClient
{
public:
	CHWPushClient(CConfigFileReader* pConfigReader);
	~CHWPushClient();

	bool Init();

	void Start();

	void Stop(){};

	int AddTask(shared_ptr<APushData>);

	static void OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData);
private:

	bool Regist();

private:

	int16_t	m_uConnectNum;
	uint16_t	m_uMaxConnectNum;

	CApushLocalManager *m_pManage;

	CPostPoolMgr *m_pPostPoolMgr;


	CConfigFileReader *m_pConfigReader;
};


#endif
