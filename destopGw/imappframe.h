#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__
#include "util.h"

#define MAX_INSTANCE_SERVICE 8

class CConfigFileReader;
class DesktopMsgTran;
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
    DesktopMsgTran*    m_pDesktopMsgTran[MAX_INSTANCE_SERVICE];//
	int					m_nActualServiceInst;
};


#endif
