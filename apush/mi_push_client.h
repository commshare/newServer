#ifndef __MI_PUSH_CLIENT_H_INCL__
#define __MI_PUSH_CLIENT_H_INCL__


#include "ssl_post_impl.h"
#include "base_client.h"

/**
 * TODO: Add class description
 * 
 * @author   lang
 */
class CMiPushClient : public CBaseClient
{
public:
	// Constructor
	CMiPushClient(CConfigFileReader *pConfigReader)
	{
		m_pConfigReader = pConfigReader;
	}
	
	// Destructor
	virtual ~CMiPushClient();

	// Copy constructor
	// TODO: Uncomment the copy constructor when you need it.
	//CMiPushClient(const CMiPushClient& src);

	// Assignment operator
	// TODO: Uncomment the assignment operator when you need it.
	//CMiPushClient& operator=(const CMiPushClient& src);

	bool Init();
	

	bool Regist();

	void Start();
	
	void Stop();

	int AddTask(shared_ptr<APushData> data);
	
	static string appSecret;
	static string appPackageName;
private:

	
	int16_t	m_uConnectNum;
	//uint16_t	m_uMaxConnectNum;
	

	CConfigFileReader *m_pConfigReader;
	CApushLocalManager *m_pManage;

	CPostPoolMgr *m_pPostPoolMgr;
};

// Destructor implementation
inline CMiPushClient::~CMiPushClient()
{
}

// TODO: Uncomment the copy constructor when you need it.
//inline CMiPushClient::CMiPushClient(const CMiPushClient& src)
//{
//   // TODO: copy
//}

// TODO: Uncomment the assignment operator when you need it.
//inline CMiPushClient& CMiPushClient::operator=(const CMiPushClient& rhs)
//{
//   if (this == &rhs) {
//      return *this;
//   }
//
//   // TODO: assignment
//
//   return *this;
//}

#endif // __MI_PUSH_CLIENT_H_INCL__
