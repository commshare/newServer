
#include "ssl_post_mgr.h"
#include "ssl_http2.h"


void CPostMgr::CheckHeartBeat(HeartRange *range)
{
	
	int num = range->end - range->begin;
	int icount = 0;
	for (int i = range->begin; i < range->end; i++)
	{

		CPost *post = GetPostIndex(i);

		if (!post || !post->GetSslSock())
		{
			ErrLog("GetPostIndex %d is null", i);
			continue;
		}

		if (post->TimeOut())
		{
			InfoLog("GetUsrable Check out");
			post->GetSslSock()->Close();

			if(post->GetSslSock()->ReConnect() > 0)
			{
				post->SetUsrable(true);
				post->SetRespHeartBeatTime(time(NULL));
			}
		} 
		else
		{
			++icount;
		}
	}
	if (icount < num)
		WarnLog("socket total:%d, useable:%d ok!",num, icount);
}

void* CPostMgr::HeartBeatCheckThread(void* data)
{
	if (!data)
	{
		ErrLog("HeartBeatCheck Thread");
		pthread_exit(nullptr);
		return nullptr;
	}
	
	HeartRange *range = (HeartRange*)data;

	CPostMgr *mgr  = (CPostMgr *)range->usrData;
	if (!mgr)
	{
		ErrLog("HeartBeatCheck Thread");
		pthread_exit(nullptr);
		return nullptr;
	}
	
	while (1)
	{
		mgr->CheckHeartBeat(range);
		sleep(5);
	}
}

int CPostMgr::GetPostSize()
{
	 CAutoLock lock(&m_vecMutex);
	 return m_postVec.size();
}

CPost* CPostMgr::GetPostIndex(int index)
{
	CAutoLock lock(&m_vecMutex);

	if (index > (int)m_postVec.size())
	{
		return nullptr;
	}

	return m_postVec[index];
}

void CPostMgr::InsertPost(CPost * data)
{
	CAutoLock lock(&m_vecMutex);

	m_postVec.push_back(data);
}

int CPostMgr::AddThreadEventConns(int MaxNum)
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
void* CPostMgr::Increase(void* data)
{
	CPostMgr *mgr  = (CPostMgr *)data;

	int i = 0;
	int num = mgr->GetPerThreadConns();

	for (; i < num; i++)
	{
		CPost *post = mgr->createPost();
		mgr->InsertPost(post);

		/*int iRet =*/ post->InitConnection(mgr->m_sHost, mgr->m_uPort, mgr->m_callBackFun, 
										mgr->m_callback_data,  mgr->m_pSslEventDispatch, mgr->m_sslCeFileName.c_str(), mgr->m_sslRsaPkFileName.c_str());

	}

	pthread_exit(NULL);
}


//ResponeCallBack_t CPostMgr::m_callBackFun = nullptr;
//void*		CPostMgr::m_callback_data = nullptr;

static time_t tt = 0;


bool CPostMgr::Init(int uMaxConnections, ResponeCallBack_t fun, void* data, string host, uint16_t port, const char *sslCeFileName, const char *sslRsaPkFileName)
{
	//one thread hold 20 connections; 
	//debug
	tt = time(NULL);


	m_callBackFun = fun;
	m_callback_data = data;

	m_sHost = host;
	m_uPort = port;

	//
	if (uMaxConnections < m_uPerThreadConns)
	{
		m_uPerThreadConns = uMaxConnections;
	}

	m_uPostMaxConns = uMaxConnections - uMaxConnections % m_uPerThreadConns;

	if (sslCeFileName)
		m_sslCeFileName = sslCeFileName;
	if (sslRsaPkFileName)
		m_sslRsaPkFileName = sslRsaPkFileName;

	return true;
}


bool CPostMgr::Start()
{
	pthread_t tid;

	//每个线程创建m_uPerThreadConns个conn, 用m_uPostMaxConns/m_uPerThreadConns个线程创建所有的连接
	AddThreadEventConns(m_uPostMaxConns);

	InfoLog("%d connect init completed", m_uPostMaxConns);


	//创建线程检测heartBeat超时，每个线程检测HearBeatThreadCheckNum个conn连接
	int heartBeatCheckThreadNum = (m_uPostMaxConns + HearBeatThreadCheckNum - 1) / HearBeatThreadCheckNum;
	for (int i = 0; i< heartBeatCheckThreadNum; ++i)
	{

		HeartRange *range = new HeartRange;
		range->usrData = this;
		range->begin = i * HearBeatThreadCheckNum;
		const int endIndex = (i + 1)* HearBeatThreadCheckNum;
		range->end = endIndex > m_uPostMaxConns ? m_uPostMaxConns : endIndex;

		(void)pthread_create(&tid, nullptr, HeartBeatCheckThread, range);

	}

	InfoLog("HeartBeat Check Thread start", m_uPostMaxConns);
	return true;
}

static int postDebug = 0; 
int32_t CPostMgr::Post(shared_ptr<CApnsPostData> pData)
{
	//static int testCount = 0;
	//InfoLog("Post Post: %d\n", testCount++);
	int iRet = -2;

	CPost *post = nullptr;
	for (int i = 0; i < GetPostSize(); ++i)
	{
		++m_uPostIndex;

		if (m_uPostIndex == GetPostSize())
		{
			m_uPostIndex = 0;
		}

		post = m_postVec[m_uPostIndex];

		if (!post)
		{
			ErrLog("post");
			continue;
		}

		if (!post->GetUsrable())
		{
			InfoLog("socket:%d unUsrable!", post->GetSslSock()->GetSocket());
			continue;
		}
		else
		{
			iRet = post->Request(pData);
			if (iRet < 0)
			{
				//WarnLog("Request msgId %s to Id %s failed", pData->sMsgId.c_str(), pData->sToId.c_str());
				usleep(20);
			}
			else
			{
				++postDebug;
				//InfoLog("post success %d times, msgId = %s, toId = %s speed:%d/s\n", postDebug, pData->sMsgId.c_str(), pData->sToId.c_str(), postDebug / ((time(NULL) - tt)));
				break;
			}			
		}
	}
	
	return iRet;
}

CPost* CPostMgr::createPost()
{
	return new CPost;
}
