#ifndef __IM_PUSH_FRAME_H__
#define __IM_PUSH_FRAME_H__


class CConfigFileReader;
class CPushHandler;
class CClientBase;

#define MAX_INSTANCE_SERVICE 5

//APNS业务处理模块,
//可包含多种业务处理模块，例如：msg服务器中包含消息模块，好友模块，离线模块等等
//这里主要包含推送请求处理模块

class CIMPushFrame
{
public:
	CIMPushFrame(CConfigFileReader* pReader);
	~CIMPushFrame();

	virtual bool Initialize();  	//config parameter of application 
	virtual bool StartApp();	 	// application instance definition
	virtual void StopApp();			// detroy application instance , release application resource 
protected:	

private:

	CConfigFileReader* 		m_pConfigReader;		//Pointer of config file stream. 
	CPushHandler*   		m_pSvrToLogic[MAX_INSTANCE_SERVICE];			//hold the local logic server data
//	CClientBase*			m_pApnsClient;			//hold the apple APNS server;
//	CClientBase*			m_pVoipPushClient;			//hold the apple APNS server;
    CClientBase*            m_pVoipHttp2DevClient;
    CClientBase*            m_pVoipHttp2ProClient;
	int m_nActualServiceInst;
};

#endif // __IM_PUSH_FRAME_H__
