/******************************************************************************
Filename: friendhandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/29
Description: 
******************************************************************************/
#include "configfilereader.h"
#include "friendhandle.h"
#include "im_loginInfo.h"
#include "im.cm.pb.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
//#include "im.group.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "mysqlFriendMgr.h"
#include "redisFriendMgr.h"
#include "redisLoginInfoMgr.h"
#include "redisUserCfgMgr.h"
#include "serverinfo.h"
#include "util.h"

using namespace im;
using namespace std;

CFriendHandler::CFriendHandler(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CFriendHandler::~CFriendHandler()
{
	
}

//首先尝试从redis中获取数据
//获取失败后从mysql获取数据并同步到redis
std::vector<CFriendRelation>  GetFriendShips(const string& userId, const string& friendId)
{
	std::vector<CFriendRelation> relationShips = CReidsFriendMgr::GetFriendRelation(userId, friendId);	//get relationship from redis
	if (/*relationShips.empty()*/relationShips.size() < 2)									//if get from redis failed, try get from mysql and update to redis
	{
		try
		{
			relationShips = CMysqlFriendMgr::GetFriendShips(userId, friendId);
			std::vector<CFriendRelation>::iterator it = relationShips.begin();
			for (; it != relationShips.end(); ++it)
			{
				//如果状态为删除，拒绝，被拒绝，则不需要同步到redis(避免redis占用内存过多)
				if (it->GetState() & (CFriendRelation::FRIEND_RELATION_STATE_DELETED | CFriendRelation::FRIEND_RELATION_STATE_REFUSE | CFriendRelation::FRIEND_RELATION_STATE_REFUSED)) 
					continue;
				CReidsFriendMgr::InsertFriendRelation(*it);
			}
		}
		catch (...)
		{
			return relationShips;		//return what is reasonable
		}
	}
	return relationShips;
}

bool insertFriendShip(const CFriendRelation& friendRelation)
{
	//先删除redis中的记录，删除成功后改写mysql
	bool retCode = CReidsFriendMgr::RemoveFriendRelation(friendRelation.GetUserId(), friendRelation.GetFriendId());
	if (retCode)
	{
		retCode = CMysqlFriendMgr::InsertFriendShip(friendRelation);
	}
	return retCode;
}

bool updateFriendShip(const string& userId, const string& friendId, int state)
{
	return insertFriendShip(CFriendRelation(userId, friendId, state));
}

bool updateFriendShip(const CFriendRelation& friendRelation)
{
	return insertFriendShip(friendRelation);
}

//从数据库中删除好友关系
bool deleteFriendShip(const string& userId, const string& friendId)
{
	bool retCode = CReidsFriendMgr::RemoveFriendRelation(userId, friendId); 
	if (retCode)
		retCode = CMysqlFriendMgr::DelFriendShip(userId, friendId);
	return retCode;
}

bool deleteFriendShip(const CFriendRelation& friendRelation)
{
	return deleteFriendShip(friendRelation.GetUserId(), friendRelation.GetFriendId());
}

//im::ErrCode GetErrorCodeForAddFriend(const string& userId, const string& friendId)
//{
//	std::vector<CFriendRelation> relationShips = GetFriendShips(userId, friendId);	//get relationship from redis
//
//	//no history record,can add friend
//	if (relationShips.empty())
//		return NON_ERR;
//
//	bool bPeerAdd = false;
//	bool bLocateAdd = false;
//	//if added before, check whether can add friend again
//	std::vector<CFriendRelation>::iterator it = relationShips.begin();
//	for (; it != relationShips.end(); ++it)
//	{
//		if (it->GetUserId() == userId && it->GetFriendId() == friendId )		//check whether this item is user --> friend
//		{
//			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_ADDING)) return ERR_CHAT_FRIEND_ADDING;
//			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL))
//			{
//				bLocateAdd = true;
//			}
//		}
//		else if (it->GetUserId() == friendId && it->GetFriendId() == userId)	//peer's relation(friend-->user)
//		{
//			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_BLOCKED)) return ERR_CHAT_FRIEND_BLOCK;
//			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL))
//			{
//				bPeerAdded = true;
//			}
//		}
//	}
//	if (bLocateAdd && bPeerAdded)	//双方已互加则显示已添加
//	{
//		return ERR_CHAT_FRIEND_ADDED;
//	}
//
//	return NON_ERR;
//}

im::ErrCode GetErrorCodeForBlockFriend(const string& userId, const string& friendId)
{
	std::vector<CFriendRelation> relationShips = GetFriendShips(userId, friendId);	//get relationship from redis

	//if added before, check whether can add friend again
	std::vector<CFriendRelation>::iterator it = relationShips.begin();
	for (; it != relationShips.end(); ++it)
	{
		if (it->GetUserId() == userId && it->GetFriendId() == friendId)		//check whether this item is user --> friend
		{
			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_BLOCKED)) return ERR_CHAT_FRIEND_BLOCK;
			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL)) return NON_ERR;
		}
	}

	return ERR_CHAT_FORBIDDEN;
}


