#ifndef __IMFRAME_H__
#define __IMFRAME_H__
#include "configfilereader.h"
#include "serverinfo.h"
#include "zookeeper.h"
#include "clientlinkmgr.h"
#include "util.h"

#include "hw_push_client.h"
#include "push_provide_server.h"

#include "mi_push_client.h"


#define IMFRAME_VERSION "1.0"
#define CONFIG_FILE	 "server.conf"


#define MAX_DEVICE_TYPE 	64
#define INVALID_DEVICE_TYPE -1

#define MAX_INSTANCE_SERVICE 5
using namespace std;

/* we have this global to let the callback get easy access to it */
static pthread_mutex_t *lockarray;

#include <openssl/crypto.h>
static void lock_callback(int mode, int type, const char *file, int line)
{
	(void)file;
	(void)line;
	if(mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&(lockarray[type]));
	}
	else {
		pthread_mutex_unlock(&(lockarray[type]));
	}
}

static unsigned long thread_id(void)
{
	unsigned long ret;
	
	ret=(unsigned long)pthread_self();
	return ret;
}

static void init_locks(void)
{
	int i;
	
	printf("CRYPTO_num_locks() %d\n", CRYPTO_num_locks());
	
	lockarray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
												sizeof(pthread_mutex_t));
	for(i=0; i<CRYPTO_num_locks(); i++) {
		pthread_mutex_init(&(lockarray[i]), NULL);
	}
	
	CRYPTO_set_id_callback((unsigned long (*)())thread_id);
	CRYPTO_set_locking_callback(lock_callback);
	//CRYPTO_set_locking_callback((void (*)())lock_callback);
}

//static void kill_locks(void)
//{
//	int i;
//	
//	CRYPTO_set_locking_callback(NULL);
//	for(i=0; i<CRYPTO_num_locks(); i++)
//		pthread_mutex_destroy(&(lockarray[i]));
//	
//	OPENSSL_free(lockarray);
//}

class CIMFrame
{ 
public:
	
	CIMFrame();
	~CIMFrame();

public: 
	static CIMFrame* GetInstance(void);                           //


	static void *_StartEventLoop(void *);

	bool InitFrame(const char* pConfigFile);	 // 
	bool StartFrame(void);					// Run  framework
	void StopFrame();						// Release framework 

protected:

private:
	CConfigFileReader* 	m_pConfigReader;		//Config file srteam , 
	//CAppApnsPushserver* m_pCAppApnsPushserver;				//Application framework instance pointer

	CApushLocalSvr		*m_pApushLocalSvr[MAX_INSTANCE_SERVICE];
	CHWPushClient 		*m_pHWPushClient;


	pthread_t 	m_thread_id;
	int m_nActualServiceInst;


};

#endif
