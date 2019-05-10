/*****************************************************************************************
Filename: serverinfo.h
Author: jack			Version: im-1.0 		Date:2017/05/23
Description:     		关联服务器数据、连接
*****************************************************************************************/

#ifndef __SERVERINFO_H__
#define __SERVERINFO_H__

#include <string.h>
#include <tr1/unordered_map>
#include <vector>

#include "singleton.h"
#include "util.h"
#include "imconn.h"
#include "configfilereader.h"

using namespace std;
using namespace std::tr1;


namespace imsvr
{

#define MAX_RECONNECT_CNT	64
#define MIN_RECONNECT_CNT	4
#define MAX_ASSOCSER_TYPE   12
#define INVALID_ASSOCSER_ID -1

typedef enum _ASSOCIATED_SERVICE_ID          //Associated identification
{
	CM		= 1,
	MSG 	= 2,
	GROUP 	= 4,
	IPUSH	= 8,			//For Ios push device 
	APUSH	= 16,			//For Android  push device 
	BPN 	= 32,
	FS		= 64,			//freeswitch
	CSR		= 128,			//customer service
	NOTIFY	= 256,
	LOGIN	= 512,			// login service
	CHANNEL	= 1024,			// channel service
    DESKTOP = 2048          //desktop service
} ASSOCIATED_SERVICE_ID;


typedef struct 								//Base information of associated server. 
{
	string		server_ip;
	uint16_t	server_port;
	uint32_t	idle_cnt;
	uint32_t	reconnect_cnt;
	CImConn*	serv_conn;					// server link , null indicate never building connection . 
} serv_info_t;

typedef struct _AssociatedSvr				// associcated server attr, id, node path and it's type. 
{
	int16_t	nServiceId;
	string 	sServiceType;
	string 	sServicePath;
} AssociatedSvr_t;


typedef list<serv_info_t *> ServerList_t; // list of basic associated server.
typedef std::tr1::unordered_map<string, serv_info_t*>  ServerInfoMap_t;  // reserved for future using , server info map. 


typedef struct _AssocSvrList				  // list of refreshed associated server  	
{
	bool bUpdate;
	int16_t nAssocSvrId;
	//ServerList_t lsAssocSvr;
	ServerInfoMap_t mapAssocSvr;
} AssocSvrList_t;

typedef struct _AssocSvrInfo				  // array of really associated server to be connected . 
{
	int16_t nServiceId;
	int16_t nServiceCount;
	serv_info_t* pServerList;
} AssocSvrInfo_t;

struct hash_func  //hash 函数  
{  
    size_t operator()(const UidCode_t &uid) const  
    {  
         return uid.Uid_Item.nReserverd[0]*1000 + uid.Uid_Item.nReserverd[1]*100+uid.Uid_Item.nReserverd[2]*10;  
    }  
}; 

extern const AssociatedSvr_t assocSvrArray[];


template <class T>
void serv_init(AssocSvrInfo_t* pAssocSvr,int nServerCount)  //Associcated server connection initialize. 
{
	if(!pAssocSvr || !nServerCount )
		return;

	int i,j;
	AssocSvrInfo_t* pAssocService = 0; 
	serv_info_t* pSvrInfo = NULL;
	T* pLink = NULL;
	for(i = 0; i < nServerCount; i++)
	{
		pAssocService = (AssocSvrInfo_t*)&pAssocSvr[i];

		for(j = 0; j < pAssocService->nServiceCount; j++)
		{
			pSvrInfo = (serv_info_t*)&pAssocService->pServerList[j];
			pLink = (T*)pSvrInfo->serv_conn;
			if(!pLink)						//Don't create server link if the link is exist.
			{
				T* pLink = new T();
				pLink->Connect(pSvrInfo);
				pSvrInfo->serv_conn = pLink;
				pSvrInfo->idle_cnt = 0;
				pSvrInfo->reconnect_cnt = MIN_RECONNECT_CNT / 2;
			}
		}
	}
}
		


template <class T>
void serv_check_reconnect(AssocSvrInfo_t* pAssocSvr,int nServerCount) // checking associated server reconnection. 
{
	
	if(!pAssocSvr || !nServerCount )
		return;

	int i,j;
	T* pLink;
	serv_info_t* pSvrInfo = NULL;
	AssocSvrInfo_t* pAssocService = 0; 

	for(i = 0; i < nServerCount; i++)
	{
		pAssocService = (AssocSvrInfo_t*)&pAssocSvr[i];
		
		for(j = 0; j < pAssocService->nServiceCount; j++)
		{
			pSvrInfo = (serv_info_t*)&pAssocService->pServerList[j];
			pLink = (T*)pSvrInfo->serv_conn;
			if (!pLink && (pSvrInfo->reconnect_cnt > 0))
			{
				pSvrInfo->idle_cnt++;
				
				if (pSvrInfo->idle_cnt >= pSvrInfo->reconnect_cnt)
				{
					pLink = new T();
					pLink->Connect(pSvrInfo);
					pSvrInfo->serv_conn = pLink;
				}
			}
			//else if( pLink && !pLink->IsConnect())
			//{
			//	pLink->Connect(pSvrInfo);
			//}
		}
	}

}

template <class T>
void serv_reset(serv_info_t* pSvrInfo)	// associated link reset . used to release resource of server link 				
{

	if(!pSvrInfo)
		return;
	
	pSvrInfo->serv_conn = NULL;
	pSvrInfo->idle_cnt = 0; 
	//pSvrInfo->reconnect_cnt = 0;
	
	pSvrInfo->reconnect_cnt *= 2;
	if (pSvrInfo->reconnect_cnt > MAX_RECONNECT_CNT) 
	{
		pSvrInfo->reconnect_cnt = MIN_RECONNECT_CNT;
	}
		
}

}
#endif /* SERVINFO_H_ */
