/******************************************************************************
Filename: im_threadpool.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/09
Description: 
******************************************************************************/
#include <stdlib.h>
#include "threadpool.h"
#include "task.h"
#include "util.h"

/////////////////////////////////////////////////////////////////
CImWorkerThread::CImWorkerThread()
{
	m_task_cnt = 0;
	m_bRunning = false;
}

CImWorkerThread::~CImWorkerThread()
{

}

void* CImWorkerThread::StartRoutine(void* arg)
{
	CImWorkerThread* pThread = (CImWorkerThread*)arg;

	pThread->Execute();

	return NULL;
}

void CImWorkerThread::Start()
{
	m_bRunning = true;
	(void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
}


void CImWorkerThread::Stop()
{
	m_bRunning = false;
	m_thread_notify.Signal();
}


void CImWorkerThread::Execute()
{
	while (m_bRunning || !m_task_list.empty())	//线程没有结束或者还有任务未执行，则继续执行
	{
		m_thread_notify.Lock();

		// put wait in while cause there can be spurious wake up (due to signal/ENITR)
		while (m_task_list.empty() && m_bRunning) {
			m_thread_notify.Wait();
		}

		if (!m_task_list.empty())
		{
			 CTask* pTask = m_task_list.front();
			m_task_list.pop_front();
			m_thread_notify.Unlock();

			pTask->run();

			delete pTask;

			m_task_cnt++;
		}
		//log("%d have the execute %d task\n", m_thread_idx, m_task_cnt);
	}
}

void CImWorkerThread::PushTask(CTask* pTask)
{
	m_thread_notify.Lock();
	m_task_list.push_back(pTask);
	m_thread_notify.Signal();
	m_thread_notify.Unlock();
}

void CImWorkerThread::Wait()
{
	pthread_join(m_thread_id, NULL);
}

//////////////


CImThreadPool::CImThreadPool()
{
	m_worker_size = 0;
	m_worker_list = NULL;
}

CImThreadPool::~CImThreadPool()
{

}

int CImThreadPool::Init(uint32_t worker_size)
{
    m_worker_size = worker_size;
	m_worker_list = new CImWorkerThread [m_worker_size];
	if (!m_worker_list) {
		return 1;
	}

	for (uint32_t i = 0; i < m_worker_size; i++) {
		m_worker_list[i].SetThreadIdx(i);
		m_worker_list[i].Start();
	}

	return 0;
}

void CImThreadPool::Destory()
{
    if(m_worker_list)
        delete [] m_worker_list;
}

void CImThreadPool::AddTask(CTask* pTask, int index/*=-1*/)
{
	//uint32_t thread_idx = (-1 == index ? random() : (uint32_t)index) % m_worker_size;
	uint32_t thread_idx = random() % m_worker_size;
	m_worker_list[thread_idx].PushTask(pTask);
	//DbgLog("%d thread %p insert task %p", thread_idx, &m_worker_list[thread_idx], pTask);
}

void CImThreadPool::StopThreads()
{
	for (uint32_t i = 0; i < m_worker_size; i++) 
	{
		m_worker_list[i].Stop();
	}
}

void CImThreadPool::waitForDone()
{
	for (uint32_t i = 0; i < m_worker_size; i++)
	{
		m_worker_list[i].Wait();
	}
}

