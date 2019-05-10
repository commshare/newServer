/**
* 功能: file protobuf_phase.h 
* param: 日期:2017-8-29-14:40 作者:jobs
**/

#ifndef PROTOBUF_PHASE_H
#define PROTOBUF_PHASE_H

#include <string>
#include <vector>
#include <functional>
#include "jsoncpp/json/json.h"
#include "util.h"
#include "impdubase.h"
#include "im.push.android.pb.h"

using namespace std;

typedef  bool(*CheckSendBufCallBack_t)(string strSendBuf, bool bDel, void *userData);


//static int iCurPostIndex = 0;

static int TestShared_apushData = 0;

//typedef struct _httpdata {
//    string strUrl;//推送请求url
//    vector<string> httpHeaders;//请求头部
//    string strPostData;//请求参数 
//
//}HttpData, *pHttpData;

typedef struct httpReqData_
{
    //http
	int nIndex_;
	std::vector<std::string> vecHeader;
	std::string strUrl_;
	std::string strPost_;
	std::string strCainInfo_;
	std::string strSslCert_;
	std::string strSslKey_;
	std::string strKeyPwd_;
	bool isCert_;
	httpReqData_()
	{
		nIndex_ = 0;
		strUrl_ = "";
		strPost_ = "";
		strCainInfo_ = "";
		strSslCert_ = "";
		strSslKey_ = "";
		strKeyPwd_ = "";
		isCert_ = false;

        diveceType = -1;
        sendBuf = "";
        sessionId = UidCode_t();
        msgId = "";
        toId = "";
//        retStatus = -1;
        timeSend = 0;
        mapIndex = 0;
        reSendTimes = 0;
	}
   //proto
    int diveceType;			//设备类型 详见 enum DiveceType
    std::string sendBuf;			//发送数据, 二进制数据配合底层socket
	UidCode_t sessionId;	//上游回话
    std::string msgId;			//消息id
    std::string toId;			//接收者id
	ErrCode retStatus;		//返回状态

	time_t	timeSend;		//发送时间
	int	mapIndex;			//map中index
    int reSendTimes;
	
}HTTP_REQDATA_, *P_HTTP_REQDATA_;

typedef struct _apushData
{

	_apushData()
	{
		InfoLog("_apushData %d", ++TestShared_apushData);
        httpdata = new HTTP_REQDATA_;     
		retStatus = EXCEPT_ERR;
	}
	~_apushData()
	{
        delete httpdata;
		InfoLog("~_apushData %d", --TestShared_apushData);
	}

	int diveceType;			//设备类型 详见 enum DiveceType
	string sendBuf;			//发送数据, 二进制数据配合底层socket
//    HttpData httpdata;      //发送数据，文本协议内容，配合libcurl, 代替sokcet，简化程序，二只选一
    P_HTTP_REQDATA_ httpdata;
	UidCode_t sessionId;	//上游回话
	string msgId;			//消息id
	string toId;			//接收者id
	ErrCode retStatus;		//返回状态

	time_t	timeSend;		//发送时间
	int	mapIndex;			//map中index

}APushData, *PaPushData;


typedef std::function<void (void*)> HTTPRESPONSECALLBACK;


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
    vector<shared_ptr<HTTP_REQDATA_>> GetAndroidPushData(const char *buf , int len , string &hwToken);

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
