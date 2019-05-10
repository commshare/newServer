#include "clientlinkmgr.h"
#include "configfilereader.h"
#include "imappframe.h"
#include "imframe.h"
#include "util.h"
#include "zookeeper.h"
#include "grpcnotifyservice.h"

//static CIMFrame* gIMFrame = 0;



using namespace std;

CIMFrame::CIMFrame()
{

}
CIMFrame::~CIMFrame()
{

}

CIMFrame* CIMFrame::GetInstance(void)
{
	static CIMFrame* gIMFrame = NULL;
	if (NULL == gIMFrame)
	{
		gIMFrame = new CIMFrame();
	}
	return gIMFrame;
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
		ErrLog("Err of init app frame");
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

	if (!m_pIMAppFrame->StartApp())									// Start application framework . 
	{
		ErrLog("start IMAppFrame failed");
		return false;
	}
	writePid();
    netlib_eventloop(true);			//Enter eventloop in other thread diffrent from the caller
	
	return true;
}

void CIMFrame::StopFrame()
{
	netlib_stop_event();
	if (m_pIMAppFrame)
	{
		m_pIMAppFrame->StopApp();
	}
}

void SignHandler(int iSignNo)
{
    static bool sigIntTriggered = false;
    if (sigIntTriggered)
    {
        return;
    }
    sigIntTriggered = true;
    printf("Enter SignHandler,signo:%d\r\n", iSignNo);
    CIMFrame* pImFrame = CIMFrame::GetInstance();
    if (pImFrame)
    {
            pImFrame->StopFrame();	//结束主线程的循环
    }

    GrpcServer* grpcserver = GrpcServer::GetInstance();
    if (grpcserver) {
        grpcserver->StopServer();
    }

    printf("Leave SignHandler,signo:%d \r\n", iSignNo);
}
/*
int main(int argc, char** argv)
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) 
	{
		printf("Current verison of IMFrame: %s\r\n", IMFRAME_VERSION);
		printf("Revision : %s\r\n", REVISION);
		printf("Compile datatime: %s %s\r\n", __DATE__, __TIME__);
		return -1;
	}

	srand(time(NULL));
	WarnLog("start msgServer %s_R%s now", IMFRAME_VERSION, REVISION);
	if (NULL == CIMFrame::GetInstance())
	{
		ErrLog("Failed to define server instance!");
		return -1;
	}
	
	if (false == CIMFrame::GetInstance()->InitFrame(CONFIG_FILE))
	{
		ErrLog("Failed to initialize server ...");
		return -1;
	}
	signal(SIGINT, SignHandler);

	if (false == CIMFrame::GetInstance()->StartFrame())							// start server now . 
	{
		ErrLog("Failed to startup server ...");
		return -1;
	}
}

*/
