#include "configfilereader.h"
#include "custWebProxy.h"
#include "im_time.h"
#include  "redisMgr.h"
#include "util.h"
#include "imappframe.h"
#include "custWebTask.h"
#include "userMgr.h"

using namespace std;

CCustWebProxy::CCustWebProxy()
	:m_bRunning(false)
{
	//NOTUSED_ARG(pConfigReader);
}


CCustWebProxy::~CCustWebProxy()
{
	StopThread();

	MsgQueue_t emptymsg;
	swap(m_queueMsg, emptymsg); //Release  msgqueue
}


void CCustWebProxy::OnThreadRun(void)
{
	CWsBase::run();
}

void CCustWebProxy::PostClientMsg(const std::string& msg)
{
	CAutoLock autolock(&m_Lock);
	static int totalCount = 0;
	m_queueMsg.push(msg);
	DbgLog("current size = %d, total send %d msg to customer service\r\n",m_queueMsg.size(), ++totalCount);
}

void CCustWebProxy::OnCustomerSvrMsg(const std::string& jsonContent)
{
	printf("read msg from redis, msgStr = %s", jsonContent.c_str());
	CIMAppFrame::GetInstance()->HandleCustomSvrMsg(jsonContent);
}

void CCustWebProxy::StopThread()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		pthread_join(m_thread_id, NULL);
	}
}

bool CCustWebProxy::init(uint32_t listen_port,uint32_t work_size)
{
	CWsBase::init(listen_port,work_size);
	StartThread();
	return true;
}

void CCustWebProxy::on_message(wsServer* s, websocketpp::connection_hdl hdl, message_ptr msg)
{
	
	Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
	Json::Value tempVal;
	if(false == JsonParser.parse(msg->get_payload(), tempVal))
	{
		ErrLog("parse msg->json failed!");
		return;
	}

	DbgLog("on_message called with hdl:%p,and message:%s",hdl.lock().get(),tempVal.toStyledString().c_str());
	
	int32_t cmd = tempVal[CUST_JSON_FIELD_COMMAND].asInt();
	string  sessionId = tempVal[CUST_JSON_FIELD_SESSIONID].asString();
	
	if(CUST::MES_LOGIN != cmd)
	{
		//校验sessionId
		websocketpp::connection_hdl hd = CUserMgr::getInstance()->getSessionConn(sessionId);
		if(hd.lock().get() != hdl.lock().get())
		{
			WarnLog("[hd:%p,sessionId:%s] not matched!",hdl.lock().get(),sessionId.c_str());
			return;
		}
	}
	
	

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }

	//new a task to threadpool
	msg_context msg_inner;
	msg_inner.s = s,
	msg_inner.hdl = hdl;

	
	
	msg_inner.msg = tempVal;
	msg_inner.opcode = msg->get_opcode();
	
	CTask* pCustTask = new CCustWebTask(msg_inner);
	m_taskThreadPool.AddTask(pCustTask);


}

void CCustWebProxy::on_close(websocketpp::connection_hdl hdl)
{
	CWsBase::on_close(hdl);
	//设置链路上的cust为离线状态
	
	
	
}


void CCustWebProxy::close(websocketpp::connection_hdl hdl,const string& reason)
{
	
}



