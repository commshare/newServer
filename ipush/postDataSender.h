#ifndef __POSTDATASENDER_H__
#define __POSTDATASENDER_H__


//class CApnsPostData;
#include "CApnsPostData.h"
#include "CPostPduCacheMgr.h"
#include "thread.h"
#include <vector>

namespace im
{
	class PSvrMsg;
}
/**
 * TODO: Add class description
 * 
 * @author   root
 */




class CPostDataSender : public CThread
{
public:
	CPostDataSender(CPostMgr *pPostMgr, CPostPduCacheMgr *pPostPduCacheMgr);
	~CPostDataSender();

	void StopThread();								//Stop packet processing thread.
	void OnThreadRun(void);							// Thread of Process queue message 
	bool PostData(shared_ptr<CApnsPostData> data);	//Post message to queue ,waitting for process. 
private:
	bool					m_bRunning;
	CThreadNotify			m_ThreadNotify;			
	CPackStreamQueue		m_PackStreamQueue;
	CPostMgr				*m_pPostMgr;
	CPostPduCacheMgr		*m_pPostPduCacheMgr;
};


#endif // __POSTDATASENDER_H__
