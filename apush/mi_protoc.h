#ifndef __MI_PROTOC_H_INCL__
#define __MI_PROTOC_H_INCL__

#include "util.h"
#include "mi_push_client.h"
#include "im.push.android.pb.h"

#define NOTIFY_ID_MAX 1000000000

class CMiProtoc
{

public:
	CMiProtoc(im::ANDPushMsg *pPbMsg);

	string GetSendBuf();
    //https
    string GetHttpUrl();
    void GetHttpHeaders(vector<string>& vstr);
    string GetHttpPostData(bool phoneCall = 0);
private:
	im::ANDPushMsg *m_pPbMsg;
};


#endif // __MI_PROTOC_H_INCL__

