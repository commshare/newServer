#ifndef __MI_PROTOC_H_INCL__
#define __MI_PROTOC_H_INCL__

// TODO: your code here
#include "util.h"
#include "mi_push_client.h"
#include "im.push.android.pb.h"

class CMiProtoc
{

public:
	CMiProtoc(im::ANDPushMsg *pPbMsg);

	string GetSendBuf();
private:
	im::ANDPushMsg *m_pPbMsg;
};


#endif // __MI_PROTOC_H_INCL__