im::ErrCode GetErrorCodeForUnblockFriend(const string& userId, const string& friendId)
{
	std::vector<CFriendRelation> relationShips = GetFriendShips(userId, friendId);	//get relationship from redis

	//if added before, check whether can add friend again
	std::vector<CFriendRelation>::iterator it = relationShips.begin();
	for (; it != relationShips.end(); ++it)
	{
		if (it->GetUserId() == userId && it->GetFriendId() == friendId)		//check whether this item is user --> friend
		{
			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_BLOCKED)) return NON_ERR;
		}
	}

	return ERR_CHAT_FORBIDDEN;
}

/*
void AddFriendDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short result, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CFriendHandler* pHandle = (CFriendHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg= dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnAddFriendDeliverInserted(*pMsg, result, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}

void CFriendHandler::OnAddFriend(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriend msg;

	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addfriend(0x%x) %s received %s:%s, %s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.sselfintroduce().c_str());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	msg.set_msgtime(getCurrentTime());//set server time
	
	im::ErrCode retCode = ERR_CHAT_FRIEND_ADDING;
	const string& userId = msg.sfromid();
	const string& friendId = msg.stoid();
	std::vector<CFriendRelation> relationShips = GetFriendShips(userId, friendId);	//get relationship from redis
	bool bLocateAdd = false;		//本端是否已经添加对端
	bool bPeerAdded = false;		//对端是否已经添加本端

	bool isHiden = false;			// 本端已添加对端,对端是否在隐藏模式

	//如果之前已经有联系人记录，判断是否可以再次添加
	if (!relationShips.empty())		
	{
		std::vector<CFriendRelation>::iterator it = relationShips.begin();
		for (; it != relationShips.end(); ++it)
		{
			if (it->GetUserId() == userId && it->GetFriendId() == friendId)		//查看本端当前状态 user --> friend
			{
				//if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_ADDING))
				//{
				//	retCode = ERR_CHAT_FRIEND_ADDING;
				//	break;
				//}
				if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL))
				{
					bLocateAdd = true;
					if(it->IsHidenModel())
						isHiden = true;
				}
			}
			else if (it->GetUserId() == friendId && it->GetFriendId() == userId)	//查看对端当前状态(friend-->user)
			{
				if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_BLOCKED))
				{
					retCode = ERR_CHAT_FRIEND_BLOCK;
					break;
				}
				if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL))
				{
					bPeerAdded = true;
				}
			}
		}
		if (bLocateAdd && bPeerAdded)	//双方已互加则显示已添加
		{
			retCode = ERR_CHAT_FRIEND_ADDED;
			if(isHiden) 
				retCode = ERR_CHAT_FRIEND_HIDENADDED;
		}
	}

	//如果有权限添加，并且本端-->对端还不是好友(例如：本端 已删除 对方，已拉黑 对方，未添加)
	if (ERR_CHAT_FRIEND_ADDING == retCode && !bLocateAdd)	
	{
		CFriendRelation friendRelation(msg);
		if (bPeerAdded)	//如果对端已经和本端是好友，则自动成为好友关系
		{
			friendRelation.SetState(CFriendRelation::FRIEND_RELATION_STATE_NORMAL);
			retCode = NON_ERR;
		}
		//将好友关系写入数据库
		retCode = insertFriendShip(friendRelation) ? retCode : EXCEPT_ERR;		
	}

	//如果本端有权限添加，已经修改状态成功，并且对端当前还不是本端好友
	if (ERR_CHAT_FRIEND_ADDING == retCode && !bPeerAdded)
	{
		if(bLocateAdd)		// 本端已是好友关系,更新
		{
			CFriendRelation friendRelation(msg);
			friendRelation.SetState(CFriendRelation::FRIEND_RELATION_STATE_NORMAL);
			if(!insertFriendShip(friendRelation))
				WarnLog("update local friend ship to db fail");
		}
		//添加或修改对端的好友状态到数据库（对端已拉黑本端的情况不会到这一步）
		retCode = insertFriendShip(CFriendRelation(msg.stoid(), msg.sfromid(), CFriendRelation::FRIEND_RELATION_STATE_VERIFYING, 1,0,0, msg.sselfintroduce())) ? retCode : EXCEPT_ERR;
	}

	// if can't add friend , return 
	if (retCode != ERR_CHAT_FRIEND_ADDING)
	{
		//send addFriendAck to sender
		MESAddFriendAck addFndAck;
		addFndAck.set_smsgid(msg.smsgid());
		addFndAck.set_suserid(msg.sfromid());
		addFndAck.set_errcode(retCode);
		sendAck(&addFndAck, MES_ADDFRIEND_ACK, pPdu->GetSessionId());
		log("****send MESAddFriendAck(0x%x) %s to %s sent error 0x%x", MES_ADDFRIEND_ACK,
			addFndAck.smsgid().c_str(), addFndAck.suserid().c_str(), retCode);
		return;
	}

	//if receiver was online, send addFriendDeliver to receiver
	bool bneedSendPush = true;
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
	if (pLogin && pLogin->IsLogin())
	{
		sendReq(&msg, MES_ADDFRIEND_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		bneedSendPush = false;
		log("****send MES_ADDFRIEND_DELIVER(0x%x) %s to %s sent", MES_ADDFRIEND_DELIVER, 
			msg.smsgid().c_str(), msg.stoid().c_str());
	}

	//save addFriendDeliver msg to mongodb, if insert failed ,can try to send deliver
	msg.set_sdesc("");
	msg.set_smemoname("");
	msg.set_packetid(-1);
	m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), AddFriendDeliverInsertedCallBack, 
					new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId(), bneedSendPush));

}

void CFriendHandler::OnAddFriendDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
{
	MESAddFriendAck addFndAck;
	addFndAck.set_smsgid(Msg.GetMsgId());
	addFndAck.set_suserid(Msg.GetFromId());
    addFndAck.set_errcode((bInsertSuccess & MONGO_OPERATION_SUCCESS) ? ERR_CHAT_FRIEND_ADDING : EXCEPT_ERR);
	sendAck(&addFndAck, MES_ADDFRIEND_ACK, sessionID);

	log("****send MESAddFriendAck(0x%x) %s to %s sent error 0x%x", MES_ADDFRIEND_ACK,
		addFndAck.smsgid().c_str(), addFndAck.suserid().c_str(), addFndAck.errcode());

    //bNeedSendPush 
    //(bInsertSuccess & MONGO_OPERATION_SUCCESS) 
    //!(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE) 
    bNeedSendPush = bNeedSendPush && ((bInsertSuccess & MONGO_OPERATION_SUCCESS) && !(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE));

    if (bNeedSendPush)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
		if (pLogin)
		{
			// 需要修改，调PHP接口
			const std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(Msg.GetToId());
			if (!pUserCfg->IsGlobalNoInterruption())
			{
				//你有一条新请求！
				sendPush(pLogin, Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), CONTACTS, NewRequestStr );
			}
		}		
	}
}

void CFriendHandler::OnAddFriendDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriendDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addFriendDeliverAck(0x%x) %s received", pPdu->GetCommandId(), msg.smsgid().c_str());
	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	//delete addfriendDeliver msg from mongo 
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid() ,MES_ADDFRIEND_DELIVER, msg.smsgid());
}

void AddFriendAnsDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CFriendHandler* pHandle = (CFriendHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnAddFriendAnsDeliverInserted(*pMsg, bInsertSuccess, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}

void CFriendHandler::OnAddFriendAns(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriendAns msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addfriendAns(0x%x) %s received, dir(%s-->%s),ans = %s, code = 0x%x", pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.sans().c_str(), msg.errcode());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

//	if (!msg.msgtime())
//	{
//        msg.set_msgtime(getCurrentTime());
//	}
    //以服务器时间为准
    msg.set_msgtime(getCurrentTime());//set server time

	//获取之前的记录
	std::vector<CFriendRelation> relationShips;
	try
	{
		relationShips = CMysqlFriendMgr::GetFriendShips(msg.stoid(), msg.sfromid());
	}
	catch (...)
	{

	}


	im::ErrCode retCode = NON_ERR;
	if (relationShips.size() < 2)
	{
		retCode = EXCEPT_ERR;
	}

	if (NON_ERR == retCode)
	{
		bool isHiden = false;
		retCode = ERR_CHAT_FRIEND_ADDED;
		for(auto& it : relationShips)
		{
			if(it.GetState() != CFriendRelation::FRIEND_RELATION_STATE_NORMAL)
			{
				retCode = NON_ERR;
				break;
			}

			// 好友请求回应时，看fromid是否在toid的隐藏模式下
			if(it.GetUserId() == msg.stoid() && it.IsHidenModel())
			{
				isHiden = true;
		 	}
		}
		if(retCode == ERR_CHAT_FRIEND_ADDED && isHiden)
			retCode = ERR_CHAT_FRIEND_HIDENADDED;

		log("add friend ans code:%d", retCode);
		if(retCode == NON_ERR)
		{
			const bool bAgree = (NON_ERR == msg.errcode());
			//update new state to db(mysql and redis)
			if (bAgree)
			{
				const int32_t state = CFriendRelation::FRIEND_RELATION_STATE_NORMAL;
				std::vector<CFriendRelation>::iterator it = relationShips.begin();
				for (; it != relationShips.end(); ++it)
				{
					it->SetState(state);
					if(!updateFriendShip(*it))
					{
						retCode = EXCEPT_ERR;
						break;
					}
				}
				
			}
			else //if refuse
			{
				
				//only update state to mysql
				std::vector<CFriendRelation>::iterator it = relationShips.begin();
				for (; it != relationShips.end(); ++it)
				{
					if(it->GetUserId() == msg.stoid())
					{
						//添加好友请求的发起方，（保留本端验证对方添加好友请求的状态，用在两边互加的情况）
						const int32_t state = CFriendRelation::FRIEND_RELATION_STATE_REFUSED;
						//if(!updateFriendShip(msg.stoid(), msg.sfromid(), ((it->GetState() & CFriendRelation::FRIEND_RELATION_STATE_VERIFYING) | state)))
						if(!updateFriendShip(msg.stoid(), msg.sfromid(),  state))
						{
							retCode = EXCEPT_ERR;
							break;
						}
					}
					else if(it->GetUserId() == msg.sfromid())
					{
						//添加好友请求的接收方，（保留本端添加对方的状态，用在两边互加的情况）
						const int32_t state = CFriendRelation::FRIEND_RELATION_STATE_REFUSE;
						//const int32_t newState = (it->GetState() & ~CFriendRelation::FRIEND_RELATION_STATE_VERIFYING) | state;
						//if (newState != state)
						//{
						//	if(!updateFriendShip(msg.sfromid(), msg.stoid(), newState))
						//	{
						//		retCode = EXCEPT_ERR;
						//		break;
						//	}
						//}
						//else
						//{
						//	deleteFriendShip(msg.sfromid(), msg.stoid());
						//}
						if(!updateFriendShip(msg.sfromid(), msg.stoid(), state))
						{
							retCode = EXCEPT_ERR;
							break;
						}

					}
				}
			}
		}
	}

	if (retCode != NON_ERR)
	{
		//send addFriendAnsAck to sender
		MESAddFriendAnsAck addFndAnsAck;
		addFndAnsAck.set_smsgid(msg.smsgid());
		addFndAnsAck.set_suserid(msg.sfromid());
		addFndAnsAck.set_errcode(retCode );
		sendAck(&addFndAnsAck, MES_ADDFRIEND_ANS_ACK, pPdu->GetSessionId());
		log("****send MESAddFriendAnsAck(0x%x) %s to %s sent,ret Code = 0x%x", MES_ADDFRIEND_ANS_ACK,
			addFndAnsAck.smsgid().c_str(), addFndAnsAck.suserid().c_str(), retCode);
		return;
	}

	bool bneedSendPush = (NON_ERR == msg.errcode());		//同意才发
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
	if (pLogin && pLogin->IsLogin())	//if receiver was online, send addFriendDeliver to receiver
	{
		sendReq(&msg, MES_ADDFRIEND_ANS_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		bneedSendPush = false;
		log("****send MES_ADDFRIEND_ANS_DELIVER(0x%x) %s to %s sent", 
			MES_ADDFRIEND_ANS_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str());
	}

	m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), AddFriendAnsDeliverInsertedCallBack, 
		new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId(), bneedSendPush));
}


void CFriendHandler::OnAddFriendAnsDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
{
	MESAddFriendAnsAck addFndAnsAck;
	addFndAnsAck.set_smsgid(Msg.GetMsgId());
	addFndAnsAck.set_suserid(Msg.GetFromId());
    addFndAnsAck.set_errcode((bInsertSuccess & MONGO_OPERATION_SUCCESS) ? NON_ERR : EXCEPT_ERR);
	sendAck(&addFndAnsAck, MES_ADDFRIEND_ANS_ACK, sessionID);
	log("****send MESAddFriendAnsAck(0x%x) %s to %s sent", MES_ADDFRIEND_ANS_ACK,
		addFndAnsAck.smsgid().c_str(), addFndAnsAck.suserid().c_str());

		bNeedSendPush = bNeedSendPush && ((bInsertSuccess & MONGO_OPERATION_SUCCESS) && !(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE));
    if (bNeedSendPush)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
		if (pLogin)
		{
			// 需要修改，调PHP接口
			const std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(Msg.GetToId());
			if (!pUserCfg->IsGlobalNoInterruption())
			{
				sendPush(pLogin, Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), CONTACTS, NewMsgStr );
			}
		}	
	}
}

void CFriendHandler::OnAddFriendAnsDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriendAnsDeliverACK msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addFriendAnsDeliverAck(0x%x) %s received", pPdu->GetCommandId(), msg.smsgid().c_str());

	if (msg.suserid().empty() || msg.smsgid().empty() )
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	//delete addfriendDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_ADDFRIEND_ANS_DELIVER, msg.smsgid());
}

void CFriendHandler::OnDelFriend(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESDelFriend msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("delfriend(0x%x) %s received, %s delfriend %s\r\n", pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	im::ErrCode retCode =
			updateFriendShip(msg.sfromid(), msg.stoid(), CFriendRelation::FRIEND_RELATION_STATE_DELETED) ? NON_ERR : EXCEPT_ERR;

	//send addFriendAck to sender
	MESDelFriendAck delFndAck;
	delFndAck.set_smsgid(msg.smsgid());
	delFndAck.set_suserid(msg.sfromid());
	delFndAck.set_errcode(retCode);

	sendAck(&delFndAck, MES_DELFRIEND_ACK, pPdu->GetSessionId());
	log("****send MESDelFriendAck(0x%x) %s to %s, code = 0x%x", MES_DELFRIEND_ACK, 
		delFndAck.smsgid().c_str(), delFndAck.suserid().c_str(), delFndAck.errcode());
}

void CFriendHandler::OnBlockFriend(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESIncBlacklist msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("blockfriend(0x%x) %s received, %s block %s", pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	//check whether is friend now
	im::ErrCode retCode = GetErrorCodeForBlockFriend(msg.sfromid(), msg.stoid());

	if (NON_ERR == retCode)
	{
		retCode =
			updateFriendShip(msg.sfromid(), msg.stoid(), CFriendRelation::FRIEND_RELATION_STATE_BLOCKED) ? NON_ERR : EXCEPT_ERR;
	}
	//send blockFndAck to sender
	MESIncBlacklistAck blockFndAck;
	blockFndAck.set_smsgid(msg.smsgid());
	blockFndAck.set_suserid(msg.sfromid());
	blockFndAck.set_errcode(retCode);
	sendAck(&blockFndAck, MES_INCBLACKLIST_ACK, pPdu->GetSessionId());
	log("****send blockFriendAck(0x%x) %s to %s, code = 0x%x", MES_INCBLACKLIST_ACK,
		blockFndAck.smsgid().c_str(), blockFndAck.suserid().c_str(), retCode);
}

void CFriendHandler::OnUnblockFriend(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESDecBlacklist msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("unblockfriend(0x%x) %s received, %s unblock %s", pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	im::ErrCode retCode = GetErrorCodeForUnblockFriend(msg.sfromid(), msg.stoid());
	if (NON_ERR == retCode)
	{
		retCode = updateFriendShip(msg.sfromid(), msg.stoid(), CFriendRelation::FRIEND_RELATION_STATE_NORMAL) ? NON_ERR : EXCEPT_ERR;
	}
	//send addFriendAck to sender
	MESDecBlacklistAck unblockFndAck;
	unblockFndAck.set_smsgid(msg.smsgid());
	unblockFndAck.set_suserid(msg.sfromid());
	unblockFndAck.set_errcode(retCode);
	sendAck(&unblockFndAck, MES_DECBLACKLIST_ACK, pPdu->GetSessionId());
	log("****send unblockFriendAck(0x%x) %s to %s, code = 0x%x", MES_DECBLACKLIST_ACK,
		unblockFndAck.smsgid().c_str(), unblockFndAck.suserid().c_str(), unblockFndAck.errcode());

}
*/

