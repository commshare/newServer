#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__
#include "util.h"

#define MAX_INSTANCE_SERVICE 8

class CConfigFileReader;
class CChannelChat;
class CChannelNotify;

class CIMAppFrame
{
public:
	CIMAppFrame(CConfigFileReader* pReader);
	~CIMAppFrame();

	virtual bool Initialize();  	//config parameter of application 
	virtual bool StartApp();	 	// application instance definition
	virtual void StopApp();			// detroy application instance , release application resource 

protected:	

private:
	CConfigFileReader*  m_pConfigReader;	//Pointer of config file stream. 
	CChannelChat*	    m_pChat[MAX_INSTANCE_SERVICE]; 		//Module of group chat instance 
	CChannelNotify*		m_pNotify[MAX_INSTANCE_SERVICE];
	int					m_nActualServiceInst;
};


#endif
