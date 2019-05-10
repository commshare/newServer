#include <unistd.h> 
#include <limits.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include "mainframe.h"
#include "utility.hpp"
#include "hw_push_client.h"
#include "jpush_client.h"
#include "google_push_client.h"

static int SetSysFdLimit(int iRlimMax)
{
	struct rlimit rlim;
	rlim.rlim_cur = iRlimMax;
	rlim.rlim_max = iRlimMax;
	if (setrlimit(RLIMIT_NOFILE, &rlim) != 0)
	{
		ErrLog("setrlimit");
		return -1;
	}
	else
	{
		InfoLog("setrlimit ok\n");
	}
	return 0;
}

static CIMFrame* gIMFrame = 0;

CIMFrame::CIMFrame()
{
	m_nActualServiceInst = 0;
}
CIMFrame::~CIMFrame() 
{
	pthread_join(m_thread_id, nullptr);
}

CIMFrame* CIMFrame::GetInstance(void)
{
	return (gIMFrame==0) ? new CIMFrame() : gIMFrame;
}

bool CIMFrame::InitFrame(const char * pConfigFile)
{

	SSL_library_init();
	SSL_load_error_strings();
	::init_locks();

	if(!pConfigFile || is_file_exist(pConfigFile))
	{
		ErrLog("Err of config file or the file is not exist!");
		return false;
	}
	
	if (netlib_init() == NETLIB_ERROR){
		return false;
	}

	m_pConfigReader = CConfigFileReader::GetInstance(pConfigFile);
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

	char* pInstNumber = m_pConfigReader->GetConfigName("actual_instant_number");
	if(pInstNumber==NULL)
		return false;

	m_nActualServiceInst = atoi(pInstNumber);
	if(m_nActualServiceInst > MAX_INSTANCE_SERVICE)
	{
		ErrLog("The actual service inst can't over %d!",MAX_INSTANCE_SERVICE);
		return false;
	}
	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);
	
	int i;
	for (i = 0; i < m_nActualServiceInst; i++)
	{

		m_pApushLocalSvr[i] = new CApushLocalSvr(m_pConfigReader,i);
		if (!m_pApushLocalSvr[i])
		{
			ErrLog("Err of m_pApushLocalSvr!");
			return false;
		}

		if(!m_pApushLocalSvr[i]->Initialize())
		{
			ErrLog("Err of m_pApushLocalSvr  initialization!");
			return false;
		}
	}

	m_pHWPushClient = new CHWPushClient(m_pConfigReader);
	if(!m_pConfigReader)
	{
		ErrLog("Err m_pHWPushClient!");
		return false;
	}

	if(!m_pHWPushClient->Init())
	{
		ErrLog("Err m_pHWPushClient initialization!");
		return false;
	}

	return true;	
}


//void *CIMFrame::_StartEventLoop(void *)
//{
//	CSslEventDispatch::Instance()->StartDispatch();
//	return NULL;
//}




bool CIMFrame::StartFrame()
{
	//(void)pthread_create(&m_thread_id, nullptr, _StartEventLoop, nullptr);

	signal(SIGPIPE, SIG_IGN);

	string sRunCZookeeper = m_pConfigReader->GetConfigName("Zookeeper");
	if (sRunCZookeeper != "false")
	{
		CZookeeper::GetInstance()->StartThread();
	}

	//1. huawei push
    m_pHWPushClient->Start();

	//2. google fcm push
	CGooglePushClient *fcmClient = new CGooglePushClient(m_pConfigReader);
	if (!fcmClient)
	{
		ErrLog("new CHttpFcmClient");
		return false;
	}
	if (!fcmClient->Init())
	{
		ErrLog("fcmClient->Init()");
		return false;
	}

	fcmClient->Start();

	//3. xiaomi push
	CMiPushClient *miPushClient = new CMiPushClient(m_pConfigReader);
	if (!miPushClient)
	{
		ErrLog("new miPushClient");
		return false;
	}
     //commit templely
	if (!miPushClient->Init())
	{
		ErrLog("miPushClient->Init()");
		return false;
	}
	miPushClient->Start();
    
   //4. jpush
    CJPushClient *jPushCient = new CJPushClient(m_pConfigReader);
    if (!jPushCient)
    {
    	ErrLog("new jPushCient");
    	return false;
    }
    if (!jPushCient->Init())
    {
    	ErrLog("miPushClient->Init()");
    	return false;
    }
    jPushCient->Start();
	

	printf("service started!!");
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

	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) 
	{
		ErrLog("block sigpipe error\n");
		exit(-1);
	} 

	int ulimit = 70000;
	if(0 > SetSysFdLimit(ulimit))
	{
		ErrLog("SetSysFdLimit : %d\n", ulimit);
		exit(-1);
	}

	if (IsPidExist(argv[0])) // Exit directly if the IMFrame is exist.
	{
		ErrLog("Warning: Server %s aleady startup,pid=%d",argv[0], getpid());
		return -1;
	}
	else if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		ErrLog("Current verison of IMFrame: %s\n", IMFRAME_VERSION);
	}
	
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

	//try 直接让他崩溃, 好调试
	{
		if (false == gIMFrame->StartFrame())                         // start server now .
		{
			ErrLog("Failed to startup server ...");
			return -1;
		}
	}

	pause();
}

