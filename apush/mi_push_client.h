#ifndef __MI_PUSH_CLIENT_H_INCL__
#define __MI_PUSH_CLIENT_H_INCL__
#include "http_client.h"
class CApushLocalManager;
class CConfigFileReader;

class CMiPushClient : public CHttpClient
{
public:
	CMiPushClient(CConfigFileReader *pConfigReader)
	{
		m_pConfigReader = pConfigReader;
	}
	
	virtual ~CMiPushClient();

	bool Init();
	bool Regist();
	void Start();
	void Stop();
	void AddTask(shared_ptr<HTTP_REQDATA_> data);
	
	static string appSecret;
	static string appPackageName;
private:

	CConfigFileReader *m_pConfigReader;
	CApushLocalManager *m_pManage;

};

inline CMiPushClient::~CMiPushClient()
{
}

#endif // __MI_PUSH_CLIENT_H_INCL__
