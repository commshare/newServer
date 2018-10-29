/**
 * ssl_packet_queue.h 
 *  
 * create by liulang 
 * desc: 
 * class  
 * class 
 *  
 * 2017-07-11 
 * datuhao@foxmail.com 
 */

#ifndef SSL_PACKET_QUEUE_H
#define SSL_PACKET_QUEUE_H

#include <queue>
#include <stdio.h>
#include <pthread.h>
#include "lock.h"
#include "protobuf_phase.h"

using namespace std;


class CPackStreamQueue
{
public:

	CPackStreamQueue(unsigned int MaxQueueSize = 1000000)
		:m_uMaxQueueSize(MaxQueueSize)
	{
	}

	~CPackStreamQueue()
	{
	}

	shared_ptr<APushData> PopFront()
	{
		CAutoLock autoLock(&m_MutexLock);

		if(m_packQueue.empty())
		{
			//printf("queue is empty!\n");
			return nullptr;
		}

		shared_ptr<APushData> pack = m_packQueue.front();

		m_packQueue.pop();

		return pack;
	}

	int PushBack(shared_ptr<APushData> pPack, bool bForce = false)
	{
		CAutoLock autoLock(&m_MutexLock);

		if (!pPack)
		{
			printf("pPack is null!\n");
		}
		if (m_packQueue.size() >= m_uMaxQueueSize && !bForce)
		{
			printf("queue size is >= %d!\n", m_uMaxQueueSize);
			return -1;
		}

		m_packQueue.push(pPack);

		return m_packQueue.size();;
	}

	uint32_t GetSize()
	{
		CAutoLock autoLock(&m_MutexLock);
		return m_packQueue.size();
	}

private:
	queue<shared_ptr<APushData>> 	m_packQueue;
	unsigned int 	m_uMaxQueueSize;
	CLock 			m_MutexLock;
};





#endif
