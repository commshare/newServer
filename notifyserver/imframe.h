#ifndef __IMFRAME_H__
#define __IMFRAME_H__

//#include "util.h"


#define IMFRAME_VERSION MAJOR_VERSION"." MINOR_VERSION

//#define MAX_DEVICE_TYPE 	64
//#define INVALID_DEVICE_TYPE -1

class CConfigFileReader;
class CIMAppFrame;

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
extern void SignHandler(int iSignNo);
#endif
