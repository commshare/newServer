#include "clientlinkmgr.h"
#include "configfilereader.h"
#include "imappframe.h"
#include "imframe.h"
#include "util.h"
#include "zookeeper.h"
#include "grpcnotifyservice.h"


int main(int argc, char** argv)
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) 
	{
		printf("Compile datatime: %s %s\r\n", __DATE__, __TIME__);
		return -1;
	}

	srand(time(NULL));
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

    GrpcServer* grpcserver = GrpcServer::getInstance();
    grpcserver->RunServer(CONFIG_FILE);
}