bool CFriendHandler::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
//	CmdRegist(MES_ADDFRIEND,				m_nNumberOfInst,  CommandProc(&CFriendHandler::OnAddFriend));
//	CmdRegist(MES_ADDFRIEND_DELIVER_ACK,	m_nNumberOfInst,  CommandProc(&CFriendHandler::OnAddFriendDeliverAck));
//	CmdRegist(MES_ADDFRIEND_ANS,			m_nNumberOfInst,  CommandProc(&CFriendHandler::OnAddFriendAns));
//	CmdRegist(MES_ADDFRIEND_ANS_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CFriendHandler::OnAddFriendAnsDeliverAck));
//	CmdRegist(MES_DELFRIEND,				m_nNumberOfInst,  CommandProc(&CFriendHandler::OnDelFriend));
//	CmdRegist(MES_INCBLACKLIST,				m_nNumberOfInst,  CommandProc(&CFriendHandler::OnBlockFriend));
//	CmdRegist(MES_DECBLACKLIST,				m_nNumberOfInst,  CommandProc(&CFriendHandler::OnUnblockFriend));

//	CmdRegist(MES_JOINGRP,					m_nNumberOfInst,  CommandProc(&CFriendHandler::OnJoinGrp));
//	CmdRegist(MES_JOINGRP_DELIVER_ACK,		m_nNumberOfInst,  CommandProc(&CFriendHandler::OnJoinGrpDeliverAck));
	
	//CmdRegist(MES_JOINGRP_ANS,			m_nNumberOfInst,  CommandProc(&CFriendHandler::OnJoinGrpAns));
	//CmdRegist(GROUP_PERMIT_ACK,			m_nNumberOfInst,  CommandProc(&CFriendHandler::OnGroupPermitAck));

