#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__

#include "im.mes.pb.h"
#include "im.pub.pb.h"

#define MAX_INSTANCE_SERVICE 8

#define CONFIG_FILE	 "server.conf"

class CConfigFileReader;
class CFriendHandler;
class CMsgHandler;
class COfflineMsgHandler;
class CGrpMsgHandler;
class CSigHandler;

class CIMAppFrame
{
public:
	CIMAppFrame(CConfigFileReader* pReader);
	~CIMAppFrame();

	virtual bool Initialize();  	//config parameter of application 
	virtual bool StartApp();	 	// application instance definition
	virtual void StopApp();			// detroy application instance , release application resource 
private:
	CConfigFileReader* 	m_pConfigReader;		//Pointer of config file stream.
	CFriendHandler*		m_pFriendHandler[MAX_INSTANCE_SERVICE];		//Module of friend handle instance 
	CMsgHandler*		m_pMsgHandler[MAX_INSTANCE_SERVICE];
	COfflineMsgHandler* m_pOfflineMsgHandler[MAX_INSTANCE_SERVICE];
	CGrpMsgHandler*		m_pGrpMsgHandler[MAX_INSTANCE_SERVICE];	
	CSigHandler*		m_pSigHandler[MAX_INSTANCE_SERVICE];
	int					m_nActualServiceInst;
};


#endif
