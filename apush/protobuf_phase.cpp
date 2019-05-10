/**
* 功能:	protobuf_phase.cpp 
* 日期:2017-8-29-14:42
* 作者:jobs
**/

#include "lock.h"
#include "im.push.android.pb.h"
#include "util.h"
#include "protobuf_phase.h"
#include "hw_handle_protobuf.h"
#include "mi_protoc.h"
#include "jpush_protoc.h"
#include "google_fcm_protoc.h"
#include "configfilereader.h"
#define CONFIG_FILE  "server.conf"

int CSendDataMapMgrSharedPtr::Insert(shared_ptr<APushData> Shared_pushData, const int index)
{
	CAutoLock lock(&m_cacheMutex);

	APushData *pushData = Shared_pushData.get();
	if (!pushData)
	{
		ErrLog("pushData is null");
		return -1;
	}

	if (!m_sendDataCache.insert(make_pair(index, Shared_pushData)).second)
	{
		hash_map<int, shared_ptr<APushData>>::iterator it = m_sendDataCache.find(index);
		if (it != m_sendDataCache.end())
		{
			ErrLog("the index:%d is exist!", index);

			//return -1;
		}
	}

	return index;
}


shared_ptr<APushData> CSendDataMapMgrSharedPtr::Delete(const int index)
{
	CAutoLock lock(&m_cacheMutex);
	if (m_sendDataCache.empty())
	{
		ErrLog("m_sendDataCache empty");
		return  nullptr;
	}

	hash_map<int, shared_ptr<APushData>>::iterator it = m_sendDataCache.find(index);
	if (it != m_sendDataCache.end())
	{
		shared_ptr<APushData> data = it->second;
		m_sendDataCache.erase(it);

		return data;
	} else
	{
		InfoLog("not found index:%d", index);
	}

	return nullptr;
}


int CSendDataMapMgrSharedPtr::CheckCache(time_t timeRate)
{
	NOTUSED_ARG(timeRate);
	//CAutoLock lock(&m_cacheMutex);
	if (m_sendDataCache.empty())
	{
		InfoLog("m_sendDataCache empty");
		return  0;
	}

	int iCount= 0;
	hash_map<int, shared_ptr<APushData>>::iterator it = m_sendDataCache.begin();
	for (; it != m_sendDataCache.end(); it++)
	{
		shared_ptr<APushData> data = it->second;
		if (!data)
		{
			iCount++;
			APushData *pushData = data.get();
			InfoLog("msgid:%s, check time:%d", pushData->msgId.c_str(), pushData->timeSend);
		}
		else
		{
			   //
		}	
	}
	
	return iCount;
}

size_t CSendDataMapMgrSharedPtr::GetSize()
{
	CAutoLock lock(&m_cacheMutex);
	return m_sendDataCache.size();
}

//int CSendDataMapMgrSharedPtr::CheckCache(time_t timeRate);

int CSendDataMapMgr::Insert(PaPushData pushData, int index)
{
	CAutoLock lock(&m_cacheMutex);
	if (!pushData)
	{
		ErrLog("pushData is null");
		return -1;
	}

	if (!(m_sendDataCache.insert(make_pair(index, pushData))).second)
	{
		hash_map<int, PaPushData>::iterator it = m_sendDataCache.find(index);
		if (it != m_sendDataCache.end())
		{
			//big Error!
			ErrLog("the index is exist!");
			return -1;
		}
	}

	return index;
}


PaPushData CSendDataMapMgr::Delete(int index)
{
	CAutoLock lock(&m_cacheMutex);
	if (m_sendDataCache.empty())
	{
		ErrLog("m_sendDataCache empty");
		return  nullptr;
	}

	hash_map<int, PaPushData>::iterator it = m_sendDataCache.find(index);
	if (it != m_sendDataCache.end())
	{
		PaPushData data = it->second;
		m_sendDataCache.erase(it);

		return data;
	} else
	{
		InfoLog("not found index:%d", index);
	}

	return nullptr;
}

int CSendDataMapMgr::CheckCache(time_t timeRate)
{
	NOTUSED_ARG(timeRate);
	return 1;
}

CSendDataMapMgr::CSendDataMapMgr()
{
}


int CProtoHandele::m_CurMapIndex = 0;
CProtoHandele::CProtoHandele()
{
	
}

