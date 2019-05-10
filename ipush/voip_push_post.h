#ifndef __APNS_PUSH_POST_H__
#define __APNS_PUSH_POST_H__

#include "ssl_post.h"

class CVoipPushPost : public CPost
{
public:
	
	CVoipPushPost()
	{
	}
	~CVoipPushPost()
	{
	}

private:
	virtual CSocketBase* createSocket();

	virtual int32_t Request(shared_ptr<CApnsPostData> pData) override;

};
#endif // __APNS_PUSH_POST_H__



