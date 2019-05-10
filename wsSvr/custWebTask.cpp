#include "redisMgr.h"
#include "custWebTask.h"
#include "userMgr.h"
#include "common.h"
#include "util.h"
#include "wsBase.h"

CCustWebTask::CCustWebTask(msg_context& msg)
{
	m_msg = msg;
}

void CCustWebTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	int command =  m_msg.msg[CUST_JSON_FIELD_COMMAND].asInt();
	switch(command)
	{
		case CUST::MES_LOGIN:
			loginHandler();
			break;
		case CUST::MES_CHAT_HEARTBRAT:
			heartBeatHandler();
			break;
		case CUST::STATUS_UPDATE:
			updateCustStatus();
			break;
		case CUST::SESSION_CLOSE:
			sessionCloseHandler();
			break;
		case CUST::SESSION_TRANSFER:
			sessionTransferHandler();
			break;
		case CUST::SESSION_PULL:
			sessionPullHandler();
			break;
		case CUST::SESSION_ACTIVE:
			sessionActiveHander();
			break;
		default:
			{
				WarnLog("unknown command[%d]",command);
			}
	}
		
	DbgLog("thread %lu run CCustWebTask %p, use %llu usecond", pthread_self(), this, elspsedTimer.elapsed());
}

bool CCustWebTask::send(const string jsonMsg)
{
	try {
		DbgLog("send msg[%s]",jsonMsg.c_str());
        m_msg.s->send(m_msg.hdl, jsonMsg, m_msg.opcode);
    }
	catch (websocketpp::exception const & e) 
	{
    	ErrLog("send %s faild exception[%s]",jsonMsg.c_str(),e.what());
        return false;
    }

	return true;
}

bool CCustWebTask::sendToCust(const string custId,const string jsonMsg)
{
	//--TODO
}

bool CCustWebTask::getCustCurrentSessions(const string& custId,const string& serviceId,list<CUST::SESSION_INFO>& usersInfo)
{
	bool ret=true;
	
	string cust_serviceId = CUST::packCustBind(custId,serviceId);
	list<string> users;
	ret = CRedisMgr::getInstance()->getCustCurrentUsers(cust_serviceId,users);
	if(true == ret)
	{
		list<CCustProcessed> custProcessedRecs;
		ret = m_custProcessedMgr.getCustProcesseds(custId,users,serviceId,custProcessedRecs);

		if(true == ret)
		{
			list<CCustProcessed>::iterator pCustProcessedRec = custProcessedRecs.begin();
			for(; pCustProcessedRec!=custProcessedRecs.end();pCustProcessedRec++)
			{	
				CUST::SESSION_INFO sessionInfo;
				sessionInfo.userId = pCustProcessedRec->m_sUserId;
				sessionInfo.encrypt = pCustProcessedRec->m_encrypt;
				sessionInfo.lastMsgContent = pCustProcessedRec->m_sLastMsg;
				sessionInfo.lastMsgTime = pCustProcessedRec->m_LastMsgTime;
				
				
				CUST::USER_INFO userInfo;
				getUserInfo(pCustProcessedRec->m_sUserId,userInfo);
				sessionInfo.nickName = userInfo.nickName;
				sessionInfo.headPixel = userInfo.headPixel;
				
				usersInfo.push_back(sessionInfo);
			}
		}
	}

	return ret;
}

