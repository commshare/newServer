#include "voip_push_post_mgr.h"
#include "voip_push_post.h"

CVoipPostMgr::CVoipPostMgr(CSslEventDispatch *pSslEventDispatch /*= nullptr*/)
	:CPostMgr(pSslEventDispatch)
{

}

CVoipPostMgr::~CVoipPostMgr()
{

}

CPost* CVoipPostMgr::createPost()
{
	return new CVoipPushPost;
}

