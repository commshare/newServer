#include <unistd.h> 
#include <limits.h>
#include <string.h>
#include "imframe.h"

static CIMFrame* gIMFrame = 0;

CIMFrame::CIMFrame()
{

}
CIMFrame::~CIMFrame()
{

}

CIMFrame* CIMFrame::GetInstance(void)
{
	return (gIMFrame==0) ? new CIMFrame() : gIMFrame;
}

bool CIMFrame::InitFrame(const char * pConfigFile)
{
	if(!pConfigFile || is_file_exist(pConfigFile))
	{
		ErrLog("Err of config file or the file is not exist!");
		return false;
	}
	
	if (netlib_init() == NETLIB_ERROR){
		return false;
	}

	m_pConfigReader = new CConfigFileReader(pConfigFile);
	if(!m_pConfigReader)
	{
		ErrLog("Err of Confile reader initialization!");
		return false;
	}
	
    if(false == CZookeeper::GetInstance()->Initialize(m_pConfigReader)) // server link and zookeeper host initialize 
    {
    	ErrLog("Err of CZookeeper initialization");
    	return false;
    }
	else if(false == CClientLinkMgr::GetInstance()->Initialize(m_pConfigReader)) //client link and creating listen port for client . 
	{
		ErrLog("Err of CClientLinkMgr initialization");
		return false;
	}

	m_pIMAppFrame = new CIMAppFrame(m_pConfigReader);			// create application framework  and initialize it .
	if(!m_pIMAppFrame)
	{
		ErrLog("Err of app frame definition");
		return false;
	}

	if (!m_pIMAppFrame->Initialize())
	{
		ErrLog("Err of init imapp frame");
		return false;
	}

	return true;	

}


bool CIMFrame::StartFrame()
{
	if(!m_pIMAppFrame)
		return false;

	signal(SIGPIPE, SIG_IGN);

	CZookeeper::GetInstance()->StartThread();

	m_pIMAppFrame->StartApp();									// Start application framework . 

	writePid();
	netlib_eventloop();			//Enter eventloop
	
	return true;
}

void CIMFrame::StopFrame()
{

}

/////////////////////////////////////////////////////////////////////////////
// 
int main(int argc, char** argv)
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0))
	{
		printf("Current verison of IMFrame: %s\r\n", IMFRAME_VERSION);
		printf("Base version: %s\r\n",BaseVersion().c_str());
		printf("Compile datatime: %s %s\r\n", __DATE__, __TIME__);
		return 0;
	}
	else if (IsPidExist(argv[0])) // Exit directly if the IMFrame is exist.
	{
		ErrLog("Warning: GRP SVR aleady startup,pid=%d", getpid());
		return -1;
	}
	srand(time(NULL));
	gIMFrame =  CIMFrame::GetInstance();
	if(NULL == gIMFrame)
	{
		ErrLog("Failed to define server instance!");
		return -1;
	}
	
	if(false == gIMFrame->InitFrame(CONFIG_FILE))
	{
		ErrLog("Failed to initialize server ...");
		return -1;
	}

	if(false == gIMFrame->StartFrame())							// start server now . 
	{
		ErrLog("Failed to startup server ...");
		return -1;
	}

	//pause();
}

