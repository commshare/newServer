#ifndef __SIGHANDLER_H__
#define __SIGHANDLER_H__
/******************************************************************************
Filename: friendhandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/29
Description: 处理添加，删除，拉黑好友等请求
******************************************************************************/
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

class CSigHandler : public CBaseHandle
{
public:
	CSigHandler(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CSigHandler();

	void OnSponsorCall(std::shared_ptr<CImPdu> pPdu);						// 响应添加好友请求
	void HandleSponsorCallTask(const im::SIGSponsorCall& msg, const UidCode_t& sessionId);
	void OnSponsorCallDeliverAck(std::shared_ptr<CImPdu> pPdu);			// 收到添加好友推送应答

	void OnSponsorCallAns(std::shared_ptr<CImPdu> pPdu);					// 响应添加好友操作结果（同意、拒绝）
	void OnSponsorCallAnsDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

	void OnHangUp(std::shared_ptr<CImPdu> pPdu);						// 响应挂机
	void HandleHangUpTask(const im::SIGHangUp& msg, const UidCode_t& sessionId);											// 实际响应聊天消息
	void OnHangupDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答
	

	////群组添加（邀请）请求
	//void OnJoinGrp(std::shared_ptr<CImPdu> pPdu);
	//void OnJoinGrpDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);	//AddFriendAnsDeliver插入mongo后的回调函数
	//void OnJoinGrpDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

	//交换key
	void OnExchangeNatInfo(std::shared_ptr<CImPdu> pPdu);
	void OnExchangeNatInfoDeliverAck(std::shared_ptr<CImPdu> pPdu);

	//呼叫状态改变
	void OnCallStateNotify(std::shared_ptr<CImPdu> pPdu);
	void OnCallStateNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu);
private:
	void sendHangUpAck(const im::SIGHangUp& msg, im::ErrCode retCode, const UidCode_t& sessionId);
	void sendSponsorCallAck(const im::SIGSponsorCall& msg, im::ErrCode retCode, const UidCode_t& sessionId);
protected:
	virtual bool RegistPacketExecutor(void);
private:
	//COfflineMsgMgr	m_offlineMsgMgr;
	int 			m_nNumberOfInst;
};

#endif // __SIGHANDLER_H__

