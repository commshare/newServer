/******************************************************************************
Filename: friendhandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/29
Description: 处理添加，删除，拉黑好友等请求
******************************************************************************/
#ifndef __FRIENDHANDLE_H__
#define __FRIENDHANDLE_H__

#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include "im.push.android.pb.h"

class CImPdu;
class CConfigFileReader;

class CLoginInfo;
namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

class CFriendHandler : public CBaseHandle
{
public:
	CFriendHandler(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CFriendHandler();

//	void OnAddFriend(std::shared_ptr<CImPdu> pPdu);						// 响应添加好友请求
//	void OnAddFriendDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);		//AddFriendDeliver插入mongo后的回调函数
//	void OnAddFriendDeliverAck(std::shared_ptr<CImPdu> pPdu);			// 收到添加好友推送应答

//	void OnAddFriendAns(std::shared_ptr<CImPdu> pPdu);					// 响应添加好友操作结果（同意、拒绝）
//	void OnAddFriendAnsDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);	//AddFriendAnsDeliver插入mongo后的回调函数
//	void OnAddFriendAnsDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

//	void OnDelFriend(std::shared_ptr<CImPdu> pPdu);						// 响应删除好友请求
//	void OnBlockFriend(std::shared_ptr<CImPdu> pPdu);					// 响应拉黑好友请求
//	void OnUnblockFriend(std::shared_ptr<CImPdu> pPdu);					// 从黑名单中拉出好友的请求

	//群组添加（邀请）请求
//	void OnJoinGrp(std::shared_ptr<CImPdu> pPdu);
//	void OnJoinGrpDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);	//AddFriendAnsDeliver插入mongo后的回调函数
//	void OnJoinGrpDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

	////群组添加（邀请）应答
	//void OnJoinGrpAns(std::shared_ptr<CImPdu> pPdu);
	//void OnGroupPermitAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

	//交换key
//	void OnExchangeKey(std::shared_ptr<CImPdu> pPdu);
//	void OnExchangeKeyInserted(const std::vector<std::shared_ptr<COfflineMsg> >& msgs, bool bInsertSuccess, const UidCode_t& sessionID);
//	void OnExchangeKeyDeliverAck(std::shared_ptr<CImPdu> pPdu);
//	void OnExchangeKeyDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess);
//	void OnExchangeKeyDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu);

	//额外任务，透传kickout消息
	void OnKickOut(std::shared_ptr<CImPdu> pPdu);
protected:	
	virtual bool RegistPacketExecutor(void);
//private:
	//bool	updateFriendState(const string& userId, const string& friendId, int32_t state);
	//bool	delFriendState(const string& userId, const string& friendId);
private:
	//COfflineMsgMgr	m_offlineMsgMgr;
	int 			m_nNumberOfInst;
};
#endif // __FRIENDHANDLE_H__


