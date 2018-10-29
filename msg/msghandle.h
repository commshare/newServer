/******************************************************************************
Filename: msghandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/07
Description: 处理聊天消息的处理（在线）
******************************************************************************/
#ifndef __MSGHANDLE_H__
#define __MSGHANDLE_H__

#include "basehandle.h"
#include "offlineMsg.h"
#include "im.push.android.pb.h"
#include "offlineMsgMgr.h"

class CImPdu;
class CConfigFileReader;
class CLoginInfo;

class CMsgHandler : public CBaseHandle
{
public:
	CMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CMsgHandler();

	void OnMsgChat(std::shared_ptr<CImPdu> pPdu);						// 初步处理聊天消息
	void HandleMsgChatTask(const im::MESChat& msg, const UidCode_t& sessionId);											// 实际响应聊天消息
	

	void OnMsgChatCancel(std::shared_ptr<CImPdu> pPdu);						// 初步处理聊天消息
	void HandleMsgChatCancelTask(const im::MESChatCancel& msg, const UidCode_t& sessionId);											// 实际响应聊天消息

	void onMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到消息接收方聊天消息推送应答
	void HandleMsgChatDeliverAckTask(const im::MESChatDeliveredAck& msg);	
	//void OnMsgChatDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess);

	//void HandleMsgChatDeliverNotifAckTask(const im::MESChatDeliveredNotificationAck& msg);
	void onMsgChatDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu);		// 收到消息发送方聊天消息送达通知应答

	void HandleMsgChatReadTask(const im::MESChatRead& msg, const UidCode_t& sessionId);
	void OnMsgRead(std::shared_ptr<CImPdu> pPdu);						// 收到消息接收方发送的消息读结果
	void OnMsgReadDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, const UidCode_t& sessionID);

	//void HandleMsgChatReadDeliverAckTask(const im::MESChatReadDelivereAck& msg);
	void OnMsgReadDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到添加好友操作结果推送应答

	void OnAndPushAck(std::shared_ptr<CImPdu> pPdu);
	void OnAndNotify(std::shared_ptr<CImPdu> pPdu);
	void OnIPushAck(std::shared_ptr<CImPdu> pPdu);
	void OnINotify(std::shared_ptr<CImPdu> pPdu);
	// 通知消息处理 2018-8-16 Abner
	void OnCommonNotifyMsg(std::shared_ptr<CImPdu> pPdu);
	void HandleCommonNotifyMsgTask(const im::MSGCommonNotify& msg, const UidCode_t& sessionId);
	void OnCommonNotifyMsgDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到消息接收方聊天消息推送应答
	void UserInfoModifyNotify(const im::MSGCommonNotify& msg, const UidCode_t& sessionId);
	
protected:	
	virtual bool RegistPacketExecutor(void);
private:
	void sendMsgChatAck(const im::MESChat& msg, im::ErrCode retCode, const UidCode_t& sessionId);
	void sendMsgChatCancelAck(const im::MESChatCancel& msg, im::ErrCode retCode, const UidCode_t& sessionId);
	// 通知消息会确认消息 2018-8-16 Abner
	void sendCommonNotifyMsgAck(const im::MSGCommonNotify& msg, im::ErrCode retCode, const UidCode_t& sessionId);
	
//	int32_t getFriendState(const string& userId, const string& friendId);
private:
	//COfflineMsgMgr m_offlineMsgMgr;
	int 			m_nNumberOfInst;
};
#endif // __MSGHANDLE_H__



