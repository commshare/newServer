#include <unistd.h> 
#include <limits.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <curl/curl.h>
#include "mainframe.h"
#include "utility.hpp"

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

	 curl_global_init(CURL_GLOBAL_DEFAULT);

	  /* SSL 库初始化 */
	SSL_library_init();
	  /* 载入所有 SSL 算法 */
	OpenSSL_add_all_algorithms();
	  /* 载入所有 SSL 错误消息 */
	SSL_load_error_strings();


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

	//if(false == CServerInfo::GetInstance()->Initialize(m_pConfigReader))  //net frame parameter initialize.
	//{
	//	ErrLog("Err of serverinfo initilization");
	//	return false;
	//}
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

	m_pCIMPushFrame = new CIMPushFrame(m_pConfigReader);			// create application framework  and initialize it .
	if(!m_pCIMPushFrame)
	{
		ErrLog("Err of app frame definition");
		return false;
	}

	if (!m_pCIMPushFrame->Initialize())
	{
		ErrLog("Err of init app ApnsPushserver");
		return false;
	}	

	return true;	
}


bool CIMFrame::StartFrame()
{
	if(!m_pCIMPushFrame)
		return false;

	signal(SIGPIPE, SIG_IGN);

	CZookeeper::GetInstance()->StartThread();

	if(!m_pCIMPushFrame->StartApp())
	{
		ErrLog("m_pCIMPushFrame->StartApp()");
		return false;// Start application framework .
	}

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
		printf("Revision : %s\r\n", REVISION);
		printf("Compile datatime: %s %s\r\n", __DATE__, __TIME__);
		return -1;
	}
	else if (IsPidExist(argv[0])) // Exit directly if the IMFrame is exist.
	{
		ErrLog("Warning: ipush aleady startup,pid=%d", getpid());
		return -1;
	}

	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) 
	{
		ErrLog("block sigpipe error\n");
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

	if(false == gIMFrame->StartFrame())							// start server now . 
	{
		ErrLog("Failed to startup server ...");
		return -1;
	}

	curl_global_cleanup();
	//pause();
}