//	CmdRegist(MES_EXCHANGE_KEY,				m_nNumberOfInst,  CommandProc(&CFriendHandler::OnExchangeKey));
//	CmdRegist(MES_EXCHANGE_KEY_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CFriendHandler::OnExchangeKeyDeliverAck));
//	CmdRegist(MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK, m_nNumberOfInst,  CommandProc(&CFriendHandler::OnExchangeKeyDeliverNotifyAck));

//	CmdRegist(CM_KICKOUT_NOTIFICATION,		m_nNumberOfInst,  CommandProc(&CFriendHandler::OnKickOut));
	return true;
}

//bool CFriendHandler::updateFriendState(const string& userId, const string& friendId, int32_t state)
//{
//	bool retCode = CMysqlFriendMgr::UpdateFriendShipState(userId, friendId, state);
//	if (retCode)
//	{
//		if (state & (CFriendRelation::FRIEND_RELATION_STATE_DELETED | CFriendRelation::FRIEND_RELATION_STATE_REFUSE | CFriendRelation::FRIEND_RELATION_STATE_REFUSED))
//			retCode = CReidsFriendMgr::RemoveFriendRelation(userId, friendId);
//		else
//			retCode = CReidsFriendMgr::UpdateFriendShipState(userId, friendId, state);
//	}
//	return retCode;
//}

