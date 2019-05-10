#ifndef __JPUSH_CLIENT_H_INCL__
#define __JPUSH_CLIENT_H_INCL__
#include "http_client.h"
class CApushLocalManager;
class CConfigFileReader;

class CJPushClient : public CHttpClient
{
public:
	CJPushClient(CConfigFileReader *pConfigReader)
	{
		m_pConfigReader = pConfigReader;
	}
	
	virtual ~CJPushClient()
    {

    }

	bool Init();
	bool Regist();
	void Start();
	void Stop();
	
	void AddTask(shared_ptr<HTTP_REQDATA_> data);
	static string appKey;
	static string masterSecret;
private:
	CConfigFileReader *m_pConfigReader;
	CApushLocalManager *m_pManage;

};

#endif // __JPUSH_CLIENT_H_INCL__
