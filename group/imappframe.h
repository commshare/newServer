#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__
#include "util.h"

#define MAX_INSTANCE_SERVICE 8

class CConfigFileReader;
class CGroupCreate;
class CGroupJoin;
class CGroupLeave;
class CGroupModify;
class CGroupChat;
class CGroupNotify;

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
	CConfigFileReader* 	m_pConfigReader;	//Pointer of config file stream. 
	CGroupCreate*   	m_pCreate[MAX_INSTANCE_SERVICE];	//Module of group creation instance 
	CGroupJoin*	   		m_pJoin[MAX_INSTANCE_SERVICE]; 		//Module of group joining instance 
	CGroupLeave*	   	m_pLeave[MAX_INSTANCE_SERVICE]; 	//Module of group leaving instance 
	CGroupModify*	   	m_pModify[MAX_INSTANCE_SERVICE]; 	//Module of group modification instance 
	CGroupChat*	   		m_pChat[MAX_INSTANCE_SERVICE]; 		//Module of group chat instance 
	CGroupNotify*	   	m_pNotify[MAX_INSTANCE_SERVICE]; 	//Module of group Notification instance 
	int					m_nActualServiceInst;
};


#endif
