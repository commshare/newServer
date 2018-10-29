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
	virtual bool OnMsgTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnGroupTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnClientTrans(std::shared_ptr<CImPdu> pPdu);
	virtual bool OnClientExange(std::shared_ptr<CImPdu> pPdu);
protected:
	bool RegistPacketExecutor(void);
	bool GetUserId(std::shared_ptr<CImPdu> pPdu,string &sUserId);
private:
	int m_nNumberOfInst;
	
};

#endif

