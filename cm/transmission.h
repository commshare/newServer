#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "util.h"
#include "packet.h"


using namespace im;
using namespace imsvr;


class CTransimission : public CPacket
{
public:
	CTransimission(int nNumOfInst);
	~CTransimission();

	bool Initialize(void);
	virtual bool OnLoginTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnMsgTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnGroupTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnClientTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnClientExange(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnChannelTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnDesktopTrans(std::shared_ptr<CImPdu> pPdu);

	bool OnCloseLink(std::shared_ptr<CImPdu> pPdu);
	bool OnCmLoginNotifyAck(std::shared_ptr<CImPdu> pPdu);
	bool OnLoginCMNotify(std::shared_ptr<CImPdu> pPdu);
	bool OnLoginResult(std::shared_ptr<CImPdu> pPdu);
    bool NotifyPCToLineOff(std::shared_ptr<CImPdu> pPdu);
protected:
	bool RegistPacketExecutor(void);
	bool GetUserId(std::shared_ptr<CImPdu> pPdu,string &sUserId);
	void UpdateUserLink(UidCode_t currentSessionId,string sUserId);

private:
	bool sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId);
	
private:
	int m_nNumberOfInst;
	
};

#endif

