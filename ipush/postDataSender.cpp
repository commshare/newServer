#include "postDataSender.h"
#include "pushHandler.h"
//#include "CApnsPostData.h"
#include "pushserver_manager.hpp"

CPostDataSender::CPostDataSender(CPostMgr *pPostMgr, CPostPduCacheMgr *pPostPduCacheMgr)
	:m_pPostMgr(pPostMgr), m_pPostPduCacheMgr(pPostPduCacheMgr)
{
}

CPostDataSender::~CPostDataSender()
{
	StopThread();
}

void CPostDataSender::StopThread()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		pthread_join(m_thread_id, NULL);
	}
}

void CPostDataSender::OnThreadRun(void)
{
	m_bRunning = true;
	while (m_bRunning || m_PackStreamQueue.GetSize() > 0)
	{
		//InfoLog("CPostDataSender thread ready for SendData");

		m_ThreadNotify.Lock();

		// put wait in while cause there can be spurious wake up (due to signal/ENITR)
		while (m_PackStreamQueue.GetSize() == 0 && m_bRunning)
		{
			m_ThreadNotify.Wait();
		}


		if (m_PackStreamQueue.GetSize() > 0)
		{
			shared_ptr<CApnsPostData> pData = m_PackStreamQueue.PopFront();

			if (m_pPostPduCacheMgr && m_pPostPduCacheMgr->Insert(pData) < 0)
			{
				ErrLog("m_pPostPduCacheMgr Insert");
			}
			m_ThreadNotify.Unlock();

			m_pPostMgr->Post(pData);		//
		}
	}
}

bool CPostDataSender::PostData(shared_ptr<CApnsPostData> pPostData)
{
	bool bRet = false;

	if (m_bRunning)
	{
		if (m_PackStreamQueue.PushBack(pPostData) > 0)
		{
			bRet = true;
		}

		m_ThreadNotify.Signal();
	}
	else
	{
		WarnLog("###CPostDataSender not running");
	}

	return bRet;
}


