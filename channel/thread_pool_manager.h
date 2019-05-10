#ifndef __THREAD_POOL_MANAGER_H__
#define __THREAD_POOL_MANAGER_H__

#include "singleton.h"
#include "util.h"
#include "thread_pool.h"

class CThreadPoolManager :public Singleton <CThreadPoolManager>
{
public:
	virtual ~CThreadPoolManager();

public:
	void initSendChannelMsgPool(size_t threadNum, const std::string& poolName);
	std::shared_ptr<CThreadPool> getSendGroupMsgPool();

	void initInsertChannelMsgPool(size_t threadNum, const std::string& poolName);
	std::shared_ptr<CThreadPool> getInsertGroupMsgPool();
								
private:
	CThreadPoolManager();
	friend class Singleton <CThreadPoolManager>;

private:
	std::shared_ptr<CThreadPool> m_sendChannelMsgPoolPtr;
	std::shared_ptr<CThreadPool> m_insertChannelMsgPoolPtr;
};

#endif // __THREAD_POOL_MANAGER_H__
