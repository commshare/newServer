#include "thread_pool_manager.h"

CThreadPoolManager::CThreadPoolManager()
{

}

CThreadPoolManager::~CThreadPoolManager()
{

}

void CThreadPoolManager::initLoginCacheMgrPoolPtr(size_t threadNum, const std::string& poolName)
{
	if(!m_loginCacheMgrPoolPtr)
		m_loginCacheMgrPoolPtr = std::make_shared<CThreadPool>();
	try
	{
		m_loginCacheMgrPoolPtr->init_thread_num(threadNum, poolName);
	}
	catch(std::invalid_argument& ia)
	{
		ErrLog("init login cache manager pool except:%s", ia.what());
	}
}

std::shared_ptr<CThreadPool> CThreadPoolManager::getLoginCacheMgrPool()
{
	return m_loginCacheMgrPoolPtr;
}

