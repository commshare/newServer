
#include "ssl_post_mgr.h"


void CPostPoolMgr::CheckHeartBeat(HeartRange *range)
{
	
	int num = range->end - range->begin;
	int icount = 0;
	for (int i=range->begin; i< range->end; i++)
	{
		CPost *post = GetPostDataIndex(i);

		if (!post)
		{
			ErrLog("range:%d, i%d", range->end, i);
			continue;
		}

		//not respone, wait for 2 sec
		if (!post->GetUsrable())
		{			
			if (time(NULL) - post->GetHeartbeat() > 2)
			{
				DbgLog("not userable Check Heart Beat time out");

				if(post->ReConnectPost() < 0)
				{
					ErrLog("ReConnectPost");
				}
			}
		}
		else
		{
			icount++;
		}
	}
	if (icount < num)
		DbgLog("socket total:%d, useable:%d ok!", num, icount);
}

void* CPostPoolMgr::HeartBeatCheckThread(void* data)
{
	if (!data)
	{
		ErrLog("thread param null");
		return nullptr;
	}
	
	HeartRange *range = (HeartRange*)data;

	CPostPoolMgr *mgr  = (CPostPoolMgr *)range->threadParam;

	if (!mgr)
	{	
		ErrLog("thread param threadParam null");
		return nullptr;
	}
	
	while (1)
	{
		mgr->CheckHeartBeat(range);
		usleep(100*1000);
	}
}

int CPostPoolMgr::GetPostDataSize()
{
	 CAutoLock lock(&m_vecMutex);
	 return m_pPostVec.size();
}

CPost* CPostPoolMgr::GetPostDataIndex(int index)
{
	//CAutoLock lock(&m_vecMutex);

	if ((unsigned int)index > m_pPostVec.size())
	{
		return nullptr;
	}

	return m_pPostVec[index];
}

void CPostPoolMgr::InsertPostData(CPost * data)
{
	//CAutoLock lock(&m_vecMutex);
	m_pPostVec.push_back(data);
}

int CPostPoolMgr::AddThreadEventConns(int MaxNum)
{
	
	//to use multi threads connect to the huawei server quickly
	//m_uPerThreadConns = 10;
	int num = MaxNum/m_uPerThreadConns;

	pthread_t tid[num];
	for (int i=0; i<num; i++)
	{
		(void)pthread_create(&tid[i], nullptr, Increase, this);
	}

	//wait for complete
	for (int i=0; i<num; i++)
	{
		pthread_join(tid[i], nullptr);
	}
	
	return 0;
}

//Increase Connections
void* CPostPoolMgr::Increase(void* data)
{
	if (!data)
	{
		ErrLog("thread param null");
		return nullptr;
	}
	CPostPoolMgr *mgr  = (CPostPoolMgr *)data;

	int i = 0;
	int num = mgr->GetPerThreadConns();
	for (; i < num; i++)
	{
		CPost *post = new CPost;
		/*int iRet = */post->InitConnection(mgr->m_sHost, mgr->m_uPort, 
						mgr->m_callBackFun, mgr->m_callback_data, nullptr);

		mgr->InsertPostData(post);
	}

	pthread_exit(NULL);
}

bool CPostPoolMgr::Init(int uMaxConnections, OnNotifyCallBack_t fun, void* data, string host, uint16_t port)
{

	if (uMaxConnections <= 0)
	{
		ErrLog("uMaxConnections: %d ", uMaxConnections);
		return false;
	}

	if (!fun || !data)
	{
		ErrLog("param null ");
		return false;
	}
	//one thread hold 20 connections;
	m_callBackFun = fun;
	m_callback_data = data;

	m_sHost = host;
	m_uPort = port;

	//防止下面的值为0;
	if (uMaxConnections < m_uPerThreadConns) 
		m_uPerThreadConns = uMaxConnections;

	m_uPostMaxConns = uMaxConnections - uMaxConnections % m_uPerThreadConns;

	pthread_t tid;

	//初始化链路,通过在多线程中启动链路加快启动速度
	AddThreadEventConns(m_uPostMaxConns);
	InfoLog("%d connect init completed", m_uPostMaxConns);

	//初始化心跳管理线程
	int threadCount = (m_uPostMaxConns + HearBeatThreadCheckNum - 1 ) / HearBeatThreadCheckNum;
	for (int i = 0; i < threadCount; ++i)
	{

		HeartRange *range = new HeartRange;

		range->threadParam = this;
		range->begin = i * HearBeatThreadCheckNum;
		range->end = (i + 1) * HearBeatThreadCheckNum;
		range->end = range->end > m_uPostMaxConns ? m_uPostMaxConns : range->end;

		(void)pthread_create(&tid, nullptr, HeartBeatCheckThread, range);
	}

	InfoLog("HeartBeat Check Thread start", m_uPostMaxConns);
	
	return true;
}

int32_t CPostPoolMgr::Post(shared_ptr<APushData> ptrData)
{
	int iRet = -1;
	int iCount = 0;

	//如果单轮查找失败，休眠100us后进行第二轮查找，查找5轮后如果还没有可用的则放弃
	while (iCount++ < 5 )
	{
		/*if (m_uPostIndex == GetPostDataSize())
		{
			m_uPostIndex = 0;
		}*/
		//assert(GetPostDataSize());
		m_uPostIndex = m_uPostIndex % GetPostDataSize();

		for (; m_uPostIndex < GetPostDataSize(); m_uPostIndex++)
		{
			//该数组初始化后,不做任何增删改操作,故无需加锁
			CPost *post = m_pPostVec[m_uPostIndex];
			if (!post)
			{
				ErrLog("post");
				continue;
			}
			if (!post->GetUsrable())
			{
				continue;
			}

			iRet = post->Request(ptrData);

			if (iRet <= 0)
			{
				ErrLog("Request failue");

				//for debug
				//return iRet;
			}
			else
			{
				//InfoLog("\n%d post success: %d, speed:%d/s\n", post->GetSslSock()->GetSocket(), ++postDebug, postDebug/((time(NULL)-tt)));
				DbgLog("%d post success", post->GetSslSock()->GetSocket());
				return iRet;
			}
			
		}

		usleep(100);
		ErrLog("Server busy!");
	}


	if (iCount == 5)
	{
		APushData *pData = ptrData.get();
		ErrLog("msg:%s, diveceType:%d send time out!", pData->msgId.c_str(), pData->diveceType);
		iRet = -1;
	}

	return iRet;
}


