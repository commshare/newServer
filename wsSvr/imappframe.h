#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__

#include <singleton.h>
#include <vector>
#include <string>
#include "custWebProxy.h"

#define MAX_INSTANCE_SERVICE 8
#define CONFIG_FILE	 "server.conf"


class CConfigFileReader;
class CWebSvrClientProxy;
class CCustomerSvrProxy;
class CIMAppFrame;
class CCustWebProxy;

class CIMAppFrame :public Singleton < CIMAppFrame >
{
public:

	~CIMAppFrame();
	virtual bool Initialize(CConfigFileReader * pReader);  	//config parameter of application 
	virtual bool StartApp();	 	// application instance definition
	virtual void StopApp();			// detroy application instance , release application resource 

	static bool HandleClientMsg(const std::string& jsonMsg);
	static bool HandleCustomSvrMsg(const std::string& jsonMsg);
private:
	CIMAppFrame();
	friend class Singleton < CIMAppFrame >;
	CConfigFileReader* 	m_pConfigReader;		//Pointer of config file stream.
	static std::vector<std::shared_ptr<CWebSvrClientProxy>	>	m_pWebSvrClientProxys;	//客户代理，负责与客服交互
	static std::vector<std::shared_ptr<CCustomerSvrProxy> >		m_pCustomerSvrProxys;		//客服代理，处理客户发过来的消息
	//CCustWebProxy m_custWebProxy;    //客服web代理
	int					m_nActualServiceInst;
};


#endif
