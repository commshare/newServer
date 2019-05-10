#ifndef HW_PUSH_CLIENT_H
#define HW_PUSH_CLIENT_H
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <string> 
#include <sys/socket.h>
#include "utility.hpp"
#include "protobuf_phase.h"
#include "http_client.h"
class CApushLocalManager;
class CConfigFileReader;
class CHWPushClient : public CHttpClient
{
public:
	CHWPushClient(CConfigFileReader* pConfigReader);
	~CHWPushClient();

	bool Init();

	void Start();

	void Stop(){};

    void AddTask(std::shared_ptr<HTTP_REQDATA_>);
	static void OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData);
private:
	bool Regist();
private:
	CApushLocalManager *m_pManage;
	CConfigFileReader *m_pConfigReader;
};


#endif
