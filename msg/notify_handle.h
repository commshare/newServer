#ifndef __NOTIFY_HANDLE_H__
#define __NOTIFY_HANDLE_H__

#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include "im.push.android.pb.h"
#include "util.h"
#include"getDataInterfaceManager.h"
#include "im.inner.pb.h"

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


class CNotifyHandle : public CBaseHandle
{
public:
	CNotifyHandle(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CNotifyHandle();

public:
	void groupRelationNotify(std::shared_ptr<CImPdu> pPdu);
	void friendRelationNotify(std::shared_ptr<CImPdu> pPdu);
	void commonMsgNotify(std::shared_ptr<CImPdu> pPdu);

public:
	//群组添加（邀请）请求
	void OnJoinGrp(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionID);
	//AddFriendAnsDeliver插入mongo后的回调函数
	void OnJoinGrpDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);
	void OnJoinGrpDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 加群请求应答
	
	void OnGrpNotify(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionID);
	void HandleMsgGrpNotifyTask(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionId = UidCode_t());
	void OnGrpNotifyDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, bool bNeedSendPush);	
	void OnGrpNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu);

public:
	void AddFriendHandle(const im::SVRMSGFriendRelationNotify& friendNotify, const UidCode_t& sessionID);
	// AddFriendDeliver插入mongo后的回调函数
	void OnAddFriendDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);
	void OnAddFriendDeliverAck(std::shared_ptr<CImPdu> pPdu);			// 收到添加好友推送应答
	
	void AddFriendAnsHandle(const im::SVRMSGFriendRelationNotify& friendNotify, const UidCode_t& sessionID);
	//AddFriendAnsDeliver插入mongo后的回调函数
	void OnAddFriendAnsDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush);
	void OnAddFriendAnsDeliverAck(std::shared_ptr<CImPdu> pPdu);		// 收到添加好友操作结果推送应答

public:
	void CommonMsgNotifyHandle(const SVRMSGCommonMsgNotify& commNotify, const UidCode_t& sessionID);
	void HandleCommonNotifyMsgTask(const im::MSGCommonNotify& msg, const UidCode_t& sessionId);
	void OnCommonNotifyMsgDeliverAck(std::shared_ptr<CImPdu> pPdu);

protected:	
	virtual bool RegistPacketExecutor(void);

private:
	int 			m_nNumberOfInst;
	CGetDataInterfaceManager m_dataInterface;
};

#endif	//__NOTIFY_HANDLE_H__

