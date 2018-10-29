#include "CPostPduCacheMgr.h"
#include "CApnsPostData.h"
// TODO: your implementation here


CPostPduCacheMgr::CPostPduCacheMgr()
	:m_timeOut(10), m_bRunning(false)
{
	m_index = 0;

	m_pCacheMap = new map<int, shared_ptr<CApnsPostData>>;
}

CPostPduCacheMgr::~CPostPduCacheMgr()
{

}

void CPostPduCacheMgr::Init(CheckCallBack_t fun, void *userData)
{
	m_checkCallBack = fun;
	m_userData = userData;

	m_index = 0;
}

int CPostPduCacheMgr::Insert(shared_ptr<CApnsPostData> pdata)
{

	if (!pdata)
	{
		WarnLog("postData is nullptr!");
		return -1;
	}

	CApnsPostData *postData = pdata.get();
	if (!postData)
	{
		WarnLog("pdata.get() is nullptr!");
		return -1;
	}

	CAutoLock lock(&m_mapMutex);
	m_index++;

	postData->mapIndex = m_index;
	postData->sendTime = time(NULL);
	++postData->sendTimes;
	
	//InfoLog("post msgId:%s, toId:%s send %d times", postData->sMsgId.c_str(), postData->sToId.c_str(), postData->sendTimes);
	//return m_index;

	if(!(m_pCacheMap->insert(pair<int, shared_ptr<CApnsPostData> >(m_index, pdata))).second )
	{
		map<int, shared_ptr<CApnsPostData>>::iterator it = m_pCacheMap->find(m_index);
		if (it != m_pCacheMap->end())
		{
			ErrLog("the index is exist!");
			//big Error!
			/*
			delete pCache;
			pCache = nullptr;
			*/
			return -1;
		}
	}

	if (m_index > 0x7ffffffd)
	{
		m_index = 0;
	}

	return m_index;
}


bool CPostPduCacheMgr::Delete(shared_ptr<CApnsPostData> pdata)
{
	if (!pdata)
	{
		ErrLog("pdata is nullptr");
		return false;
	}
	
	CApnsPostData *data;
	data = pdata.get();
	if (!data)
	{
		ErrLog("data is nullptr");
		return false;
	}

	return Delete(data->mapIndex);
}


bool CPostPduCacheMgr::Delete(int index)
{

	//return true;
	CAutoLock lock(&m_mapMutex);

	if (m_pCacheMap->empty())
	{
		ErrLog("m_cacheMap empty");
		return  false;
	}

	map<int, shared_ptr<CApnsPostData> >::iterator it = m_pCacheMap->find(index);

	if (it != m_pCacheMap->end())
	{
		shared_ptr<CApnsPostData> pdata = it->second;
		
		if (pdata)
		{
			CApnsPostData *pCache = pdata.get();

			pdata.reset();
			m_pCacheMap->erase(it);
			return true;

			if (pCache->mapIndex!=index)
			{
				ErrLog("pCache->mapIndex:%d!=index:%d", pCache->mapIndex, index);

				//m_pCacheMap->erase(it);
				//return false;
			}
			
			InfoLog("delete apnsid:%s mapIndex:%d", pCache->sMsgId.c_str(), pCache->mapIndex);
			m_pCacheMap->erase(it);

			return true;			
		}
		else
		{
			ErrLog("pCache");
		}
	}

	ErrLog("m_cacheMap not found index %d", index);
	return false;
}

int CPostPduCacheMgr::CheckCache(time_t timeRate)
{
	CAutoLock lock(&m_mapMutex);

	if (!m_checkCallBack || !m_userData)
	{
		ErrLog("m_checkCallBack, m_userData null");
		return -1;
	}

	if (m_pCacheMap->size() > 0)
		InfoLog("pduCache map size:%d\n", m_pCacheMap->size());
	
	map<int, shared_ptr<CApnsPostData>>::iterator it = m_pCacheMap->begin();

	CApnsPostData *pCache;

	//time_t tm = time(NULL);

	//int iDeleteNum = 0;
	for (; it != m_pCacheMap->end();)
	{
		pCache = it->second.get();

		if (pCache->Timeout(timeRate))
		{
			m_checkCallBack(it->second, m_userData);
			m_pCacheMap->erase(it++);
		}
		else
		{
			++it;
		}
	}//for

	return 0;
}

void CPostPduCacheMgr::OnThreadRun(void)
{
	m_bRunning = true;
	InfoLog("CPostPduCacheMgr %p checkMap thread start, timeOut = %d", this, m_timeOut);
	while (m_timeOut)
	{

		CheckCache(m_timeOut);

		sleep(15);
	}
	m_bRunning = false;
	InfoLog("CPostPduCacheMgr %p checkMap thread stop", this);
}

int CPostPduCacheMgr::CheckMap(time_t timeRate)
{
	m_timeOut = timeRate > 100000 ? 100000 : timeRate;
	if (!m_bRunning)		//如果线程没有启动则启动线程
		StartThread();
	else
		InfoLog("CPostPduCacheMgr %p checkMap thread already running,timeout set to %d", this, m_timeOut);
	return true;
}

