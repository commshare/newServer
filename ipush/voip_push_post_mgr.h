#ifndef __APNS_PUSH_POST_MGR_H__
#define __APNS_PUSH_POST_MGR_H__


#include "ssl_post_mgr.h"
class CSslEventDispatch;

class CVoipPostMgr : public CPostMgr
{
public:
	CVoipPostMgr(CSslEventDispatch *pSslEventDispatch = nullptr);
	virtual ~CVoipPostMgr();
private:
	virtual CPost* createPost() override;

};

#endif // __APNS_PUSH_POST_MGR_H__






