#ifndef __IMFRAME_H__
#define __IMFRAME_H__
#include "configfilereader.h"
#include "serverinfo.h"
#include "zookeeper.h"
#include "clientlinkmgr.h"
#include "util.h"
#include "im_push_frame.h"


#define IMFRAME_VERSION "1.0"
#define REVISION "3856"

#define CONFIG_FILE	 "server.conf"


#define MAX_DEVICE_TYPE 	64
#define INVALID_DEVICE_TYPE -1
#define MAX_INSTANCE_SERVICE 5

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
	CIMPushFrame* m_pCIMPushFrame; //Application framework instance pointer
};

#endif
