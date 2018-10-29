/**
* 功能: file protobuf_phase.h 
* param: 日期:2017-8-29-14:40 作者:jobs
**/

#ifndef PROTOBUF_PHASE_H
#define PROTOBUF_PHASE_H

#include <string>
#include "jsoncpp/json/json.h"
#include "util.h"
#include "impdubase.h"
#include "im.push.android.pb.h"



using namespace std;

typedef  bool(*CheckSendBufCallBack_t)(string strSendBuf, bool bDel, void *userData);


//static int iCurPostIndex = 0;

static int TestShared_apushData = 0;
typedef struct _apushData
{

	_apushData()
	{
		InfoLog("_apushData %d", ++TestShared_apushData);

		retStatus = EXCEPT_ERR;
	}
	~_apushData()
	{
		InfoLog("~_apushData %d", --TestShared_apushData);
	}

	int diveceType;			//设备类型 详见 enum DiveceType
	string sendBuf;			//发送数据

	UidCode_t sessionId;	//上游回话
	string msgId;			//消息id
	string toId;			//接收者id
	ErrCode retStatus;		//返回状态

	time_t	timeSend;		//发送时间
	int	mapIndex;			//map中index
}APushData, *PaPushData;


class CSendDataMapMgr
{
public:
	CSendDataMapMgr();

	int Insert(PaPushData pushData, int index);
	PaPushData Delete(int index);
	int CheckCache(time_t timeRate);

private:
	CLock					m_cacheMutex;
	hash_map<int, PaPushData> m_sendDataCache;
};


class CSendDataMapMgrSharedPtr
{
public:
	CSendDataMapMgrSharedPtr(){};

	int Insert(shared_ptr<APushData>, const int index);
	shared_ptr<APushData> Delete(const int index);
	size_t GetSize();

	void Clear()
	{
		CAutoLock lock(&m_cacheMutex);

		CheckCache(0);
		m_sendDataCache.clear();
	}

private:
	int CheckCache(time_t timeRate);

	CLock					m_cacheMutex;
	hash_map<int, shared_ptr<APushData>> m_sendDataCache;
};


/**
* 功能: 1. 解析protobuf协议数据;
		2. 申请,组装fcm, hwPush等推送数据
* 日期:2017-8-29-14:46
* 作者:jobs
**/
class CProtoHandele
{
public:
	//CProtoHandele();
	CProtoHandele();

	shared_ptr<APushData> GetaPushData(const char *buf, int len, string &hwToken);

private:
	im::ANDPushMsg m_pbMsg;


	const char *m_buf;
	int m_len;

	static int m_CurMapIndex;
};

struct CAppOpenJson
{
	Json::Value GetJsonValue();

	const static char type[12];
	int utype;

	const static char id[3];
	string sid;

	const static char name[5];
	string sname;

	const static char url[4];
	string surl;

	const static char extra[6];
	string sextra;
};


#endif