//bool CFriendHandler::delFriendState(const string& userId, const string& friendId)
//{
//	bool retCode = CMysqlFriendMgr::DelFriendShip(userId, friendId);
//	if (retCode)
//		retCode = CReidsFriendMgr::RemoveFriendRelation(userId, friendId);
//	return retCode;
//}

void CFriendHandler::OnKickOut(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	CMKickoutNotification msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("kickout received, userId;%s\r\n", msg.suserid().c_str());
	int retCode = SendPdu(msg.ip(), msg.port(), pPdu.get());
	if (retCode <= 0)
	{
		WarnLog("!!!send kickOut to %s:%p for user %s failed, return %d",
			msg.ip().c_str(), msg.port(), msg.suserid().c_str(), retCode);
	}
}

//void JoinGrpDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short result, void* paras)
//{
//	if (NULL == paras) return;
//	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

//	CFriendHandler* pHandle = (CFriendHandler*)pCallBackPara->m_handle;
//	if (NULL == pHandle)
//	{
//		delete pCallBackPara;
//		return;
//	}
//	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
//	pHandle->OnJoinGrpDeliverInserted(*pMsg, result, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

//	delete pCallBackPara;		//回调函数负责释放资源
//}


//void CFriendHandler::OnJoinGrp(std::shared_ptr<CImPdu> pPdu)
//{
//	assert(NULL != pPdu);
//	MESJoinGrp msg;

