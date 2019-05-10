#ifndef __EXCHANGE_KEY_HANDLE_H__
#define __EXCHANGE_KEY_HANDLE_H__
#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"

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

class CExchangeKeyHandle : public CBaseHandle
{
public:
	CExchangeKeyHandle(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CExchangeKeyHandle();

public:
	void OnExchangeKey(std::shared_ptr<CImPdu> pPdu);
	void OnExchangeKeyInserted(const std::vector<std::shared_ptr<COfflineMsg> >& msgs, bool bInsertSuccess, const UidCode_t& sessionID);
	void OnExchangeKeyDeliverAck(std::shared_ptr<CImPdu> pPdu);
	void OnExchangeKeyDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess);
	void OnExchangeKeyDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu);

protected:	
	virtual bool RegistPacketExecutor(void);

private:
	int 			m_nNumberOfInst;
};


#endif // __EXCHANGE_KEY_HANDLE_H__