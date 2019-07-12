#ifndef __CHANNEL_NOTIFY_H__
#define __CHANNEL_NOTIFY_H__

#include "basehandle.h"
#include "configfilereader.h"
#include "im.mes.pb.h"
#include "im.inner.pb.h"

class CChannelNotify : public CBaseHandle
{
public:
	CChannelNotify(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CChannelNotify();

public:
	bool OnChannelNotify(std::shared_ptr<CImPdu> pPdu);
	void insertChannelNotifyToDataBase(im::SVRRadioMsgNotify msg);
	void sendChannelNotify(im::RadioChat msg, std::vector<string> vecMember);

	bool OnChannelPushSetNotify(std::shared_ptr<CImPdu> pPdu);
	void setChannelPushStatus(im::SVRRadioPushSetNotify msg);

private:
	bool updateUserChannelInfo(const im::SVRRadioMsgNotify& msg);
	bool packagingRadioChat(const im::SVRRadioMsgNotify& msg, im::RadioChat& chat);

protected:
	virtual bool RegistPacketExecutor(void);

private:
	int m_nNumberOfInst;
};

#endif // __CHANNEL_NOTIFY_H__