//	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
//	{
//		return;
//	}
//	log("joinGrp(0x%x) %s received %s(%s)-->%s", pPdu->GetCommandId() /*MES_JOINGRP*/, msg.smsgid().c_str(), msg.sgrpid().c_str(),
//			msg.sfromid().c_str(), msg.stoid().c_str(), msg.sselfintroduce().c_str());

//	//if receiver was online, send addFriendDeliver to receiver
//	bool bneedSendPush = true;
//	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
//	if (pLogin && pLogin->IsLogin())
//	{
//		sendReq(&msg, MES_JOINGRP_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
//		bneedSendPush = false;
//		log("****send MES_JOINGRP_DELIVER(0x%x) %s to %s", MES_JOINGRP_DELIVER,
//			msg.smsgid().c_str(), msg.stoid().c_str());
//	}

	//save addFriendDeliver msg to mongodb, if insert failed ,can try to send deliver
//	m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), JoinGrpDeliverInsertedCallBack,
//		new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId(), bneedSendPush));
//}

//void CFriendHandler::OnJoinGrpDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
//{
//	MESJoinGrpAck joinGrpAck;
//	joinGrpAck.set_smsgid(Msg.GetMsgId());
//	joinGrpAck.set_suserid(Msg.GetFromGrpUserId());
//	joinGrpAck.set_sgrpid(Msg.GetFromId());

//	joinGrpAck.set_errcode((bInsertSuccess & MONGO_OPERATION_SUCCESS) ? NON_ERR : EXCEPT_ERR);
//	sendAck(&joinGrpAck, MES_JOINGRP_ACK, sessionID);