bool CCustWebTask::dispatchCurrentSessions(const string& custId,const string& serviceId,uint32_t num,list<CUST::SESSION_INFO>& usersInfo)
{
	bool ret=true;

	list<CServiceWait> serviceWaitRecs;
	list<CCustProcessed> custProcessedRecs;
	list<CCustWait> custCustWaitRecs;
	list<string> usersCustWait,usersServiceWait,users;
	CUST::SESSION_INFO sessionInfo;
	
	//先从mongo cust排队会话中分配
	ret = m_custWaitMgr.getCustWaits(custId,serviceId,num,custCustWaitRecs);

	list<CCustWait>::iterator pCustWait = custCustWaitRecs.begin();
	for(; pCustWait != custCustWaitRecs.end(); pCustWait++)
	{
		sessionInfo.userId = pCustWait->m_sUserId;
		sessionInfo.encrypt = pCustWait->m_encrypt;
		sessionInfo.lastMsgContent = pCustWait->m_sLastMsg;
		sessionInfo.lastMsgTime = pCustWait->m_LastMsgTime;

		CUST::USER_INFO userInfo;

		getUserInfo(pCustWait->m_sUserId,userInfo);
		sessionInfo.nickName = userInfo.nickName;
		sessionInfo.headPixel = userInfo.headPixel;
				
		usersInfo.push_back(sessionInfo);
		
		custProcessedRecs.push_back(*pCustWait);
			
		usersCustWait.push_back(pCustWait->m_sUserId);
		users.push_back(pCustWait->m_sUserId);

		num--;
	}
	
	//中从mongo service排队会话中分配
	
	ret = m_serviceWaitMgr.getServiceWaits(serviceId,num,serviceWaitRecs);

	list<CServiceWait>::iterator pServiceWait = serviceWaitRecs.begin();
	for(; pServiceWait != serviceWaitRecs.end(); pServiceWait++)
	{
		
		//CCustProcessed custProcessedRec;
		
		sessionInfo.userId = pServiceWait->m_sUserId;
		sessionInfo.encrypt = pServiceWait->m_encrypt;
		sessionInfo.lastMsgContent = pServiceWait->m_sLastMsg;
		sessionInfo.lastMsgTime = pServiceWait->m_LastMsgTime;

		CUST::USER_INFO userInfo;

		getUserInfo(pServiceWait->m_sUserId,userInfo);
		sessionInfo.nickName = userInfo.nickName;
		sessionInfo.headPixel = userInfo.headPixel;
		
		usersInfo.push_back(sessionInfo);

		//custProcessedRec.m_encrypt = pServiceWait->m_encrypt;
		//custProcessedRec.m_LastMsgTime = pServiceWait->m_LastMsgTime;
		//custProcessedRec.m_sCustId = custId;
		//custProcessedRec.m_sLastMsg = pServiceWait->m_sLastMsg;
		//custProcessedRec.m_sServiceId = serviceId;
		//custProcessedRec.m_sUserId = pServiceWait->m_sUserId;
		pServiceWait->m_sCustId = custId;
		
		custProcessedRecs.push_back(*pServiceWait);
		
		usersServiceWait.push_back(pServiceWait->m_sUserId);
		users.push_back(pCustWait->m_sUserId);
		
	}

	//入cust当前队列
	if(true ==m_custProcessedMgr.insertCustProcesseds(custProcessedRecs))
	{	
		CRedisMgr::GetInstance()->usersToCustCurrent(users,custId);

		//删除custWait队列
		ret = m_custWaitMgr.deleteCustWaits(custId,usersCustWait, serviceId);
		
		//删除serviceWait队列
		ret = m_serviceWaitMgr.deleteServiceWaits(serviceId,usersServiceWait);
	}
	
	if(false == ret)
	{
		usersInfo.clear();
	}
	
	return ret;

}

bool CCustWebTask::getCustQueueSessions(const string& custId,const string& serviceId,list<CUST::SESSION_INFO>& usersInfo)
{
	bool ret=true;
	
	string cust_serviceId = CUST::packCustBind(custId,serviceId);
	list<string> users;
	ret = CRedisMgr::getInstance()->getCustQueueUsers(cust_serviceId,users);

	if(true == ret)
	{
		list<CCustWait> custWaitRecs;
		ret = m_custWaitMgr.getCustWaits(custId,users,serviceId,custWaitRecs);

		if(true == ret)
		{
			list<CCustWait>::iterator pCustWaitRec = custWaitRecs.begin();
			for(; pCustWaitRec!=custWaitRecs.end();pCustWaitRec++)
			{	
				CUST::SESSION_INFO sessionInfo;
				sessionInfo.userId = pCustWaitRec->m_sUserId;
				sessionInfo.encrypt = pCustWaitRec->m_encrypt;
				sessionInfo.lastMsgContent = pCustWaitRec->m_sLastMsg;
				sessionInfo.lastMsgTime = pCustWaitRec->m_LastMsgTime;
				
				CUST::USER_INFO userInfo;
				getUserInfo(pCustWaitRec->m_sUserId,userInfo);
				sessionInfo.nickName = userInfo.nickName;
				sessionInfo.headPixel = userInfo.headPixel;

				usersInfo.push_back(sessionInfo);
			}
		}
	}

	return ret;
	
}

