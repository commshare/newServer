#ifndef __GROUP_CHAT_HANDLE_H__
#define __GROUP_CHAT_HANDLE_H__

#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include"getDataInterfaceManager.h"
//#include "im.group.pb.h"
#include "im.mes.pb.h"
#include "grpOfflineMsgMgr.h"

class CGroupChatHandle : public CBaseHandle
{
public:
	CGroupChatHandle(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CGroupChatHandle();

public:
	void OnGrpMsgChat(std::shared_ptr<CImPdu> pPdu);						// 响应群聊消息
	void OnGrpMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu);				// 收到消息接收方聊天消息推送应答
	
	void OnGrpMsgChatCancel(std::shared_ptr<CImPdu> pPdu);					// 响应群聊撤回消息
	void OnGrpMsgChatCancelDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到消息接收方撤销聊天消息推送应答

public:
	bool OnGrpChatInserted(const CGrpOfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID);
	bool HandleMsgChatCancelTask(const im::GroupChatCancel& msg, const UidCode_t& sessionID);

private:
	void sendGroupMsg(const im::MESGrpChat& msg, const MAP_GRP_MEMBER_INFO& mapMemberInfo);
	void sendGroupCancelMsg(const im::GroupChatCancel& msg, const MAP_GRP_MEMBER_INFO& mapMemberInfo);

private:
	void sendChatAck(const im::MESGrpChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
	void sendChatAck(const CGrpOfflineMsg& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
	void sendChatCancleAck(const im::GroupChatCancel& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));

protected:	
	virtual bool RegistPacketExecutor(void);

private:
	CGrpOfflineMsgMgr m_grpOfflineMsgMgr;
	int m_nNumberOfInst;
	CGetDataInterfaceManager m_dataInterface;
};

#endif // __GROUP_CHAT_HANDLE_H__