//	if (joinGrpAck.errcode() != NON_ERR)
//	{
//		WarnLog("****send MESAddFriendAck(0x%x) %s to %s sent, errorCode = %d", MES_JOINGRP_ACK,
//			joinGrpAck.smsgid().c_str(), joinGrpAck.suserid().c_str(), joinGrpAck.errcode());
//	}

//	bNeedSendPush = bNeedSendPush &&
//			((bInsertSuccess & MONGO_OPERATION_SUCCESS) &&
//			!(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE));

//	if (bNeedSendPush)
//	{
//		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
//		if (pLogin)
//		{
//			const std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(Msg.GetToId());
//			if (!pUserCfg->IsGlobalNoInterruption())
//			{
//				sendPush(pLogin, Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), GRP_CONTACTS, NewRequestStr);
//			}
//		}
//		
//	}
//}

//void CFriendHandler::OnJoinGrpDeliverAck(std::shared_ptr<CImPdu> pPdu)
//{
//	assert(NULL != pPdu);
//	MESJoinGrpDeliverAck msg;
//	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
//	{
//		return;
//	}
//	DbgLog("JoinGrpDeliverAck(0x%x) %s received", pPdu->GetCommandId()/*MES_JOINGRP_DELIVER_ACK*/, msg.smsgid().c_str());

	//delete addfriendDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
//	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_JOINGRP_DELIVER, msg.smsgid());
//}

#if 0
void ExchangeKeyInsertedCallBack(const std::vector<std::shared_ptr<IMongoDataEntry> >& msgs, bool result, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CFriendHandler* pHandle = (CFriendHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	std::vector<std::shared_ptr<COfflineMsg> > offMsgs;
	for (unsigned int i = 0; i < msgs.size(); ++i)
	{
		offMsgs.push_back(dynamic_pointer_cast<COfflineMsg>(msgs[i]));
	}
	pHandle->OnExchangeKeyInserted(offMsgs, result, pCallBackPara->m_sessionID);

	delete pCallBackPara;		//回调函数负责释放资源
}

#define CHANGE_GRP_KEY 0x00
void CFriendHandler::OnExchangeKey(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESExchangeKey msg;

	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("exchangeKey(0x%x) %s received %s-->grp:%s", pPdu->GetCommandId()/*MES_EXCHANGE_KEY*/, msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());
//	if (!msg.msgtime())
//	{
//		msg.set_msgtime(getCurrentTime());
//	}
    msg.set_msgtime(getCurrentTime());//set server time

	int userNum = msg.lsuserkeys_size();
	std::vector<CFriendRelation> friends;
	if ((msg.exchangemode() & 0x10))	//如果是单人的
	{
		friends = CMysqlFriendMgr::GetFriendShips(msg.sfromid());
		userNum = friends.size();
	}

	if (0 == userNum)
	{
		MESExchangeKeyAck exchangeKeyAck;
		exchangeKeyAck.set_smsgid(msg.smsgid());
		exchangeKeyAck.set_suserid(msg.sfromid());
		exchangeKeyAck.set_sgrpid(msg.sgrpid());
		exchangeKeyAck.set_errcode(ERR_GROUP_PARAMETER);
		sendAck(&exchangeKeyAck, MES_EXCHANGE_KEY_ACK, pPdu->GetSessionId());
		WarnLog("!!xkey userKey size = 0");
		return;
	}

	MESExchangeKeyDeliver exchangeKeyDeliver;
	exchangeKeyDeliver.set_sfromid(msg.sfromid());
	exchangeKeyDeliver.set_sgrpid(msg.sgrpid());
	exchangeKeyDeliver.set_smsgid(msg.smsgid());
	exchangeKeyDeliver.set_exchangemode(msg.exchangemode());
	exchangeKeyDeliver.set_encrypt(msg.encrypt());
	exchangeKeyDeliver.set_msgtime(msg.msgtime());
	if (!msg.sopruserid().empty())
	{
		exchangeKeyDeliver.set_sopruserid(msg.sopruserid());
	}

	std::vector<COfflineMsg> msgs;
	
	if (CHANGE_GRP_KEY == (msg.exchangemode() & 0x10))
	{
		for (int index = 0; index < userNum; ++index)
		{
			const UserKey& userKey = msg.lsuserkeys(index);
			exchangeKeyDeliver.set_stoid(userKey.suserid());
			exchangeKeyDeliver.set_skey(userKey.skey());
			msgs.push_back(COfflineMsg(exchangeKeyDeliver));
		}
	}
	else
	{
		for (unsigned int index = 0; index < friends.size(); ++index)
		{
			exchangeKeyDeliver.set_stoid(friends[index].GetFriendId());
			msgs.push_back(COfflineMsg(exchangeKeyDeliver));
		}
	}
	
	m_offlineMsgMgr.InsertOfflineMsg(msgs, ExchangeKeyInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
}

void CFriendHandler::OnExchangeKeyInserted(const std::vector<std::shared_ptr<COfflineMsg> >& msgs, bool bInsertSuccess, const UidCode_t& sessionID)
{
	//send ACK first
	const std::shared_ptr<COfflineMsg>& firstUser = msgs.front();
	MESExchangeKeyAck exchangeKeyAck;
	exchangeKeyAck.set_smsgid(firstUser->GetMsgId());
	exchangeKeyAck.set_suserid(firstUser->GetFromGrpUserId());
	exchangeKeyAck.set_sgrpid(firstUser->GetFromId());
	exchangeKeyAck.set_errcode(bInsertSuccess ? NON_ERR : EXCEPT_ERR);
	sendAck(&exchangeKeyAck, MES_EXCHANGE_KEY_ACK, sessionID);

	log("****send MESExchangeKeyAck(0x%x) %s to %s , errCode = 0x%x", MES_EXCHANGE_KEY_ACK,
		exchangeKeyAck.smsgid().c_str(), exchangeKeyAck.suserid().c_str(), exchangeKeyAck.errcode());
	if (!bInsertSuccess)
	{
		return;
	}

	//send exchangeKeyDeliver to grp members which is online
	std::vector<std::shared_ptr<COfflineMsg> >::const_iterator  iter= msgs.begin();
	std::vector<std::shared_ptr<COfflineMsg> >::const_iterator  endIter = msgs.end();
	for (; iter != endIter; ++iter)
	{
		MESExchangeKeyDeliver exchangeKeyDeliver;
		if (!exchangeKeyDeliver.ParseFromString((*iter)->GetMsgData()))
		{
			WarnLog("get exchangeKeyDeliver failed,msgId = %s, from %s:%s-->%s",
				(*iter)->GetMsgId().c_str(), (*iter)->GetFromId().c_str(), (*iter)->GetFromGrpUserId().c_str(), (*iter)->GetToId().c_str());
			continue;
		}
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo((*iter)->GetToId());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&exchangeKeyDeliver, MES_EXCHANGE_KEY_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****send MES_EXCHANGE_KEY_DELIVER(0x%x) %s from %s:%s to %s", MES_EXCHANGE_KEY_DELIVER,
				exchangeKeyDeliver.smsgid().c_str(), exchangeKeyDeliver.sgrpid().c_str(), exchangeKeyDeliver.sfromid().c_str(), exchangeKeyDeliver.stoid().c_str());
		}
	}
	
}

void ExchangeKeyDeliverNotifyInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
	CFriendHandler* pHandle = (CFriendHandler*)paras;
	if (pHandle)
	{
		const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
		pHandle->OnExchangeKeyDeliverNotifyInserted(*pMsg, bInsertSuccess);
	}
}

void CFriendHandler::OnExchangeKeyDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESExchangeKeyDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    msg.set_msgtime(getCurrentTime());//set server time
	log("MESExchangeKeyDeliverACK(0x%x) %s received,dir %s:%s --> %s",
		pPdu->GetCommandId()/*MES_EXCHANGE_KEY_DELIVER_ACK*/, msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	//if receiver was online, send addFriendDeliver to receiver
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
	if (pLogin && pLogin->IsLogin())
	{
		sendReq(&msg, MES_EXCHANGE_KEY_DELIVERD_NOTIFY, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		log("****send MES_EXCHANGE_KEY_DELIVERD_NOTIFY(0x%x) %s to %s:%s",
			MES_EXCHANGE_KEY_DELIVERD_NOTIFY, msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.stoid().c_str());
	}

	//!!!!!!!!!!!!!!!!!这里后续通过添加exchangeMode字段判断
	if (msg.sgrpid().empty() && msg.skey().empty())
		m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_EXCHANGE_KEY_DELIVER, msg.smsgid(),NULL,NULL);
	else
		m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), ExchangeKeyDeliverNotifyInsertedCallBack, this);
}

void CFriendHandler::OnExchangeKeyDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess)
{
	//if ExchangeKeyDeliverNotify Inserted success, remove ExchangeKeyDeliver
	if (bInsertSuccess)
	{
		m_offlineMsgMgr.DelOfflineMsg(offlineMsg.GetFromGrpUserId(), MES_EXCHANGE_KEY_DELIVER, offlineMsg.GetMsgId());
	}
}

void CFriendHandler::OnExchangeKeyDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESExchangeKeyDeliverNotifyAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("MESExchangeKeyDeliverNotifyAck(0x%x) %s received",
		pPdu->GetCommandId()/*MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK*/, msg.smsgid().c_str());

	//delete addfriendDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_EXCHANGE_KEY_DELIVERD_NOTIFY, msg.smsgid());
}
#endif

