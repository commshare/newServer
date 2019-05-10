#include "thread_pool_manager.h"

CThreadPoolManager::CThreadPoolManager()
{

}

CThreadPoolManager::~CThreadPoolManager()
{

}

void CThreadPoolManager::initSendChannelMsgPool(size_t threadNum, const std::string& poolName)
{
	if(!m_sendChannelMsgPoolPtr)
		m_sendChannelMsgPoolPtr = std::make_shared<CThreadPool>();
	try
	{
		m_sendChannelMsgPoolPtr->init_thread_num(threadNum, poolName);
	}
	catch(std::invalid_argument& ia)
	{
		ErrLog("init send channel msg pool except:%s", ia.what());
	}
}

std::shared_ptr<CThreadPool> CThreadPoolManager::getSendGroupMsgPool()
{
	return m_sendChannelMsgPoolPtr;
}

void CThreadPoolManager::initInsertChannelMsgPool(size_t threadNum, const std::string& poolName)
{
	if(!m_insertChannelMsgPoolPtr)
		m_insertChannelMsgPoolPtr = std::make_shared<CThreadPool>();
	try
	{
		m_insertChannelMsgPoolPtr->init_thread_num(threadNum, poolName);
	}
	catch(std::invalid_argument& ia)
	{
		ErrLog("init insert channel msg pool except:%s", ia.what());
	}
}

std::shared_ptr<CThreadPool> CThreadPoolManager::getInsertGroupMsgPool()
{
	return m_insertChannelMsgPoolPtr;
}

