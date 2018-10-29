//
//  utility.hpp
//  APNS_PushServer
//
//  Created by lang on 28/06/2017.
//  Copyright © 2017 lang. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <sys/time.h>
#include <stdlib.h>
#include <string>

#include <stdio.h>
#include <string.h>

#include "base64.h"
#include "util.h"

#define SIGN_ERROR(V)	do{InfoLog("%d", (int)V); ret=""; goto jwt_sign_sha_pem_done; }while(0);

#define TEST

//static const char		*sIAT				= "iat";	//classCHeadtimestap

//#ifdef TEST
//static const char		*sTLSHOST		= "api.development.push.apple.com";		//tlstesthost
//static const char		*sHTTP2HOST		= "api.development.push.apple.com";		//http2testhost


//#elif PRODUCT
//static const char		*sTLSHOST		= "www.apple.com";		//tlsproducthost
//static const char		*sHTTP2HOST		= "api.push.apple.com";		//http2producthost
//#endif


static const short THREADPOOL_MAX_SIZE 			= 500;			//thread pool max size
static const short THREADPOOL_MIN_SIZE 			= 1;			//thread pool min size
static const short THREADPOOL_DEFAULT_SIZE 		= 100;			//thread pool default

static const short POST_BUF_SIZE 				= 5120;

//https://developer.apple.com

//static const char *RESPBEG				= "HTTP/2";
//static const char *APNS_SUCCESS 	    = "200";		//Success

/*
static const char *BAD_REQUEST 	    	= "400";		//Bad request
static const char *CE_AUTHTOKEN_ERR 	= "403";		//There was an error with the certificate or with the provider authentication token
static const char *BAD_METHOD  	    	= "405";		//The request used a bad :method value. Only POST requests are supported.
static const char *TOKEN_TIMEAWAY		= "410";		//The device token is no longer active for the topic.
static const char *DEVICE_TOKEN 	    = "413";		//The notification payload was too large.
static const char *TOO_MANY 		   	= "429";		//The server received too many requests for the same device token.
static const char *INTERNAL 		   	= "500";		//Internal server error
static const char *SHUTTING 		   	= "503";		//The server is shutting down and unavailable. 
*/




class CApnsPostData;
typedef  void (*ResponeCallBack_t)(shared_ptr<CApnsPostData>, void *userData);

/**
 获取当前系统时间的毫秒级

 @return 系统毫米时间 
 */
//static unsigned long GetTickTime()
//{
//	struct  timeval    tv;
//	
//	gettimeofday(&tv, nullptr);
//
//	return (tv.tv_sec*1000 + tv.tv_usec/1000);
//
//}

//
//static void S_usleep(unsigned int usec)
//{
//	unsigned int sec = usec/(1000000);
//	unsigned int utmpsec = 0;
//	if (sec > 0)
//	{
//		utmpsec = sec % 1000000;
//	}
//	struct  timeval    tv{sec, utmpsec};
//
//	select(0, NULL, NULL, NULL, &tv);
//}

//static std::string rfc1738_encode( const std::string& src, bool bigOrLittle = false )
//{
//
//	char hex[] = "0123456789ABCDEF";
//	if (bigOrLittle == true)
//	{
//		strcpy(hex, "0123456789abcdef");
//	}
//	
//	
//		std::string dst;
//		for (size_t i = 0; i < src.size(); ++i)
//		{
//				unsigned char c = static_cast<unsigned char>(src[i]);
//				if (isalnum(c))
//				{
//						dst += c;
//				}
//				else
//						if (c == ' ')
//						{
//								dst += '+';
//						}
//						else
//						{
//								dst += '%';
//								dst += hex[c / 16];
//								dst += hex[c % 16];
//						}
//		}
//		return dst;
//}



/* we have this global to let the callback get easy access to it */
//static pthread_mutex_t *lockarray;

#include <openssl/crypto.h>
//static void lock_callback(int mode, int type, const char *file, int line)
//{
//	(void)file;
//	(void)line;
//	if(mode & CRYPTO_LOCK) {
//		pthread_mutex_lock(&(lockarray[type]));
//	}
//	else {
//		pthread_mutex_unlock(&(lockarray[type]));
//	}
//}

//static unsigned long thread_id(void)
//{
//	unsigned long ret;
//	
//	ret=(unsigned long)pthread_self();
//	return ret;
//}

//static void init_locks(void)
//{
//	int i;
//	
//	printf("CRYPTO_num_locks() %d\n", CRYPTO_num_locks());
//	
//	lockarray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
//												sizeof(pthread_mutex_t));
//	for(i=0; i<CRYPTO_num_locks(); i++) {
//		pthread_mutex_init(&(lockarray[i]), NULL);
//	}
//	
//	CRYPTO_set_id_callback((unsigned long (*)())thread_id);
//	CRYPTO_set_locking_callback(lock_callback);
//	//CRYPTO_set_locking_callback((void (*)())lock_callback);
//}
//
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

#endif /* utility_hpp */