bool CCustWebTask::dispatchQueueSessions(const string& custId,const string& serviceId,uint32_t num,list<CUST::SESSION_INFO>& usersInfo)
{
	bool ret=true;

	list<CServiceWait> serviceWaitRecs;
	list<CCustWait> custCustWaitRecs;
	list<string> users,usersServiceWait;
	CUST::SESSION_INFO sessionInfo;
	string cust_serviceId = CUST::packCustBind(custId,serviceId);

	#if 0
	//先从mongo cust排队会话中分配
	ret = CCustWaitMgr.getCustWaits(custId,serviceId,num,custCustWaitRecs);

	list<CCustWait>::iretator pCustWait = custCustWaitRecs.begin();
	for(; pCustWait != custCustWaitRecs.end(); pCustWait++)
	{
		sessionInfo.userId = pCustWait->m_sUserId;
		sessionInfo.m_encrypt = pCustWait->m_encrypt;
		sessionInfo.lastMsgContent = pCustWait->m_sLastMsg;
		sessionInfo.lastMsgTime = pCustWait->m_LastMsgTime;
		//去php拉取用户头像和昵称--TODE
		usersInfo.push_back(sessionInfo);
		
		
		usersServiceWait.push_back(pCustWait->m_sUserId);
		users.push_back(pCustWait->m_sUserId);

		num--;
	}
	#endif
	
	//中从mongo service排队会话中分配
	ret = m_serviceWaitMgr.getServiceWaits(serviceId,num,serviceWaitRecs);

	list<CServiceWait>::iterator pServiceWait = serviceWaitRecs.begin();
	if(serviceWaitRecs.size() > 0)
	{
		for(; pServiceWait != serviceWaitRecs.end(); pServiceWait++)
		{
			
			CCustWait custCustWaitRec;
			
			sessionInfo.userId = pServiceWait->m_sUserId;
			sessionInfo.encrypt = pServiceWait->m_encrypt;
			sessionInfo.lastMsgContent = pServiceWait->m_sLastMsg;
			sessionInfo.lastMsgTime = pServiceWait->m_LastMsgTime;

			CUST::USER_INFO userInfo;
			getUserInfo(pServiceWait->m_sUserId,userInfo);
			sessionInfo.nickName = userInfo.nickName;
			sessionInfo.headPixel = userInfo.headPixel;
			
			usersInfo.push_back(sessionInfo);

			custCustWaitRec.m_encrypt = pServiceWait->m_encrypt;
			custCustWaitRec.m_LastMsgTime = pServiceWait->m_LastMsgTime;
			custCustWaitRec.m_sCustId = custId;
			custCustWaitRec.m_sLastMsg = pServiceWait->m_sLastMsg;
			custCustWaitRec.m_sServiceId = serviceId;
			custCustWaitRec.m_sUserId = pServiceWait->m_sUserId;

			custCustWaitRecs.push_back(custCustWaitRec);
			
			users.push_back(pServiceWait->m_sUserId);
			
		}

		//入cust排队队列
		if(true ==m_custWaitMgr.insertCustWaits(custCustWaitRecs))
		{
			CRedisMgr::GetInstance()->usersToCustQueue(users,cust_serviceId);
			//删除serviceWait队列
			ret = m_serviceWaitMgr.deleteServiceWaits(serviceId,users);
		}

	
	}

	if(false == ret)
	{
		usersInfo.clear();
	}
	
	return ret;

}

