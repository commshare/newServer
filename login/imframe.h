#ifndef __IMFRAME_H__
#define __IMFRAME_H__
#include "configfilereader.h"
#include "serverinfo.h"
#include "zookeeper.h"
#include "clientlinkmgr.h"
#include "util.h"
#include "imappframe.h"

#define IMFRAME_VERSION "1.4"
#define REVISION "3860"
#define CONFIG_FILE	 "server.conf"


#define MAX_DEVICE_TYPE 	64
#define INVALID_DEVICE_TYPE -1


using namespace std;

class CIMFrame
{ 
public:
	
	CIMFrame();
	~CIMFrame();

public: 
	static CIMFrame* GetInstance(void);                           //
	bool InitFrame(const char* pConfigFile);	 // 
	bool StartFrame(void);					// Run  framework
	void StopFrame();						// Release framework 

protected:

private:
	CConfigFileReader* m_pConfigReader;		//Config file srteam , 
	CIMAppFrame* m_pIMAppFrame;				//Application framework instance pointer
};

#endif
