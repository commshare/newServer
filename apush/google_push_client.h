#ifndef __GOOGLE_PUSH_CLIENT_H_INCL__
#define __GOOGLE_PUSH_CLIENT_H_INCL__
#include "http_client.h"
class CApushLocalManager;
class CConfigFileReader;

class CGooglePushClient : public CHttpClient
{
public:
	CGooglePushClient(CConfigFileReader *pConfigReader)
	{
		m_pConfigReader = pConfigReader;
	}
	
	virtual ~CGooglePushClient();
	bool Init();

	bool Regist();
	void Start();
	void Stop();
	void AddTask(shared_ptr<HTTP_REQDATA_> data);
	
    static string sendId;
    static string privateAppKey;
private:

	CConfigFileReader *m_pConfigReader;
	CApushLocalManager *m_pManage;

};

inline CGooglePushClient::~CGooglePushClient()
{
}
#endif // __GOOGLE_PUSH_CLIENT_H_INCL__