void CCustWebTask::loginHandler()
{
	
	CUST::USER_STATUS custStatus;
	char cust_bind[48];
	string custId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	string cust_serviceId = CUST::packCustBind(custId,serviceId);
		
	snprintf(cust_bind,47,"%s_%s",custId.c_str(),serviceId.c_str());
	
	//php鉴权 --TODO

	//kickout
	CUserMgr::getInstance()->userKickOut(cust_serviceId);

	//设置用户状态
	CRedisMgr::getInstance()->setCustStatus(cust_bind,CUST::STATUS_ONLINE);

	string sSessionId = getuuid();
	
	
	m_msg.msg[CUST_JSON_FIELD_SESSIONID] = sSessionId;
	m_msg.msg[CUST_JSON_FIELD_ERRCODE] = 0;
	m_msg.msg[CUST_JSON_FIELD_COMMAND] = CUST::MES_LOGIN_ACK;

	//回ACK
	send(m_msg.msg.toStyledString().c_str());
	
	CUserMgr::getInstance()->addUserConn(cust_bind,m_msg.hdl);
	CUserMgr::getInstance()->addSessionConn(sSessionId,m_msg.hdl);

	#if 0
	//cust是否还有缓存会话（离线后会保存缓存一段时间）
	list<string> keys,existKeys;
	string sessionCurrentKey = CUST::packCustSessionCurrent(custId,serviceId);
	//string sessionHistoryKey = CUST::packCustSessionHistory(custId,serviceId);
	string sessionQueueKey = CUST::packCustSessionQueue(custId,serviceId);
	
	keys.push_back(sessionCurrentKey);
	//keys.push_back(sessionHistoryKey);
	keys.push_back(sessionQueueKey);
	#endif

	list<CUST::SESSION_INFO> currentSessions; //当前会话
	getCustCurrentSessions(custId,serviceId,currentSessions);
	

	Json::Value pushSessions;
	pushSessions[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;

	Json::Value sessionItem,sessionItems;
	
	if(currentSessions.size() > 0)
	{

	}
	else
	{
		DbgLog("custId[%s] serviceId[%s] cache no currentSessions",custId.c_str(),serviceId.c_str());
		//从service队列中分配当前会话
		dispatchCurrentSessions(custId,serviceId,15,currentSessions);
	}
	
	//push cust当前会话
		list<CUST::SESSION_INFO>::iterator pcurrentUser = currentSessions.begin();
		for( ; pcurrentUser != currentSessions.end(); pcurrentUser++)
		{	
			//CUST_LAST_MSG_INFO lastMsg;
			//m_custMsgMgr.getUserToCustLastMsg(*pcurrentUser,custId,serviceId,lastMsg);
			//Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
			//Json::Value tempVal;
			//JsonParser.parse(*pcurrentUser, tempVal);
			sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_CURRENT;
			sessionItem[CUST_JSON_FIELD_SESSION_OPT] = CUST::OPT_ADD;
			sessionItem[CUST_JSON_FIELD_USERID] = pcurrentUser->userId;
			sessionItem[CUST_JSON_FIELD_NICK_NAME] = pcurrentUser->nickName;
			sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pcurrentUser->headPixel;
			sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pcurrentUser->lastMsgContent;
			sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pcurrentUser->lastMsgTime);
			
			sessionItems.append(sessionItem);
		}
		
	
	list<CUST::SESSION_INFO> queueSessions; //排队会话
	getCustQueueSessions(custId,serviceId,queueSessions);
	
	if(queueSessions.size() > 0)
	{
		

	}
	else
	{
		DbgLog("custId[%s] serviceId[%s] no queueSessions",custId.c_str(),serviceId.c_str());
		//从service队列中分配排队会话
		dispatchQueueSessions(custId,serviceId,15,queueSessions);
	}

	//push cust排队会话
		Json::Value queueSession;
		
		list<CUST::SESSION_INFO>::iterator pqueueUser = queueSessions.begin();
		for( ; pqueueUser != queueSessions.end(); pqueueUser++)
		{	
			//CUST_LAST_MSG_INFO lastMsg;
			//m_custMsgMgr.getUserToCustLastMsg(*pqueueUser,custId,serviceId,lastMsg);
			sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_QUEUE;
			sessionItem[CUST_JSON_FIELD_SESSION_OPT] = CUST::OPT_ADD;
			sessionItem[CUST_JSON_FIELD_USERID] = pqueueUser->userId;
			sessionItem[CUST_JSON_FIELD_NICK_NAME] = pqueueUser->nickName;
			sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pqueueUser->headPixel;
			sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pqueueUser->lastMsgContent;
			sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pqueueUser->lastMsgTime);

			sessionItems.append(sessionItem);
		}

	pushSessions[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;
	send(pushSessions.toStyledString());

	#if 0
	if(false == CRedisMgr::getInstance()->custSessionIsExist(keys,existKeys))
	{
		DbgLog("cust[%s] session cacde not all exit",custId.c_str());
		if(existKeys.size() > 0)
		{
			CRedisMgr::getInstance()->delCustSession(existKeys);
		}

		//--TODO
	}
	else
	{
	
		//加载缓存会话
		list<CUST::SESSION_INFO> currentUsers;
		if(true == CRedisMgr::getInstance()->getCustCurrentUsers(cust_serviceId,currentUsers))
		{	
			if(currentUsers.size() > 0)
			{
				Json::Value currentSession;
				currentSession[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
				currentSession[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_CURRENT;
				
				Json::Value sessionItem,sessionItems;
				
				list<CUST::SESSION_INFO>::iterator pcurrentUser = currentUsers.begin();
				for( ; pcurrentUser != currentUsers.end(); pcurrentUser++)
				{	
					//CUST_LAST_MSG_INFO lastMsg;
					//m_custMsgMgr.getUserToCustLastMsg(*pcurrentUser,custId,serviceId,lastMsg);
					//Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
					//Json::Value tempVal;
					//JsonParser.parse(*pcurrentUser, tempVal);
					
					sessionItem[CUST_JSON_FIELD_USERID] = pcurrentUser->userId;
					sessionItem[CUST_JSON_FIELD_NICK_NAME] = pcurrentUser->nickName;
					sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pcurrentUser->headPixel;
					sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pcurrentUser->lastMsgContent;
					sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pcurrentUser->lastMsgTime);
					
					sessionItems.append(sessionItem);
				}

				currentSession[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;

				//SESSION_PUSH current session to cust
				send(currentSession.toStyledString());
			}
		}

		
		list<CUST::SESSION_INFO> historyUsers;
		if(true == CRedisMgr::getInstance()->getCustHistoryUsers(cust_serviceId,historyUsers))
		{	
			if(historyUsers.size() > 0)
			{
				Json::Value historySession;
				historySession[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
				historySession[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_HISTORY;
				
				Json::Value sessionItem,sessionItems;
				
				list<CUST::SESSION_INFO>::iterator phistoryUser = historyUsers.begin();
				for( ; phistoryUser != historyUsers.end(); phistoryUser++)
				{	
					//CUST_LAST_MSG_INFO lastMsg;
					//m_custMsgMgr.getUserToCustLastMsg(*phistoryUser,custId,serviceId,lastMsg);
					sessionItem[CUST_JSON_FIELD_USERID] = phistoryUser->userId;
					sessionItem[CUST_JSON_FIELD_NICK_NAME] = phistoryUser->nickName;
					sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = phistoryUser->headPixel;
					sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = phistoryUser->lastMsgContent;
					sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(phistoryUser->lastMsgTime);
					
					//Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
					//Json::Value tempVal;
					//JsonParser.parse(*phistoryUser, tempVal);
					sessionItems.append(sessionItem);
				}

				historySession[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;

				//SESSION_PUSH history session to cust
				send(historySession.toStyledString());
			}
			
		}
		
			
		list<CUST::SESSION_INFO> queueUsers;
		if(true == CRedisMgr::getInstance()->getCustQueueUsers(cust_serviceId,queueUsers))
		{	
			if(queueUsers.size() > 0)
			{
				Json::Value queueSession;
				queueSession[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
				queueSession[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_QUEUE;
				
				Json::Value sessionItem,sessionItems;
				
				list<CUST::SESSION_INFO>::iterator pqueueUser = queueUsers.begin();
				for( ; pqueueUser != queueUsers.end(); pqueueUser++)
				{	
					//CUST_LAST_MSG_INFO lastMsg;
					//m_custMsgMgr.getUserToCustLastMsg(*pqueueUser,custId,serviceId,lastMsg);
					sessionItem[CUST_JSON_FIELD_USERID] = pqueueUser->userId;
					sessionItem[CUST_JSON_FIELD_NICK_NAME] = pqueueUser->nickName;
					sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pqueueUser->headPixel;
					sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pqueueUser->lastMsgContent;
					sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pqueueUser->lastMsgTime);

					//Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
					//Json::Value tempVal;
					//JsonParser.parse(*pqueueUser, tempVal);
					sessionItems.append(sessionItem);
				}

				queueSession[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;

				//SESSION_PUSH history session to cust
				send(queueSession.toStyledString());
			}
		}
		#endif
	
}

void CCustWebTask::heartBeatHandler()
{
	send(m_msg.msg.toStyledString().c_str());
}

void CCustWebTask::sessionClose(const string& custId,const string& serviceId,const string& userId)
{
	string cust_serviceId = CUST::packCustBind(custId,serviceId);

	if( CUST::TYPE_CURRENT != m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt())
	{	
		WarnLog("sessionType[%d] not supported ", m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt());
		return;
	}

	string queueUser;
	Json::Value pushSessions;
	pushSessions[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
	Json::Value sessionItem,sessionItems;
	
	if(true == CRedisMgr::getInstance()->delUserFromCustCurrent(cust_serviceId,userId,queueUser))
	{
		//将mongo排队会话移到当前会话
		if(!queueUser.empty())
		{
			CCustWait custWaitRec;
			m_custWaitMgr.deleteCustWait(custId, queueUser, serviceId,custWaitRec);
			m_custProcessedMgr.insertCustProcessed(&custWaitRec);
	
			//PUSH 排队会话->cust当前会话	
			CUST::USER_INFO userInfo;
			getUserInfo(custWaitRec.m_sUserId,userInfo);

			sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_CURRENT;
			sessionItem[CUST_JSON_FIELD_SESSION_OPT] = CUST::OPT_ADD;
			sessionItem[CUST_JSON_FIELD_USERID] = custWaitRec.m_sUserId;
			sessionItem[CUST_JSON_FIELD_NICK_NAME] = userInfo.nickName;
			sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = userInfo.headPixel;
			sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = custWaitRec.m_sLastMsg;
			sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(custWaitRec.m_LastMsgTime);

			sessionItems.append(sessionItem);
			
			 
		}
		
		//从service队列中分配CUST排队会话
		list<CUST::SESSION_INFO> queueSessions;
		dispatchQueueSessions(custId,serviceId,1,queueSessions);

		if(queueSessions.size() > 0)
		{
			//push cust排队会话
			list<CUST::SESSION_INFO>::iterator pqueueUser = queueSessions.begin();
			for( ; pqueueUser != queueSessions.end(); pqueueUser++)
			{	
				//CUST_LAST_MSG_INFO lastMsg;
				//m_custMsgMgr.getUserToCustLastMsg(*pqueueUser,custId,serviceId,lastMsg);
				sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_QUEUE;
				sessionItem[CUST_JSON_FIELD_SESSION_OPT] = CUST::OPT_ADD;
				sessionItem[CUST_JSON_FIELD_USERID] = pqueueUser->userId;
				sessionItem[CUST_JSON_FIELD_NICK_NAME] = pqueueUser->nickName;
				sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pqueueUser->headPixel;
				sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pqueueUser->lastMsgContent;
				sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pqueueUser->lastMsgTime);

				 sessionItems.append(sessionItem);
			}

			
		}
 	
	}

	//remove Close的会话
	sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_CURRENT;
	sessionItem[CUST_JSON_FIELD_SESSION_OPT] = CUST::OPT_REMOVE;
	sessionItem[CUST_JSON_FIELD_USERID] = userId;
	sessionItems.append(sessionItem);

	pushSessions[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;
	
	send(pushSessions.toStyledString());
	
}


void CCustWebTask::sessionCloseHandler()
{
	string custId = m_msg.msg[CUST_JSON_FIELD_CUSTID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	string userId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	
	if( CUST::TYPE_CURRENT != m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt())
	{	
		WarnLog("sessionType[%d] not supported ", m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt());
		return;
	}

	sessionClose(custId, serviceId,userId);
	
}

void CCustWebTask::sessionTransferHandler()
{
	string fromCustId = m_msg.msg[CUST_JSON_FIELD_FROM_CUSTID].asString();
	string fromCustName = m_msg.msg[CUST_JSON_FIELD_FROM_CUSTID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	string toCustId = m_msg.msg[CUST_JSON_FIELD_TO_CUSTID].asString();
	string userId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	string fromCust_serviceId = CUST::packCustBind(fromCustId,serviceId);
	string toCust_serviceId = CUST::packCustBind(toCustId,serviceId);
	uint32_t sessionType = m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt();

	CCustWait fromCustWaitRec;
		
	if( CUST::TYPE_CURRENT != sessionType &&  CUST::TYPE_QUEUE != sessionType)
	{	
		WarnLog("sessionType[%d] not supported ",sessionType);
		return;
	}

	
	//处理fromCust队列
	switch(sessionType)
	{
		case CUST::TYPE_CURRENT:
		{
			sessionClose(fromCustId,serviceId,userId);
			break;
		}
		case CUST::TYPE_QUEUE:
		{
			//将user会话从fromCust排队队列中移除
			m_custWaitMgr.deleteCustWait(fromCustId, userId, serviceId,fromCustWaitRec);

			//从service排队队列中分配一个到fromCust排队队列
			list<CUST::SESSION_INFO> queueSessions;
			dispatchQueueSessions(fromCustId,serviceId,1,queueSessions);
			
			Json::Value pushSessions;
			pushSessions[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
			Json::Value sessionItem,sessionItems;
			
			if(queueSessions.size() > 0)
			{
				//push fromCust排队会话
				list<CUST::SESSION_INFO>::iterator pqueueUser = queueSessions.begin();
				for( ; pqueueUser != queueSessions.end(); pqueueUser++)
				{	
					//CUST_LAST_MSG_INFO lastMsg;
					//m_custMsgMgr.getUserToCustLastMsg(*pqueueUser,custId,serviceId,lastMsg);
					sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_QUEUE;
					sessionItem[CUST_JSON_FIELD_SESSION_OPT]= CUST::OPT_ADD;
					sessionItem[CUST_JSON_FIELD_USERID] = pqueueUser->userId;
					sessionItem[CUST_JSON_FIELD_NICK_NAME] = pqueueUser->nickName;
					sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pqueueUser->headPixel;
					sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pqueueUser->lastMsgContent;
					sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pqueueUser->lastMsgTime);

					sessionItems.append(sessionItem);
				}
			}

			//remove from queue会话
			sessionItem[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_QUEUE;
			sessionItem[CUST_JSON_FIELD_SESSION_OPT]= CUST::OPT_REMOVE;
			sessionItem[CUST_JSON_FIELD_USERID] = userId;
			sessionItems.append(sessionItem);
			
			pushSessions[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;

			send(pushSessions.toStyledString());
			break;
		}
	}

	//处理toCust队列	
	fromCustWaitRec.m_sCustId = toCustId;
	char msg[256];
	snprintf(msg,sizeof(msg),"%s转入一条会话",fromCustName.c_str());
	fromCustWaitRec.m_LastMsgTime = getCurrentTime();
	fromCustWaitRec.m_encrypt = 0;
	fromCustWaitRec.m_sLastMsg = msg;
	
	if(true ==m_custProcessedMgr.insertCustProcessed(fromCustWaitRec))
	{	
		CRedisMgr::GetInstance()->userToCustCurrent(userId,toCustId);

		//push ToCUST 当前会话
		Json::Value currentSession;
		currentSession[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PUSH;
		currentSession[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_CURRENT;
		
		Json::Value sessionItem,sessionItems;
		sessionItem[CUST_JSON_FIELD_USERID] = fromCustWaitRec.m_sUserId;
		//sessionItem[CUST_JSON_FIELD_NICK_NAME] = fromCustWaitRec->nickName;  //获取用户昵称--TODO 
		//sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = fromCustWaitRec->headPixel; //回去用户头像--TODO
		sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = fromCustWaitRec.m_sLastMsg;
		sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(fromCustWaitRec.m_LastMsgTime);

		sessionItems.append(sessionItem);
		currentSession[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;
		sendToCust(toCustId,currentSession.toStyledString());
		
	}
	
}

void CCustWebTask::sessionPullHandler()
{
	string custId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	uint32_t pageInDex = m_msg.msg[CUST_JSON_FIELD_PAGEINDEX].asInt();
	uint32_t pageSize = m_msg.msg[CUST_JSON_FIELD_PAGESIZE].asInt();
	string cust_serviceId = CUST::packCustBind(custId,serviceId);

	if( CUST::TYPE_HISTORY != m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt())
	{	
		WarnLog("sessionType[%d] not supported ", m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt());
	}

	list<CCustProcessed> custProcessedRecs;
	
	
	m_custProcessedMgr.getCustProcessedsByPage(custId,serviceId, pageInDex, pageSize, custProcessedRecs);

	//push cust历史会话
	Json::Value currentSession,sessionItem,sessionItems;;
	currentSession[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_PULL_ACK;
	currentSession[CUST_JSON_FIELD_SESSION_TYPE] = CUST::TYPE_HISTORY;

	uint32_t size = custProcessedRecs.size();
	uint32_t lastPageFlag = size > pageSize? 0:1;

	list<CCustProcessed>::iterator pCustProcessedRec = custProcessedRecs.begin();
	for(uint32_t i =0; pCustProcessedRec != custProcessedRecs.end() && i < pageSize; pCustProcessedRec++)
	{
		sessionItem[CUST_JSON_FIELD_USERID] = pCustProcessedRec->m_sUserId;
		//sessionItem[CUST_JSON_FIELD_NICK_NAME] = pCustProcessedRec->m;
		//sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = pcurrentUser->headPixel;
		sessionItem[CUST_JSON_FIELD_MSG_CONTENT] = pCustProcessedRec->m_sLastMsg;
		sessionItem[CUST_JSON_FIELD_MSG_TIME] = Json::Value::UInt64(pCustProcessedRec->m_LastMsgTime);

		CUST::USER_INFO userInfo;
		getUserInfo(pCustProcessedRec->m_sUserId,userInfo);
		sessionItem[CUST_JSON_FIELD_NICK_NAME] = userInfo.nickName;
		sessionItem[CUST_JSON_FIELD_HEAD_PIXEL] = userInfo.headPixel;
		sessionItems.append(sessionItem);
		
		i++;
	}

	currentSession[CUST_JSON_FIELD_LASTPAGE_FLAG] = lastPageFlag;
	currentSession[CUST_JSON_FIELD_SESSION_LIST] = sessionItems;
	

	
	//SESSION_PUSH current session to cust
	send(currentSession.toStyledString());
			
		
	
}

void CCustWebTask::sessionActiveHander()
{
	if( CUST::TYPE_HISTORY != m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt())
	{	
		WarnLog("sessionType[%d] not supported ", m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt());
	}

	string custId = m_msg.msg[CUST_JSON_FIELD_CUSTID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	string userId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	string cust_serviceId = CUST::packCustBind(custId,serviceId);	

	CRedisMgr::getInstance()->userToCustCurrent(userId,cust_serviceId);

	//回ACK
	Json::Value sessionActiveAck;
	sessionActiveAck[CUST_JSON_FIELD_COMMAND] = CUST::SESSION_ACTIVE_ACK;
	sessionActiveAck[CUST_JSON_FIELD_SESSION_TYPE] = m_msg.msg[CUST_JSON_FIELD_SESSION_TYPE].asInt();
	sessionActiveAck[CUST_JSON_FIELD_CUSTID] = custId;
	sessionActiveAck[CUST_JSON_FIELD_USERID] = userId;
	sessionActiveAck[CUST_JSON_FIELD_SERVICEID] = serviceId;
	sessionActiveAck[CUST_JSON_FIELD_TIMESTAMP] = Json::Value::UInt64(getCurrentTime());

	send(sessionActiveAck.toStyledString());
	
}

void CCustWebTask::updateCustStatus()
{
	string custId = m_msg.msg[CUST_JSON_FIELD_USERID].asString();
	string serviceId = m_msg.msg[CUST_JSON_FIELD_SERVICEID].asString();
	string cust_serviceId = CUST::packCustBind(custId,serviceId);

	CRedisMgr::getInstance()->setCustStatus(cust_serviceId,CUST::USER_STATUS(m_msg.msg[CUST_JSON_FIELD_CUST_STATUS].asInt()));
	
}


bool CCustWebTask::getUserInfo(const string& userId,CUST::USER_INFO& userInfo)
{
	if( false == CRedisMgr::getInstance()->getUserInfo(userId,userInfo) )
	{
		//去php拉取用户信息--TODO
		CRedisMgr::getInstance()->insertUserInfo(userId,userInfo);
	}

	return true;
}


