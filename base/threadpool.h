/******************************************************************************
Filename: im_threadpool.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/09
Description: 
******************************************************************************/

#ifndef __IM_THREADPOOL_H__
#define __IM_THREADPOOL_H__

#include "ostype.h"
#include "thread.h"
#include <list>


class CTask;

class CImWorkerThread 
{
public:
	CImWorkerThread();
	~CImWorkerThread();

	static void* StartRoutine(void* arg);

	void Start();
	void Stop();
	void Execute();
	void PushTask(CTask* pTask);
	void Wait();
	void SetThreadIdx(uint32_t idx) { m_thread_idx = idx; }

private:
	bool			m_bRunning;
	uint32_t		m_thread_idx;
	uint32_t		m_task_cnt;
	pthread_t		m_thread_id;
	CThreadNotify	m_thread_notify;
	std::list<CTask*>	m_task_list;
};

class CImThreadPool 
{
public:
	CImThreadPool();
	virtual ~CImThreadPool();

	int Init(uint32_t worker_size);
	void AddTask(CTask* pTask, int index=-1);
	void Destory();
	void StopThreads();
	//bool IsInited(){ return m_worker_list != NULL; }

	// Waits up to msecs milliseconds for all threads to exit and removes all threads from the thread pool.
	void waitForDone();
	
protected:
	uint32_t 			m_worker_size;
	CImWorkerThread* 	m_worker_list;
};
#endif // __IM_THREADPOOL_H__


