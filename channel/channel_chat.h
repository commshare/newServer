/******************************************************************************
Filename: groupchat.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description: 
******************************************************************************/
#ifndef __CHANNEL_CHAT__H__
#define __CHANNEL_CHAT__H__

#include "basehandle.h"
#include "configfilereader.h"
#include "im.mes.pb.h"
#include <set>
class CChannelChat : public CBaseHandle
{
public:
	CChannelChat(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CChannelChat();

public:
	bool OnChannelChat(std::shared_ptr<CImPdu> pPdu);
	void insertChannelChatMsgToDataBase(im::RadioChat msg, const UidCode_t& sessionId);
	void sendChannelChat(im::RadioChat msg, std::vector<string> vecMember);

	bool OnChannelAdminCancelChat(std::shared_ptr<CImPdu> pPdu);
	void channelAdminCancelChatHandle(const im::RadioCancelChat msg, const UidCode_t sessionId);
	bool OnChannelCancelChat(std::shared_ptr<CImPdu> pPdu);
	void channelCancelChatHandle(const im::RadioCancelChat msg, const UidCode_t sessionId);
	void sendCancelChatMsg(im::RadioCancelChat msg, std::vector<string> vecMember, bool isAdmin);

	void pushChannelChat(const string& fromId, const string& msgId, const string& radioId, std::vector<string> vecMember);

private:
	void cancelChatMsgToDataBase(const im::RadioCancelChat& msg, const UidCode_t& sessionId, bool isAdmin = true);
	bool checkCancelMsgIsAdmin(const im::RadioCancelChat& msg, const UidCode_t& sessionId);
	bool parseRadioCancelMsg(std::shared_ptr<CImPdu> pPdu, im::RadioCancelChat& msg);

private:
	void sendChatAck(const im::RadioChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc = string(""));
	void sendCancelChatAck(const im::RadioCancelChat& msg, const UidCode_t& sessionId, uint16_t cmdId, im::ErrCode retCode);
	
	
protected:
	virtual bool RegistPacketExecutor(void);

private:
	int 		m_nNumberOfInst;
};
#endif // __CHANNEL_CHAT__H__