vector<shared_ptr<HTTP_REQDATA_>> CProtoHandele::GetAndroidPushData(const char *buf , int len , string &hwToken)
{
    vector<shared_ptr<HTTP_REQDATA_>> vecHttpData;

	if (!buf && len <= 0)
	{
		return vecHttpData;
	}

	m_pbMsg.Clear();
	m_pbMsg.ParseFromArray(buf, len);

	shared_ptr<HTTP_REQDATA_> shared_APushData(new HTTP_REQDATA_);

	if (!shared_APushData)
	{
		ErrLog("new APushData");
		return vecHttpData;
	}
	if (!m_pbMsg.IsInitialized())
	{
		ErrLog("not IsInitialized");
		return vecHttpData;
	}

	shared_APushData->toId = m_pbMsg.stoid();
	shared_APushData->msgId = m_pbMsg.smsgid();

	switch (m_pbMsg.edivece_type())
	{
		case HW_PUSH:
			{
				InfoLog("HW_PUSH");
				shared_APushData->diveceType = HW_PUSH;

				if (hwToken.empty())
				{
					ErrLog("hwToken.empty()");
					return vecHttpData;
				}

				CHwPostSendBuf postbuf;
				//data->sendBuf = postbuf.PraseProtocbufToSendBuf(buf, len, hwToken);
                shared_APushData->strUrl_ =  postbuf.HwGetHttpUrl();
                postbuf.HwGetHttpHeaders(shared_APushData->vecHeader);
                shared_APushData->strPost_ = postbuf.HwGetHttpPostData(buf, len, hwToken);
                vecHttpData.push_back(shared_APushData);
			   // if (data->sendBuf.empty())
			   // {
			   // 	ErrLog("data->sendBuf.empty()");
			   // 	return nullptr;
			   // }
				break;
			}
		case XM_PUSH:
			{
				InfoLog("XM_PUSH");
				shared_APushData->diveceType = XM_PUSH;
				CMiProtoc miProto(&m_pbMsg);
                shared_APushData->strUrl_ =  miProto.GetHttpUrl();
                miProto.GetHttpHeaders(shared_APushData->vecHeader);
                shared_APushData->strPost_ = miProto.GetHttpPostData(m_pbMsg.emsgtype() == im::P2P_CALL);
                vecHttpData.push_back(shared_APushData);

                //小米推送同时构造海外的推送数据，两个平台一起推，跟国内推送不一样的只是url
                shared_ptr<HTTP_REQDATA_> foreignPlaform_SharedData(new HTTP_REQDATA_);
                //P_HTTP_REQDATA_  pdata = foreignPlaform_SharedData.get();
                if (!foreignPlaform_SharedData)
                {
                    ErrLog("new foreignPlaform_SharedData");
                    return vecHttpData;
                }
                foreignPlaform_SharedData->toId = m_pbMsg.stoid();
                foreignPlaform_SharedData->msgId = m_pbMsg.smsgid();
				foreignPlaform_SharedData->diveceType = XM_PUSH;
                //pdata->strUrl_ =  miProto.GetHttpUrl();
                foreignPlaform_SharedData->strUrl_ =  CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("miForeignUrl") ? 
                    CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("miForeignUrl"):
                    "https://api.xmpush.global.xiaomi.com/v2/message/alias";
                miProto.GetHttpHeaders(foreignPlaform_SharedData->vecHeader);
                foreignPlaform_SharedData->strPost_ = miProto.GetHttpPostData(m_pbMsg.emsgtype() == im::P2P_CALL);
                vecHttpData.push_back(foreignPlaform_SharedData);
//				if (data->sendBuf.empty())
//				{
//					ErrLog("data->sendBuf.empty()");
//					return nullptr;
//				}
				break;
			}
		case JPUSH:
			{
				InfoLog("JPUSH");
				shared_APushData->diveceType = JPUSH;

				CJPushProtoc jPushProtoc(&m_pbMsg);
//				data->sendBuf = jPushProtoc.GetSendBuf();
//
//				if (data->sendBuf.empty())
//				{
//					ErrLog("data->sendBuf.empty()");
//					return nullptr;
//				}
                shared_APushData->strUrl_ = jPushProtoc.GetHttpUrl();
                jPushProtoc.GetHttpHeaders(shared_APushData->vecHeader);
                shared_APushData->strPost_ = jPushProtoc.GetHttpPostData();
                vecHttpData.push_back(shared_APushData);
				break;
			}
		case GOOGLE_FCM:
			{

//				InfoLog("GOOGLE_FCM");
//				m_CurMapIndex++;
//				if (m_CurMapIndex == 0x7fffffff)
//				{
//					m_CurMapIndex = 1;
//				}
//
//				data->mapIndex = m_CurMapIndex;
//
//				char indexStr[12];
//				sprintf(indexStr, "%d", data->mapIndex);
//
//				//组合msgid + mapIndex,传给xmpp的消息id
//				string msgIdCombination = m_pbMsg.smsgid() + "--" + string(indexStr);
//
//				data->diveceType = GOOGLE_FCM;
//
//				m_pbMsg.set_smsgid(msgIdCombination);
//				CXmppFcmProtoc xmpp_fcm(&m_pbMsg);
//
//				data->sendBuf = xmpp_fcm.GetSendBuf();
//				if (data->sendBuf.empty())
//				{
//					ErrLog("xmpp_fcm.GetSendBuf()");
//					return nullptr;
//				}
                InfoLog("GOOGLE_FCM");
                shared_APushData->diveceType = GOOGLE_FCM;
//				m_pbMsg.set_smsgid(msgIdCombination);
                CHttpFcmProtoc http_fcm(&m_pbMsg);
                shared_APushData->strUrl_ = http_fcm.GetFcmPushUrl();
                http_fcm.GetFcmHeaders(shared_APushData->vecHeader);
                shared_APushData->strPost_= http_fcm.GetFcmPostData();
                vecHttpData.push_back(shared_APushData);
				break;
			}
		default:
			{
				ErrLog("m_pbMsg.edivece_type : %d", m_pbMsg.edivece_type());
				return vecHttpData;
			}
	}

//	return shared_APushData;
	return vecHttpData;
}

const char CAppOpenJson::type[12] = "notify_type";
const char CAppOpenJson::id[3] = "id";
const char CAppOpenJson::name[5] = "name";
const char CAppOpenJson::url[4] = "url";
const char CAppOpenJson::extra[6] = "extra";

Json::Value CAppOpenJson::GetJsonValue()
{
	Json::Value jroot;
	
	jroot[type] = utype;

	if (!sid.empty())
	{
		jroot[id] = sid;
	}

	if (!sname.empty())
	{
		jroot[name] = sname;
	}

	if (!surl.empty())
	{
		jroot[url] = surl;
	}

	if (!sextra.empty())
	{
		jroot[extra] = sextra;
	}
	
	return jroot;
}

