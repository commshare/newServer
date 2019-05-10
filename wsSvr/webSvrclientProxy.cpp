/******************************************************************************
Filename: msghandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/07
Description:
******************************************************************************/
#include "configfilereader.h"
#include "webSvrclientProxy.h"
#include "im_time.h"
//#include  "redisMsgMgr.h"
#include "util.h"
#include "imappframe.h"
#include "configfilereader.h"
using namespace std;

CWebSvrClientProxy::CWebSvrClientProxy(CConfigFileReader* pConfigReader)
	:m_bRunning(false)
{
	NOTUSED_ARG(pConfigReader);
}


CWebSvrClientProxy::~CWebSvrClientProxy()
{
	StopThread();

	MsgQueue_t emptymsg;
	swap(m_queueMsg, emptymsg); //Release  msgqueue
}


void CWebSvrClientProxy::OnThreadRun(void)
{
	#if 0
	usleep(MSG_PROCESS_INTERVAL * 1000 * 1000);		//延迟几秒启动，使服务有时间注册zookeeper并连接到关联服务器
#ifdef DEBUG
	static int count = 0;
	printf("CWebSvrClientProxy start %d thread\r\n", ++count);
#endif

	string jsonMsg;

	m_bRunning = true;
	std::vector<string> jsonmsgs;
	while (m_bRunning)
	{
		//protomsgs.clear();
		jsonmsgs.clear();
		//printf("CWebSvrClientProxy %lu run %d times\r\n", pthread_self(), ++runcount);
		//读取客服消息并处理
		int index = 0;
		while (++index < 500)
		{
			//CProtoMsg msg = CReidsProtoMsgMgr::PopProtoMsg();
			string msg = CReidsProtoMsgMgr::PopCustomerServiceMsg();
			if (!msg.empty())
			{
				OnCustomerSvrMsg(msg);
			}
			else
			{
				//printf("stop read redis ls %s,read count = %d\r\n", CReidsProtoMsgMgr::getMsgReadKey(), index);
				break;
			}
			//printf("read count = %d\r\n", index);
		}


		//将用户消息发给客服
		if (m_queueMsg.size())
		{
			CAutoLock autolock(&m_Lock);
			while (m_queueMsg.size() && (jsonmsgs.size() <= MSG_PROCESS_UNIT))
			{
				jsonMsg = m_queueMsg.front();
				m_queueMsg.pop();
				jsonmsgs.push_back(jsonMsg);
			}

			if (!jsonmsgs.empty())	//如果非空
			{
				//CReidsProtoMsgMgr::InsertProtoMsg(jsonmsgs);
			}
		}
		else
		{
			//printf("usleep %d usec\r\n", MSG_PROCESS_INTERVAL * 1000);
			usleep(MSG_PROCESS_INTERVAL * 100 * 1000);
		}
	}
	DbgLog("!!!thread %p stop , thread Id = %lx", this, pthread_self());
	#endif
}

void CWebSvrClientProxy::PostClientMsg(const std::string& msg)
{
	CAutoLock autolock(&m_Lock);
	static int totalCount = 0;
	m_queueMsg.push(msg);
	DbgLog("current size = %d, total send %d msg to customer service\r\n",m_queueMsg.size(), ++totalCount);
}

void CWebSvrClientProxy::OnCustomerSvrMsg(const std::string& jsonContent)
{
	printf("read msg from redis, msgStr = %s", jsonContent.c_str());
	CIMAppFrame::GetInstance()->HandleCustomSvrMsg(jsonContent);
}

void CWebSvrClientProxy::StopThread()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		pthread_join(m_thread_id, NULL);
	}
}

bool CWebSvrClientProxy::Initialize(void)
{
	StartThread();
	return true;
}



