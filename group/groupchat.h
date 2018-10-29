/******************************************************************************
Filename: groupchat.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description: 
******************************************************************************/
#ifndef __GROUPCHAT__H__
#define __GROUPCHAT__H__

#include "basehandle.h"
#include "configfilereader.h"
#include "im.group.pb.h"
#include "grpOfflineMsgMgr.h"

class CGrpOfflineMsg;

class CGroupChat : public CBaseHandle
{
public:
	CGroupChat(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CGroupChat();

	bool OnChat(std::shared_ptr<CImPdu> pPdu); 
	bool OnGrpChatInserted(const CGrpOfflineMsg& Msg, bool bInsertSuccess, const UidCode_t& sessionID);

	bool OnChatCancle(std::shared_ptr<CImPdu> pPdu);
	bool HandleMsgChatCancelTask(const GroupChatCancel& msg, const UidCode_t& sessionId);

	bool OngrpChatAck(std::shared_ptr<CImPdu> pPdu);
	bool OngrpChatCancelAck(std::shared_ptr<CImPdu> pPdu);
private:
	void sendChatAck(const im::GroupChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
	void sendChatCancleAck(const im::GroupChatCancel& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
	void sendChatAck(const CGrpOfflineMsg& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
protected:
	virtual bool RegistPacketExecutor(void);
private:
	CGrpOfflineMsgMgr m_grpOfflineMsgMgr;
	CGrpOfflineMsgMgr m_grpOfflineMsgMgrForLegalChat;		//用来处理通过合法性检查的群聊天消息的转发
	int 			   m_nNumberOfInst;
};
#endif 

