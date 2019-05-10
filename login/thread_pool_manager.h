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
	void initLoginCacheMgrPoolPtr(size_t threadNum, const std::string& poolName);
	std::shared_ptr<CThreadPool> getLoginCacheMgrPool();
								
private:
	CThreadPoolManager();
	friend class Singleton <CThreadPoolManager>;

private:
	std::shared_ptr<CThreadPool> m_loginCacheMgrPoolPtr;
};

#endif // __THREAD_POOL_MANAGER_H__
