#ifndef __IMAPPFRAME_H__
#define __IMAPPFRAME_H__
#include "configfilereader.h"
#include "util.h"
#include "template.h"




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
	CTemplate*   		m_pTemplate;			//Module of template instance 

};


#endif
