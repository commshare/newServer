#ifndef __JPUSH_CLIENT_H_INCL__
#define __JPUSH_CLIENT_H_INCL__

#include "ssl_post_impl.h"
#include "base_client.h"


/**
 * TODO: Add class description
 * 
 * @author   liulang 
 * 20170927 
 */
class CJPushClient : public CBaseClient 
{
public:
	CJPushClient(CConfigFileReader *pConfigReader)
	{
		m_pConfigReader = pConfigReader;
	}
	
	// Destructor
	virtual ~CJPushClient()
	{ };

	bool Init();
	
	bool Regist();

	void Start();
	
	void Stop();

	int AddTask(shared_ptr<APushData> data);
	
	static string appKey;
	static string masterSecret;
private:

	
	int16_t	m_uConnectNum;
	//uint16_t	m_uMaxConnectNum;
	

	CConfigFileReader *m_pConfigReader;
	CApushLocalManager *m_pManage;

	CPostPoolMgr *m_pPostPoolMgr;
};

#endif // __JPUSH_CLIENT_H_INCL__
