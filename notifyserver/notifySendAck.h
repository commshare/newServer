#ifndef __NOTIFY_SEND_ACK_H__
#define __NOTIFY_SEND_ACK_H__

#include "packet.h"

//在新线程中发送
class CNotifySendAck : public CPacket
{
public:
	CNotifySendAck();
	~CNotifySendAck();

public:
	bool Initialize();
	bool RegistPacketExecutor(void);

public:
	void OnLoginNotifyAck(std::shared_ptr<CImPdu> pPdu);
	void OnGroupRelationAck(std::shared_ptr<CImPdu> pPdu);
	void OnFriendRelationAck(std::shared_ptr<CImPdu> pPdu);
	void OnCommonMsgNotifyAck(std::shared_ptr<CImPdu> pPdu);
	void OnRadioMsgNotifyAck(std::shared_ptr<CImPdu> pPdu);
	void OnUserPushSetNotifyAck(std::shared_ptr<CImPdu> pPdu);

private:
	int m_nActualServiceInst;

	
};
#endif // __PDUSENDER_H__

