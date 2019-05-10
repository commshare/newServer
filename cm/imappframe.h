#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__
#include "configfilereader.h"
#include "util.h"
#include "packetmgr.h"
#include "transmission.h"

#define MAX_INSTANCE_SERVICE 8


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
	CConfigFileReader* 	m_pConfigReader;		//Pointer of config file stream. 
	CTransimission*	   	m_pTransmission[MAX_INSTANCE_SERVICE]; 	//Module of transmission instance 
	int					m_nActualServiceInst;
};


#endif
