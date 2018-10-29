/******************************************************************************
Filename: grphandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/29
Description: 
******************************************************************************/
#ifndef __GRPHANDLE_H__
#define __GRPHANDLE_H__

#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"

class CImPdu;

/*处理群组服务器或者移动终端传输有关群组消息*/
class CGrpMsgHandler : public CBaseHandle
{
public:
	CGrpMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CGrpMsgHandler();

	void OnGrpMsgChat(std::shared_ptr<CImPdu> pPdu);						// 响应聊天消息
	void HandleMsgGrpChatTask(const im::_MESGrpInterChat& msg, const UidCode_t& sessionId = UidCode_t());

	void OnGrpMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到消息接收方聊天消息推送应答

	void OnGrpMsgChatCancle(std::shared_ptr<CImPdu> pPdu);						// 响应撤销聊天消息
	void HandleMsgGrpChatCancleTask(const im::_MESGrpInterChatCancel& msg, const UidCode_t& sessionId = UidCode_t());
	//void OnGrpMsgChatCancleDeliverInserted(const COfflineMsg& Msg, bool bUpdateSuccess, bool bNeedSendPush = false);		//msgChatDeliver插入mongo后的回调函数

	void OnGrpMsgChatCancleDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到消息接收方撤销聊天消息推送应答

	void OnGrpNotify(std::shared_ptr<CImPdu> pPdu);
	void HandleMsgGrpNotifyTask(const im::_MESGrpInterNotify& msg, const UidCode_t& sessionId = UidCode_t());
	void OnGrpNotifyDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, bool bNeedSendPush);	
	void OnGrpNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu);
protected:	
	virtual bool RegistPacketExecutor(void);
private:
//	int32_t getFriendState(const string& userId, const string& friendId);
	void sendGrpMsgChatCancelAck(const _MESGrpInterChatCancel& msg, im::ErrCode retCode, const UidCode_t& sessionId);
	void sendGrpMsgChatAck(const _MESGrpInterChat& msg, im::ErrCode retCode, const UidCode_t& sessionId);
private:
	//COfflineMsgMgr m_offlineMsgMgr;
	int 			m_nNumberOfInst;
};
#endif // __GRPHANDLE_H__




